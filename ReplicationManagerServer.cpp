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
		if (command.second == ReplicationAction::Create) {
			GameObject* go = App->modLinkingContext->getNetworkGameObject(command.first);
			packet << command.first;
			packet << command.second;
			packet << go->position.x;
			packet << go->position.y;
			packet << go->angle;
			packet << go->size.x;
			packet << go->size.y;
			packet << go->tag;
			packet << std::string(go->sprite->texture->filename);
			packet << go->sprite->color.r;
			packet << go->sprite->color.g;
			packet << go->sprite->color.b;
			packet << go->sprite->color.a;
			if (go->collider)
				packet << go->collider->type;
		}
		else if(command.second == ReplicationAction::Update)
		{
			GameObject* go = App->modLinkingContext->getNetworkGameObject(command.first);
			packet << command.first;
			packet << command.second;
			packet << go->position.x;
			packet << go->position.y;
			packet << go->angle;
		}
		else if (command.second == ReplicationAction::Destroy)
		{
			packet << command.first;
			packet << command.second;
		}
	}

	commands.clear();
}
