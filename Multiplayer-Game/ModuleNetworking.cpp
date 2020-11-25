#include "Networks.h"
#include "ModuleNetworking.h"



////////////////////////////////////////////////////////////////////////
// ModuleNetworking public methods
////////////////////////////////////////////////////////////////////////

void ModuleNetworking::disconnect()
{
	setEnabled(false);
}



////////////////////////////////////////////////////////////////////////
// ModuleNetworking protected methods
////////////////////////////////////////////////////////////////////////

bool ModuleNetworking::createSocket()
{
	// Create
	socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	if (socket == INVALID_SOCKET)
	{
		reportError("ModuleNetworking::createSocket() - socket");
		return false;
	}

	// Set non-blocking mode
	bool enableBlockingMode = true;
	u_long arg = enableBlockingMode ? 1 : 0;
	int res = ioctlsocket(socket, FIONBIO, &arg);
	if (res == SOCKET_ERROR) {
		reportError("ModuleNetworking::createSocket() - ioctlsocket FIONBIO");
		disconnect();
		return false;
	}

	return true;
}

bool ModuleNetworking::bindSocketToPort(int port)
{
	sockaddr_in local_address;
	local_address.sin_addr.S_un.S_addr = INADDR_ANY;
	local_address.sin_family = AF_INET;
	local_address.sin_port = htons(port);
	int res = bind(socket, (sockaddr*)&local_address, sizeof(local_address));
	if (res == SOCKET_ERROR) {
		reportError("ModuleNetworkingServer::start() - bind");
		disconnect();
		return false;
	}

	return true;
}

void ModuleNetworking::sendPacket(const OutputMemoryStream & packet, const sockaddr_in &destAddress)
{
	sendPacket(packet.GetBufferPtr(), packet.GetSize(), destAddress);
}

void ModuleNetworking::sendPacket(const char * data, uint32 size, const sockaddr_in &destAddress)
{
	ASSERT(size <= DEFAULT_PACKET_SIZE); // NOTE(jesus): Increase DEFAULT_PACKET_SIZE if not enough

	int byteSentCount = sendto(socket,
		(const char*)data,
		size,
		0, (sockaddr*)&destAddress, sizeof(destAddress));

	if (byteSentCount <= 0)
	{
		reportError("ModuleNetworking::sendPacket() - sendto");
	}
	else
	{
		sentPacketsCount++;
	}
}

