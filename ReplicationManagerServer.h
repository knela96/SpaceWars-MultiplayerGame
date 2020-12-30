#pragma once
#include <map>
#include "DeliveryManager.h"
enum class ReplicationAction
{
	None,Create,Update,Destroy
};

class ReplicationDeliveryDelegate;

struct ReplicationCommand
{
	ReplicationCommand(){}
	ReplicationCommand(ReplicationAction _action, uint32 _networkID) {
		action = _action;
		networkId = _networkID;
	}

	ReplicationAction action;
	uint32 networkId;
};

class ReplicationManagerServer
{
public:
	void create(uint32 networkId);
	void update(uint32 networkId);
	void destroy(uint32 networkId);

	void write(OutputMemoryStream & packet, ReplicationDeliveryDelegate* _delegate);

	std::map<uint32, ReplicationAction> commands;
};

class ReplicationDeliveryDelegate :public DeliveryDelegate {
public: 

	ReplicationDeliveryDelegate(ReplicationManagerServer* replicationManager);

	void onDeliverySuccess(DeliveryManager* deliveryManager) override;
	void onDeliveryFailure(DeliveryManager* deliveryManager) override;

	void AddCommand(ReplicationCommand command);

private:

	ReplicationManagerServer* replicationManagerServer;
	std::vector<ReplicationCommand> replicationCommands;

};

class InputDeliveryDelegate :public DeliveryDelegate {
	void onDeliverySuccess(DeliveryManager* deliveryManager) override;
	void onDeliveryFailure(DeliveryManager* deliveryManager) override;
};