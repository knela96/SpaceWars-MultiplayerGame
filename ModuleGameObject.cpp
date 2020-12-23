#include "Networks.h"
#include "ModuleGameObject.h"

bool ModuleGameObject::init()
{
	return true;
}

bool ModuleGameObject::preUpdate()
{
	BEGIN_TIMED_BLOCK(GOPreUpdate);

	static const GameObject::State gNextState[] = {
		GameObject::NON_EXISTING, // After NON_EXISTING
		GameObject::STARTING,     // After INSTANTIATE
		GameObject::UPDATING,     // After STARTING
		GameObject::UPDATING,     // After UPDATING
		GameObject::DESTROYING,   // After DESTROY
		GameObject::NON_EXISTING  // After DESTROYING
	};

	for (GameObject &gameObject : gameObjects)
	{
		gameObject.state = gNextState[gameObject.state];
	}

	END_TIMED_BLOCK(GOPreUpdate);

	return true;
}

bool ModuleGameObject::update()
{
	// Delayed destructions
	for (DelayedDestroyEntry &destroyEntry : gameObjectsWithDelayedDestruction)
	{
		if (destroyEntry.object != nullptr)
		{
			destroyEntry.delaySeconds -= Time.deltaTime;
			if (destroyEntry.delaySeconds <= 0.0f)
			{
				Destroy(destroyEntry.object);
				destroyEntry.object = nullptr;
			}
		}
	}

	return true;
}

bool ModuleGameObject::postUpdate()
{
	return true;
}

bool ModuleGameObject::cleanUp()
{
	return true;
}

GameObject * ModuleGameObject::Instantiate()
{
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		GameObject &gameObject = App->modGameObject->gameObjects[i];

		if (gameObject.state == GameObject::NON_EXISTING)
		{
			gameObject = GameObject();
			gameObject.id = i;
			gameObject.state = GameObject::INSTANTIATE;
			return &gameObject;
		}
	}

	ASSERT(0); // NOTE(jesus): You need to increase MAX_GAME_OBJECTS in case this assert crashes
	return nullptr;
}

void ModuleGameObject::Destroy(GameObject * gameObject)
{
	ASSERT(gameObject->networkId == 0); // NOTE(jesus): If it has a network identity, it must be destroyed by the Networking module first

	static const GameObject::State gNextState[] = {
		GameObject::NON_EXISTING, // After NON_EXISTING
		GameObject::DESTROY,      // After INSTANTIATE
		GameObject::DESTROY,      // After STARTING
		GameObject::DESTROY,      // After UPDATING
		GameObject::DESTROY,      // After DESTROY
		GameObject::DESTROYING    // After DESTROYING
	};

	ASSERT(gameObject->state < GameObject::STATE_COUNT);
	gameObject->state = gNextState[gameObject->state];
}

void ModuleGameObject::Destroy(GameObject * gameObject, float delaySeconds)
{
	for (uint32 i = 0; i < MAX_GAME_OBJECTS; ++i)
	{
		if (App->modGameObject->gameObjectsWithDelayedDestruction[i].object == nullptr)
		{
			App->modGameObject->gameObjectsWithDelayedDestruction[i].object = gameObject;
			App->modGameObject->gameObjectsWithDelayedDestruction[i].delaySeconds = delaySeconds;
			break;
		}
	}
}

GameObject * Instantiate()
{
	GameObject *result = ModuleGameObject::Instantiate();
	return result;
}

void Destroy(GameObject * gameObject)
{
	ModuleGameObject::Destroy(gameObject);
}

void Destroy(GameObject * gameObject, float delaySeconds)
{
	ModuleGameObject::Destroy(gameObject, delaySeconds);
}

