#include "ModuleNetworkingClient.h"
#include <string> 

//////////////////////////////////////////////////////////////////////
// ModuleNetworkingClient public methods
//////////////////////////////////////////////////////////////////////


void ModuleNetworkingClient::setServerAddress(const char * pServerAddress, uint16 pServerPort)
{
	serverAddressStr = pServerAddress;
	serverPort = pServerPort;
}

void ModuleNetworkingClient::setPlayerInfo(const char * pPlayerName, uint8 pSpaceshipType)
{
	playerName = pPlayerName;
	spaceshipType = pSpaceshipType;
}



//////////////////////////////////////////////////////////////////////
// ModuleNetworking virtual methods
//////////////////////////////////////////////////////////////////////

void ModuleNetworkingClient::onStart()
{
	if (!createSocket()) return;

	if (!bindSocketToPort(0)) {
		disconnect();
		return;
	}

	// Create remote address
	serverAddress = {};
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons(serverPort);
	int res = inet_pton(AF_INET, serverAddressStr.c_str(), &serverAddress.sin_addr);
	if (res == SOCKET_ERROR) {
		reportError("ModuleNetworkingClient::startClient() - inet_pton");
		disconnect();
		return;
	}

	state = ClientState::Connecting;

	inputDataFront = 0;
	inputDataBack = 0;

	secondsSinceLastHello = 9999.0f;
	secondsSinceLastInputDelivery = 0.0f;
	secondsSinceLastReceivedPacket = 0.0f;
	secondsSinceLastSendPacket = 0.0f;
	spawned = false;
	score = 0;
}

void ModuleNetworkingClient::onGui()
{
	if (state == ClientState::Stopped) return;

	if (ImGui::CollapsingHeader("ModuleNetworkingClient", ImGuiTreeNodeFlags_DefaultOpen))
	{
		if (state == ClientState::Connecting)
		{
			ImGui::Text("Connecting to server...");
		}
		else if (state == ClientState::Connected)
		{
			ImGui::Text("Connected to server");

			ImGui::Separator();

			ImGui::Text("Player info:");
			ImGui::Text(" - Id: %u", playerId);
			ImGui::Text(" - Name: %s", playerName.c_str());

			ImGui::Separator();

			ImGui::Text("Spaceship info:");
			ImGui::Text(" - Type: %u", spaceshipType);
			ImGui::Text(" - Network id: %u", networkId);
			ImGui::Text(" - Score: %u", score);

			vec2 playerPosition = {};
			GameObject *playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
			if (playerGameObject != nullptr) {
				playerPosition = playerGameObject->position;
			}
			ImGui::Text(" - Coordinates: (%f, %f)", playerPosition.x, playerPosition.y);

			ImGui::Separator();

			ImGui::Text("Connection checking info:");
			ImGui::Text(" - Ping interval (s): %f", PING_INTERVAL_SECONDS);
			ImGui::Text(" - Disconnection timeout (s): %f", DISCONNECT_TIMEOUT_SECONDS);

			ImGui::Separator();

			ImGui::Text("Input:");
			ImGui::InputFloat("Delivery interval (s)", &inputDeliveryIntervalSeconds, 0.01f, 0.1f, 4);
		}
	}
	if (ImGui::Begin("Score")) {
		ImGui::Text("[Players]");
		for (int i = 0; i < MAX_CLIENTS; ++i) {
			if (scoreClients[i].name != "") {
				std::string s = scoreClients[i].name + ": " + std::to_string(scoreClients[i].score);
				ImGui::Text("%s", s.c_str());
			}
		}
		ImGui::End();
	}
}

void ModuleNetworkingClient::onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress)
{
	// TODO(you): UDP virtual connection lab session

	uint32 protoId;
	packet >> protoId;
	if (protoId != PROTOCOL_ID) return;

	ServerMessage message;
	packet >> message;

	if (state == ClientState::Connecting)
	{
		if (message == ServerMessage::Welcome)
		{
			packet >> playerId;
			packet >> networkId;

			LOG("ModuleNetworkingClient::onPacketReceived() - Welcome from server");
			state = ClientState::Connected;
		}
		else if (message == ServerMessage::Unwelcome)
		{
			if (packet.RemainingByteCount() > 0) {
				App->modScreen->screenMainMenu->showInvalidUserName = true;
				WLOG("Name already taken - Change your PlayerName :-(");
			}
			else {
				WLOG("ModuleNetworkingClient::onPacketReceived() - Unwelcome from server :-(");
			}
			disconnect();
		}
	}
	else if (state == ClientState::Connected)
	{
		if (message == ServerMessage::Ping)
		{
			secondsSinceLastReceivedPacket = 0.0f;
		}
		else if (message == ServerMessage::Replication)
		{
			ReplicationManagerClient manager;
			uint32 currentRequest = 0;
			packet >> currentRequest;

			manager.read(packet);
			InputReconciliation(currentRequest, packet);
		}
		else if (message == ServerMessage::Score)
		{
			int c = 0;
			while (packet.RemainingByteCount() > 0) {
				packet >> scoreClients[c].name;
				packet >> scoreClients[c].score;
				c++;
			}

			for (int i = c; i < MAX_CLIENTS; ++i) {
				scoreClients[i].name = "";
				scoreClients[i].score = 0;
			}
			std::sort(std::begin(scoreClients), std::end(scoreClients), [](const ScoreClient& a, const ScoreClient& b) {return a.score > b.score; });
		}
	}
}

