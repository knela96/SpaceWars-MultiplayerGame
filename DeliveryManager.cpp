#include "Networks.h"
#include "DeliveryManager.h"

Delivery* DeliveryManager::writeSequenceNumber(OutputMemoryStream& packet)
{
	Delivery* delivery = new Delivery();


	delivery->dispatchTime = Time.time;
	delivery->sequenceNumber = ++deliveryCount;

	packet << delivery->sequenceNumber;

	deliveries.push_back(delivery);

	return delivery;
}

