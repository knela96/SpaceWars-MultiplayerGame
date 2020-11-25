#pragma once

struct GameObject
{
	uint32 id;

	// Transform component
	vec2 position = vec2{ 0.0f, 0.0f };
	vec2 size = vec2{ 0.0f, 0.0f }; // NOTE(jesus): If equals 0, it takes the size of the texture
	float angle = 0.0f;

	// Render component
	Sprite *sprite = nullptr;
	Animation *animation = nullptr;

	// Collider component
	Collider *collider = nullptr;

	// "Script" component
	Behaviour *behaviour = nullptr;

	// Tag for custom usage
	uint32 tag = 0;

	// Network identity component
	uint32 networkId = 0;                    // NOTE(jesus): Only for network game objects
	bool networkInterpolationEnabled = true; // NOTE(jesus): Only for network game objects

	// NOTE(jesus): Don't use in gameplay systems (use Instantiate, Destroy instead)
	enum State {
		NON_EXISTING,
		INSTANTIATE,  // Only during the frame Instantiate() was called
		STARTING,     // One frame to allow modules do their needed update() actions
		UPDATING,     // Alive and updating
		DESTROY,      // Only during the frame Destroy() was called
		DESTROYING,   // One frame to allow modules do their needed update() actions
		STATE_COUNT
	};
	State state = NON_EXISTING;

private:

	void * operator new(size_t size) = delete;
	void operator delete (void *obj) = delete;
};

class ModuleGameObject : public Module
{
public:

	// Virtual functions

	bool init() override;

	bool preUpdate() override;

	bool update() override;

	bool postUpdate() override;

	bool cleanUp() override;

	static GameObject * Instantiate();

	static void Destroy(GameObject * gameObject);

	static void Destroy(GameObject * gameObject, float delaySeconds);



	GameObject gameObjects[MAX_GAME_OBJECTS] = {};

private:

	struct DelayedDestroyEntry
	{
		float delaySeconds = 0.0f;
		GameObject *object = nullptr;
	};

	DelayedDestroyEntry gameObjectsWithDelayedDestruction[MAX_GAME_OBJECTS];
};


// NOTE(jesus): These functions are named after Unity functions

GameObject *Instantiate();

void Destroy(GameObject *gameObject);

void Destroy(GameObject *gameObject, float delaySeconds);

inline bool IsValid(GameObject *gameObject)
{
	return
		gameObject != nullptr &&
		gameObject->state >= GameObject::INSTANTIATE &&
		gameObject->state <= GameObject::UPDATING;
}
