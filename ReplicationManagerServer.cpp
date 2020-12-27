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

void ReplicationManagerServer::write(OutputMemoryStream& packet)
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
			writeGO(go, packet, ReplicationAction::Update);
		}
	}

	commands.clear();
}

void ReplicationDeliveryDelegate::onDeliveryFailure(DeliveryManager* deliveryManager)
{

}


void ReplicationDeliveryDelegate::onDeliverySuccess(DeliveryManager* deliveryManager)
{

}