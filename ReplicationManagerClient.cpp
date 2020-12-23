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

			readGO(go, packet, ReplicationAction::Create);
		}
		else if (action == ReplicationAction::Update)
		{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(networkID);
			readGO(go, packet, ReplicationAction::Update);
		}
		else if (action == ReplicationAction::Destroy)
		{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(networkID);
			App->modLinkingContext->unregisterNetworkGameObject(go);
			Destroy(go);
		}
	}
}
