#include "Networks.h"
#include "DeliveryManager.h"

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
	packet << nextSequenceNumber;

	Delivery* delivery = new Delivery();

	delivery->dispatchTime = Time.time;
	delivery->sequenceNumber = nextSequenceNumber++;

	pendingDeliveries.push_back(delivery);

	return delivery;
}

bool DeliveryManager::processSequenceNumber(const InputMemoryStream& packet)
{
	uint32 receivedSequenceNumber = 0;
	packet >> receivedSequenceNumber;
	if (receivedSequenceNumber >= expectedSequenceNumber) {
		pendingAckDeliveries.push_back(receivedSequenceNumber);
		expectedSequenceNumber = receivedSequenceNumber + 1;
		return true;
	}

	return false;
}

bool DeliveryManager::hasSequenceNumbersPendingAck() const
{
	return !pendingAckDeliveries.empty();
}

void DeliveryManager::writeSequenceNumbersPendingAck(OutputMemoryStream& packet)
{
	for (auto& it : pendingAckDeliveries) {
		uint32 sequenceNum = it;
		packet << sequenceNum;
	}
	pendingAckDeliveries.clear();
}

void DeliveryManager::processAckSequenceNumbers(const InputMemoryStream& packet)
{
	while (packet.RemainingByteCount() > 0) {
		uint32 seqNumber = 0;
		packet >> seqNumber;
		bool deliveryFound = false;

		for (auto it = pendingDeliveries.begin(); it != pendingDeliveries.end(); it++)
		{
			if ((*it)->sequenceNumber == seqNumber) {
				if ((*it)->d_delegate) {
					(*it)->d_delegate->onDeliverySuccess(this);
				}
				pendingDeliveries.erase(it);
				deliveryFound = true;
				break;
			}
		}
		if (!deliveryFound) {
			//error: sequence number to acknowledge is missing!
			int i = 101;
		}
	}
}

void DeliveryManager::processTimedOutPackets()
{
	std::vector<int> deliveryIndexToDelete;
	for (int i = 0; i < pendingDeliveries.size(); i++)
	{
		if (Time.time - pendingDeliveries[i]->dispatchTime > PACKET_DELIVERY_TIMEOUT_SECONDS) {
			if (pendingDeliveries[i]->d_delegate)
			{
				pendingDeliveries[i]->d_delegate->onDeliveryFailure(this);
			}
			deliveryIndexToDelete.push_back(i);
		}
	}
	for (int i = deliveryIndexToDelete.size() - 1; i >= 0; i--)
	{
		pendingDeliveries.erase(pendingDeliveries.begin() + deliveryIndexToDelete[i]);
	}
	deliveryIndexToDelete.clear();
}

void DeliveryManager::clear()
{
	pendingDeliveries.clear();

	pendingAckDeliveries.clear();
	nextSequenceNumber = 0;
	expectedSequenceNumber = 0;
}