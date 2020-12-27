#pragma once

#include <vector>

// TODO(you): Reliability on top of UDP lab session
class DeliveryManager;

class DeliveryDelegate {
public:

	virtual void onDeliverySucces(DeliveryManager* deliveryManager) = 0;
	virtual void onDeliveryFailure(DeliveryManager* deliveryManager) = 0;
};

struct Delivery {
	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0f;
	DeliveryDelegate* delegate = nullptr;
};

class DeliveryManager {
public:

	// For senders to write a new seq. numbers into a packet
	Delivery* writeSequenceNumber(OutputMemoryStream& packet);

	//For receivers to process the seq. number from an incoming packet
	bool processSequenceNumber(const InputMemoryStream& packet);

	//For receivers to write ack'ed seq. numbers into a packet
	bool hasSequenceNumbersPendingAck() const;
	void writeSequenceNumbersPendingAck(OutputMemoryStream& packet);

	//For senders to process ack'ed seq. numbers from a packet
	void processAckSequenceNumbers(const InputMemoryStream& packet);
	void processTimedOutPackets();

	void clear();

private:

	/*
	Private members (sneder side)
	- the next outgoing sequence number
	- a list of pending deliveries

	Private members (receiver side)
	- The next expected sequence number
	- A list of sequence numbers pending ack
	*/
	uint32 deliveryCount = 0;
	std::vector<Delivery*> deliveries;
};