void ModuleNetworkingClient::onUpdate()
{
	if (state == ClientState::Stopped) return;

	// TODO(you): UDP virtual connection lab session
	if (state == ClientState::Connecting)
	{
		secondsSinceLastHello += Time.deltaTime;

		if (secondsSinceLastHello > 0.1f)
		{
			secondsSinceLastHello = 0.0f;

			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ClientMessage::Hello;
			packet << playerName;
			packet << spaceshipType;

			sendPacket(packet, serverAddress);
		}
	}
	else if (state == ClientState::Connected)
	{
		secondsSinceLastReceivedPacket += Time.deltaTime;

		if (secondsSinceLastReceivedPacket > DISCONNECT_TIMEOUT_SECONDS)
		{
			disconnect();
		}

		//Predicts Input
		GameObject* go = App->modLinkingContext->getNetworkGameObject(networkId);
		if (go != nullptr)
			if(go->behaviour != nullptr)
				go->behaviour->onInput(Input);

		// Process more inputs if there's space
		if (inputDataBack - inputDataFront < ArrayCount(inputData))
		{
			// Pack current input
			uint32 currentInputData = inputDataBack++;
			InputPacketData &inputPacketData = inputData[currentInputData % ArrayCount(inputData)];
			inputPacketData.sequenceNumber = currentInputData;
			inputPacketData.horizontalAxis = Input.horizontalAxis;
			inputPacketData.verticalAxis = Input.verticalAxis;
			inputPacketData.buttonBits = packInputControllerButtons(Input);
		}

		secondsSinceLastInputDelivery += Time.deltaTime;

		// Input delivery interval timed out: create a new input packet
		if (secondsSinceLastInputDelivery > inputDeliveryIntervalSeconds)
		{
			secondsSinceLastInputDelivery = 0.0f;

			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ClientMessage::Input;

			// TODO(you): Reliability on top of UDP lab session
			for (uint32 i = inputDataFront; i < inputDataBack; ++i)
			{
				InputPacketData &inputPacketData = inputData[i % ArrayCount(inputData)];
				packet << inputPacketData.sequenceNumber;
				packet << inputPacketData.horizontalAxis;
				packet << inputPacketData.verticalAxis;
				packet << inputPacketData.buttonBits;
			}
			//Delivery* delivery = deliveryManager.writeSequenceNumber(packet);
			
			sendPacket(packet, serverAddress);
		}

		secondsSinceLastSendPacket += Time.deltaTime;

		if (secondsSinceLastSendPacket >= PING_INTERVAL_SECONDS)
		{
			OutputMemoryStream packet;
			packet << PROTOCOL_ID;
			packet << ClientMessage::Ping;

			sendPacket(packet, serverAddress);

			secondsSinceLastSendPacket = 0.0f;
		}

		// Update camera for player
		GameObject *playerGameObject = App->modLinkingContext->getNetworkGameObject(networkId);
		if (playerGameObject != nullptr)
		{
			App->modRender->cameraPosition = playerGameObject->position;
			spawned = true;
		}
		else if(spawned && playerGameObject == nullptr)
		{
			disconnect();
		}
	}
}

void ModuleNetworkingClient::onConnectionReset(const sockaddr_in & fromAddress)
{
	disconnect();
}

void ModuleNetworkingClient::onDisconnect()
{
	state = ClientState::Stopped;

	GameObject *networkGameObjects[MAX_NETWORK_OBJECTS] = {};
	uint16 networkGameObjectsCount;
	App->modLinkingContext->getNetworkGameObjects(networkGameObjects, &networkGameObjectsCount);
	App->modLinkingContext->clear();

	for (uint32 i = 0; i < networkGameObjectsCount; ++i)
	{
		Destroy(networkGameObjects[i]);
	}

	App->modRender->cameraPosition = {};
}

void ModuleNetworkingClient::InputReconciliation(const uint32& currentRequest, const InputMemoryStream& packet) {
	GameObject* go = App->modLinkingContext->getNetworkGameObject(networkId);

	if (go != nullptr) {
		if (currentRequest > inputDataFront)
		{
			InputController ServerInputs;
			for (uint32 i = inputDataFront; i < currentRequest; ++i)
			{
				InputPacketData& inputPacketData = inputData[i % ArrayCount(inputData)];
				inputControllerFromInputPacketData(inputPacketData, ServerInputs);

				if (go->behaviour)
					go->behaviour->onInput(ServerInputs);
			}

			inputDataFront = currentRequest;
		}
	}
}