void ModuleNetworking::reportError(const char* inOperationDesc)
{
	LPVOID lpMsgBuf;
	DWORD errorNum = WSAGetLastError();

	FormatMessageA(
		FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		errorNum,
		MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);

	ELOG("Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf);
}



////////////////////////////////////////////////////////////////////////
// Module virtual methods
////////////////////////////////////////////////////////////////////////

bool ModuleNetworking::init()
{
	WORD version = MAKEWORD(2, 2);
	WSADATA data;
	if (WSAStartup(version, &data) == SOCKET_ERROR)
	{
		reportError("ModuleNetworking::init() - WSAStartup");
		return false;
	}

	simulatedRealWorldConditions_Init();

	return true;
}

bool ModuleNetworking::start()
{
	sentPacketsCount = 0;
	receivedPacketsCount = 0;
	
	onStart();

	return true;
}

bool ModuleNetworking::preUpdate()
{
	if (socket == INVALID_SOCKET) return true;

	BEGIN_TIMED_BLOCK(NetRecv);
	
	processIncomingPackets();

	END_TIMED_BLOCK(NetRecv);

	return true;
}

bool ModuleNetworking::update()
{
	if (socket == INVALID_SOCKET) return true;

	BEGIN_TIMED_BLOCK(NetSend);

	onUpdate();

	END_TIMED_BLOCK(NetSend);

	return true;
}

bool ModuleNetworking::gui()
{
	if (isConnected())
	{
		ImGui::Begin("ModuleNetworking window");
		
		ImGui::Text(" - Current time: %f", Time.time);
		ImGui::Text(" - # Packet sent: %u", sentPacketsCount);
		ImGui::Text(" - # Packet received: %u", receivedPacketsCount);

		ImGui::Text(" - # Networked objects: %u", App->modLinkingContext->getNetworkGameObjectsCount());

		if (ImGui::Button("Disconnect")) {
			disconnect();
		}

		ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.45f);

		// Simulate real world conditions
		if (ImGui::CollapsingHeader("Simulate real world conditions", ImGuiTreeNodeFlags_DefaultOpen))
		{
			ImGui::Checkbox("Simulate latency / jitter", &simulateLatency);
			if (simulateLatency)
			{
				ImGui::InputFloat("Max. latency (s)", &simulatedLatency, 0.001f, 0.01f, 4);
				ImGui::InputFloat("Max. jitter (s)", &simulatedJitter, 0.001f, 0.01f, 4);
				if (ImGui::Button("Reset##defaults_latency_jitter")) {
					simulatedLatency = 0.07f;
					simulatedJitter = 0.03f;
				}
			}
			ImGui::Checkbox("Simulate packet drops", &simulateDrops);
			if (simulateDrops)
			{
				ImGui::InputFloat("Drop ratio", &simulatedDropRatio, 0.01f, 0.1f, 4);
				if (ImGui::Button("Reset##default_drop_ratio")) {
					simulatedDropRatio = 0.01f;
				}
				ImGui::Text(" # Dropped packets: %d", simulatedPacketsDropped);
				ImGui::Text(" # Received packets: %d", simulatedPacketsReceived);
			}

			ImGui::Text("Simulated buffer usage: %d%%", 100 * pendingSimulatedPacketsCount / MAX_SIMULATED_PACKETS);
		}

		onGui();

		ImGui::PopItemWidth();

		ImGui::End();
	}

	return true;
}

bool ModuleNetworking::stop()
{
	onDisconnect();

	closesocket(socket);
	socket = INVALID_SOCKET;

	sentPacketsCount = 0;
	receivedPacketsCount = 0;

	simulatedRandom = RandomNumberGenerator();

	return true;
}

bool ModuleNetworking::cleanUp()
{
	if (WSACleanup() == SOCKET_ERROR)
	{
		reportError("ModuleNetworking::cleanUp() - WSACleanup");
		return false;
	}

	return true;
}



//////////////////////////////////////////////////////////////////////
// Incoming / outgoing packet processing
//////////////////////////////////////////////////////////////////////

void ModuleNetworking::processIncomingPackets()
{
	// Handle incoming packets
	while (true)
	{
		sockaddr_in fromAddress = {};
		socklen_t fromLength = sizeof(fromAddress);

		InputMemoryStream inPacket;

		int readByteCount = recvfrom(socket,
			(char*)inPacket.GetBufferPtr(),
			inPacket.GetCapacity(),
			0,
			(sockaddr*)&fromAddress,
			&fromLength);

		if (readByteCount > 0)
		{
			inPacket.SetSize(readByteCount);

			if (simulateLatency || simulateDrops)
			{
				simulatedRealWorldConditions_EnqueuePacket(inPacket, fromAddress);
			}
			else
			{
				receivedPacketsCount++;
				onPacketReceived(inPacket, fromAddress);
			}
		}
		else
		{
			int error = WSAGetLastError();

			if (readByteCount == 0)
			{
				// Graceful disconnection from remote socket
				onConnectionReset(fromAddress);
			}
			else if (error == WSAEWOULDBLOCK)
			{
				// NOTE(jesus): This is not an error for us, as the socket is configured in
				// non-blocking mode. This means that there was no incoming data available
				// when recvfrom was executed.
			}
			else if (error == WSAECONNRESET)
			{
				//this can happen if a remote socket closed and we haven't DC'd yet.
				//this is the ICMP message being sent back saying the port on that computer is closed
				char fromAddressStr[64];
				inet_ntop(AF_INET, &fromAddress.sin_addr, fromAddressStr, sizeof(fromAddressStr));
				WLOG("ModuleNetworking::processIncomingPackets() - Connection reset from %s:%d",
					fromAddressStr,
					ntohs(fromAddress.sin_port));

				onConnectionReset(fromAddress);
			}
			else
			{
				reportError("ModuleNetworking::processIncomingPackets() - recvfrom");
			}

			break;
		}
	}

	if (simulateLatency || simulateDrops)
	{
		simulatedRealWorldConditions_ProcessQueuedPackets();
	}
}



