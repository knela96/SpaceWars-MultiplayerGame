#pragma once
#include <map>
#include "DeliveryManager.h"
enum class ReplicationAction
{
	None,Create,Update,Destroy
};

struct ReplicationCommand
{
	ReplicationAction action;
	uint32 networkId;
};

class ReplicationManagerServer
{
public:
	void create(uint32 networkId);
	void update(uint32 networkId);
	void destroy(uint32 networkId);

	void write(OutputMemoryStream & packet);

	std::map<uint32, ReplicationAction> commands;
};

class ReplicationDeliveryDelegate :public DeliveryDelegate {
	void onDeliverySuccess(DeliveryManager* deliveryManager) override;
	void onDeliveryFailure(DeliveryManager* deliveryManager)override;

};