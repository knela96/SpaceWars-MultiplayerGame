#pragma once

struct Collider
{
	ColliderType type = ColliderType::None;
	GameObject *gameObject = nullptr;
	bool isTrigger = false;
};

struct CollisionData
{
	Collider *collider;   // The collider component itself
	Behaviour *behaviour; // The callbacks
	vec2 p1, p2, p3, p4;  // Transformed bounding box points
};

class ModuleCollision : public Module
{
public:

	///////////////////////////////////////////////////////////////////////
	// ModuleCollision public methods
	///////////////////////////////////////////////////////////////////////

	Collider *addCollider(ColliderType type, GameObject *parent);

	void removeCollider(Collider * collider);


private:

	///////////////////////////////////////////////////////////////////////
	// Module virtual methods
	///////////////////////////////////////////////////////////////////////

	bool update() override;

	bool postUpdate() override;

	uint32    collidersCount = 0;
	Collider  colliders[MAX_COLLIDERS];

	CollisionData activeColliders[MAX_COLLIDERS];

	friend class ModuleRender;
};

