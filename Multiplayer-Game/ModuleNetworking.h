#pragma once

class ModuleNetworking : public Module
{
public:

	virtual bool isServer() const { return false; }

	virtual bool isClient() const { return false; }

	bool isConnected() const { return socket != INVALID_SOCKET; }

	void disconnect();



protected:

	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking protected members
	//////////////////////////////////////////////////////////////////////

	SOCKET socket = INVALID_SOCKET;

	bool createSocket();

	bool bindSocketToPort(int port);

	void sendPacket(const OutputMemoryStream &packet, const sockaddr_in &destAddress);

	void sendPacket(const char *data, uint32 size, const sockaddr_in &destAddress);

	void reportError(const char *message);



private:

	//////////////////////////////////////////////////////////////////////
	// Module virtual methods
	//////////////////////////////////////////////////////////////////////

	bool init() override;

	bool start() override;

	bool preUpdate() override;

	bool update() override;

	bool gui() override;

	bool stop() override;

	bool cleanUp() override;



	//////////////////////////////////////////////////////////////////////
	// ModuleNetworking methods
	//////////////////////////////////////////////////////////////////////

	uint32 sentPacketsCount = 0;
	uint32 receivedPacketsCount = 0;

	void processIncomingPackets();

	virtual void onStart() = 0;

	virtual void onGui() = 0;

	virtual void onPacketReceived(const InputMemoryStream &packet, const sockaddr_in &fromAddress) = 0;

	virtual void onUpdate() = 0;

	virtual void onConnectionReset(const sockaddr_in &fromAddress) = 0;

	virtual void onDisconnect() = 0;



private:

	//////////////////////////////////////////////////////////////////////
	// Real world conditions simulation
	//////////////////////////////////////////////////////////////////////

	bool simulateLatency = false;
	float simulatedLatency = 0.07f;
	float simulatedJitter = 0.03f;
	bool simulateDrops = false;
	float simulatedDropRatio = 0.01f;
	uint32 simulatedPacketsReceived = 0;
	uint32 simulatedPacketsDropped = 0;

	void simulatedRealWorldConditions_Init();

	void simulatedRealWorldConditions_EnqueuePacket(const InputMemoryStream &packet, const sockaddr_in &fromAddress);

	void simulatedRealWorldConditions_ProcessQueuedPackets();

	struct SimulatedPacket {
		InputMemoryStream packet;
		sockaddr_in fromAddress;
		double receptionTime;
		SimulatedPacket *next = nullptr;
	};

	static const int MAX_SIMULATED_PACKETS = 128;
	SimulatedPacket simulatedPackets[MAX_SIMULATED_PACKETS];
	SimulatedPacket *freeSimulatedPackets = nullptr;
	SimulatedPacket *pendingSimulatedPackets = nullptr;
	RandomNumberGenerator simulatedRandom;
	uint32 pendingSimulatedPacketsCount = 0;
};

void NetworkDisconnect();
