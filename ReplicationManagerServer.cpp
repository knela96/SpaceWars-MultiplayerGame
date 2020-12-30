#include "Networks.h"
#include "ReplicationManagerServer.h"

void ReplicationManagerServer::create(uint32 networkId)
{
	commands[networkId] = ReplicationAction::Create;
}

void ReplicationManagerServer::update(uint32 networkId)
{
	commands[networkId] = ReplicationAction::Update;
}

void ReplicationManagerServer::destroy(uint32 networkId)
{
	commands[networkId] = ReplicationAction::Destroy;
}

void ReplicationManagerServer::write(OutputMemoryStream& packet, ReplicationDeliveryDelegate* _delegate)
{
	for (auto command : commands)
	{
		packet << command.first;
		packet << command.second;

		if (command.second == ReplicationAction::Create) {
			GameObject* go = App->modLinkingContext->getNetworkGameObject(command.first);
			writeGO(go, packet, ReplicationAction::Create);
		}
		else if(command.second == ReplicationAction::Update)
		{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(command.first);
			if(go)
			writeGO(go, packet, ReplicationAction::Update);
		}
		_delegate->AddCommand(ReplicationCommand(command.second/*action*/, command.first/*networkID*/));
	}
	commands.clear();
	
}

ReplicationDeliveryDelegate::ReplicationDeliveryDelegate(ReplicationManagerServer* replicationManager)
{
	replicationManagerServer = replicationManager;
}

void ReplicationDeliveryDelegate::onDeliveryFailure(DeliveryManager* deliveryManager)
{
	for (const ReplicationCommand& replicationCommand : replicationCommands)
	{
		switch (replicationCommand.action)
		{
		case ReplicationAction::Create:
			if (App->modLinkingContext->getNetworkGameObject(replicationCommand.networkId) != nullptr)
				replicationManagerServer->create(replicationCommand.networkId);
			break;
		case ReplicationAction::Update:
			if (App->modLinkingContext->getNetworkGameObject(replicationCommand.networkId) != nullptr)
				replicationManagerServer->destroy(replicationCommand.networkId);
			break;
		case ReplicationAction::Destroy:
			if (App->modLinkingContext->getNetworkGameObject(replicationCommand.networkId) == nullptr)
				replicationManagerServer->update(replicationCommand.networkId);
			break;
		}
	}
	replicationCommands.clear();
}

void ReplicationDeliveryDelegate::onDeliverySuccess(DeliveryManager* deliveryManager)
{

}

void ReplicationDeliveryDelegate::AddCommand(ReplicationCommand command)
{
	replicationCommands.push_back(command);
}

void InputDeliveryDelegate::onDeliveryFailure(DeliveryManager* deliveryManager)
{	 
	 
}	 
	 
void InputDeliveryDelegate::onDeliverySuccess(DeliveryManager* deliveryManager)
{

}