//////////////////////////////////////////////////////////////////////
// Real world conditions simulation
//////////////////////////////////////////////////////////////////////

void ModuleNetworking::simulatedRealWorldConditions_Init()
{
	freeSimulatedPackets = nullptr;
	pendingSimulatedPackets = nullptr;
	pendingSimulatedPacketsCount = 0;
	for (uint32 i = ArrayCount(simulatedPackets); i > 0; --i)
	{
		SimulatedPacket *next = freeSimulatedPackets;
		freeSimulatedPackets = &simulatedPackets[i-1];
		freeSimulatedPackets->next = next;
	}
}

void ModuleNetworking::simulatedRealWorldConditions_EnqueuePacket(const InputMemoryStream &packet, const sockaddr_in &fromAddress)
{
	float packetChance = simulatedRandom.next();

	if (simulateDrops == false)
	{
		packetChance = 1.0f;
	}

	if (packetChance > simulatedDropRatio)
	{
		float randomJitterFactor = 2.0f * simulatedRandom.next() - 1.0f; // from -1 to 1
		double receptionTime = Time.time + simulatedLatency + simulatedJitter * randomJitterFactor;

		if (simulateLatency == false)
		{
			receptionTime = Time.time;
		}

		SimulatedPacket *simulatedPacket = freeSimulatedPackets;
		ASSERT(simulatedPacket != nullptr);
		freeSimulatedPackets = simulatedPacket->next;

		std::memcpy((void*)simulatedPacket->packet.GetBufferPtr(), packet.GetBufferPtr(), packet.GetSize());
		simulatedPacket->packet.Clear();
		simulatedPacket->packet.SetSize(packet.GetSize());
		simulatedPacket->fromAddress = fromAddress;
		simulatedPacket->receptionTime = receptionTime;
		
		if (pendingSimulatedPackets == nullptr || pendingSimulatedPackets->receptionTime > receptionTime)
		{
			simulatedPacket->next = pendingSimulatedPackets;
			pendingSimulatedPackets = simulatedPacket;
		}
		else
		{
			for (SimulatedPacket *currentSimulatedPacket = pendingSimulatedPackets;
				currentSimulatedPacket != nullptr;
				currentSimulatedPacket = currentSimulatedPacket->next)
			{
				if (currentSimulatedPacket->next == nullptr || currentSimulatedPacket->next->receptionTime > receptionTime)
				{
					simulatedPacket->next = currentSimulatedPacket->next;
					currentSimulatedPacket->next = simulatedPacket;
					break;
				}
			}
		}

		pendingSimulatedPacketsCount++;
		simulatedPacketsReceived++;
	}
	else
	{
		simulatedPacketsDropped++;
	}
}

void ModuleNetworking::simulatedRealWorldConditions_ProcessQueuedPackets()
{
	while (pendingSimulatedPackets != nullptr)
	{
		SimulatedPacket *simulatedPacket = pendingSimulatedPackets;

		if (simulatedPacket->receptionTime <= Time.time)
		{
			receivedPacketsCount++;
			onPacketReceived(simulatedPacket->packet, simulatedPacket->fromAddress);

			pendingSimulatedPackets = simulatedPacket->next;
			simulatedPacket->next = freeSimulatedPackets;
			freeSimulatedPackets = simulatedPacket;

			ASSERT(pendingSimulatedPacketsCount > 0);
			pendingSimulatedPacketsCount--;
		}
		else
		{
			break;
		}
	}
}

void NetworkDisconnect()
{
	if (App->modNetClient->isConnected())
	{
		App->modNetClient->disconnect();
	}

	if (App->modNetServer->isConnected())
	{
		App->modNetServer->disconnect();
	}
}
