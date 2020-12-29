#pragma once

#include <vector>

class DeliveryManager;

class DeliveryDelegate {
public:

	virtual void onDeliverySuccess(DeliveryManager* deliveryManager) = 0;
	virtual void onDeliveryFailure(DeliveryManager* deliveryManager) = 0;
};

struct Delivery {
	uint32 sequenceNumber = 0;
	double dispatchTime = 0.0f;
	DeliveryDelegate* d_delegate = nullptr;

	~Delivery() {
		delete d_delegate;
	}
};

class DeliveryManager {
public:

	// For senders to write a new seq. numbers into a packet
	Delivery* writeSequenceNumber(OutputMemoryStream& packet, DeliveryDelegate* _delegate);

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

	uint32 nextSequenceNumber = 0;
	std::vector<Delivery*> pendingDeliveries;

	// Private members (receiver site)
	// - The next expected sequence number
	// - A list of sequence numbers pending ack

	uint32 expectedSequenceNumber = 0;
	std::vector<uint32> pendingAckDeliveries;
};