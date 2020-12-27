#include "Networks.h"
#include "ReplicationManagerClient.h"
#include "ModuleGameObject.h"

void ReplicationManagerClient::read(const InputMemoryStream& packet)
{
	while (packet.RemainingByteCount() > 0) {
		ReplicationAction action;
		uint32 networkID;
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
			if (go != nullptr) {
				readGO(go, packet, ReplicationAction::Update);
			}
			else {
				GameObject dummy;
				readDummy(&dummy, packet, ReplicationAction::Update);
			}
		}
		else if (action == ReplicationAction::Destroy)
		{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(networkID);
			if (go != nullptr) {
				App->modLinkingContext->unregisterNetworkGameObject(go);
				Destroy(go);
			}
		}
	}
}


