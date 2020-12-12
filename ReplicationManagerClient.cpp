#include "Networks.h"
#include "ReplicationManagerClient.h"
#include "ModuleGameObject.h"

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	while (packet.RemainingByteCount() > 0) {
		uint32 networkID;
		ReplicationAction action;
		packet >> networkID;
		packet >> action;

		if (action == ReplicationAction::Create) {
			GameObject* go = Instantiate();
			App->modLinkingContext->registerNetworkGameObjectWithNetworkId(go, networkID);

			ProcessCreatePacket(go, packet);
		}
		else if (action == ReplicationAction::Update)
		{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(networkID);

			packet >> go->position.x;
			packet >> go->position.y;
			packet >> go->angle;
		}
		else if (action == ReplicationAction::Destroy)
		{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(networkID);
			App->modLinkingContext->unregisterNetworkGameObject(go);
			Destroy(go);
		}
	}
}