void writeGO(GameObject* go, OutputMemoryStream& packet, ReplicationAction action)
{
	switch (action) {
	case ReplicationAction::Create:
		packet << go->position.x;
		packet << go->position.y;
		packet << go->angle;
		packet << go->size.x;
		packet << go->size.y;
		packet << go->tag;
		packet << std::string(go->sprite->texture->filename);
		packet << go->sprite->order;
		packet << go->sprite->color.r;
		packet << go->sprite->color.g;
		packet << go->sprite->color.b;
		packet << go->sprite->color.a;
		if (go->collider)
			packet << go->collider->type;
		else
			packet << ColliderType::None;

		if (go->animation)
			packet << true;
		break;
	case ReplicationAction::Update:
		packet << go->position.x;
		packet << go->position.y;
		packet << go->angle;
		go->behaviour->write(packet);
		break;
	};
}

void readGO(GameObject* go, const InputMemoryStream& packet, ReplicationAction action)
{
	switch (action) {
	case ReplicationAction::Create:
		ProcessCreatePacket(go, packet);
		break;
	case ReplicationAction::Update:
		packet >> go->position.x;
		packet >> go->position.y;
		packet >> go->angle;
		go->behaviour->read(packet);
		break;
	};
}


void ProcessCreatePacket(GameObject * gameObject, const InputMemoryStream & packet) {

	// Create a new game object with the player properties
	packet >> gameObject->position.x;
	packet >> gameObject->position.y;
	packet >> gameObject->angle;
	packet >> gameObject->size.x;
	packet >> gameObject->size.y;
	packet >> gameObject->tag;

	std::string texture_filename;
	packet >> texture_filename;

	gameObject->sprite = App->modRender->addSprite(gameObject);
	gameObject->sprite->order = 5;
	if (!std::strcmp(texture_filename.c_str(), App->modResources->spacecraft1->filename)) {
		gameObject->sprite->texture = App->modResources->spacecraft1;
	}
	else if (!std::strcmp(texture_filename.c_str(), App->modResources->spacecraft2->filename)) {
		gameObject->sprite->texture = App->modResources->spacecraft2;
	}
	else if (!std::strcmp(texture_filename.c_str(), App->modResources->spacecraft3->filename)) {
		gameObject->sprite->texture = App->modResources->spacecraft3;
	}
	else if (!std::strcmp(texture_filename.c_str(), App->modResources->laser->filename)) {
		gameObject->sprite->texture = App->modResources->laser;
	}
	else if (!std::strcmp(texture_filename.c_str(), App->modResources->explosion1->filename)) {
		gameObject->sprite->texture = App->modResources->explosion1;
	}

	packet >> gameObject->sprite->order;

	packet >> gameObject->sprite->color.r;
	packet >> gameObject->sprite->color.g;
	packet >> gameObject->sprite->color.b;
	packet >> gameObject->sprite->color.a;

	ColliderType col_type;
	packet >> col_type;

	if (col_type == ColliderType::Player) {
		gameObject->collider = App->modCollision->addCollider(ColliderType::Player, gameObject);
		gameObject->collider->isTrigger = true; // NOTE(jesus): This object will receive onCollisionTriggered events

		// Create behaviour
		Spaceship* spaceshipBehaviour = App->modBehaviour->addSpaceship(gameObject);
		gameObject->behaviour = spaceshipBehaviour;
		gameObject->behaviour->isServer = false;
	}
	else if (col_type == ColliderType::Laser) {
		gameObject->collider = App->modCollision->addCollider(ColliderType::Laser, gameObject);
		gameObject->collider->isTrigger = true; // NOTE(jesus): This object will receive onCollisionTriggered events

		// Create behaviour
		Laser* laserBehaviour = App->modBehaviour->addLaser(gameObject);
		laserBehaviour->isServer = false;
		gameObject->behaviour = laserBehaviour;
	}
	else if (col_type == ColliderType::None) {
		bool hasAnimation = false;
		packet >> hasAnimation;

		if (hasAnimation) {
			gameObject->animation = App->modRender->addAnimation(gameObject);
			gameObject->animation->clip = App->modResources->explosionClip;
			App->modSound->playAudioClip(App->modResources->audioClipExplosion);
		}
	}

}