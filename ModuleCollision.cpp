#include "ModuleCollision.h"


static bool collisionTestOverSeparatingAxis(
	vec2 a1, vec2 a2, vec2 a3, vec2 a4, // Points in box a
	vec2 b1, vec2 b2, vec2 b3, vec2 b4, // Points in box b
	vec2 axis)                          // Separating axis test
{
	float pa1 = dot(a1, axis);
	float pa2 = dot(a2, axis);
	float pa3 = dot(a3, axis);
	float pa4 = dot(a4, axis);
	float pb1 = dot(b1, axis);
	float pb2 = dot(b2, axis);
	float pb3 = dot(b3, axis);
	float pb4 = dot(b4, axis);
	float maxa = max(pa1, max(pa2, max(pa3, pa4)));
	float mina = min(pa1, min(pa2, min(pa3, pa4)));
	float maxb = max(pb1, max(pb2, max(pb3, pb4)));
	float minb = min(pb1, min(pb2, min(pb3, pb4)));
	bool separated = maxa < minb || mina > maxb;

	return !separated;
}

static bool collisionTest(CollisionData &c1, CollisionData &c2)
{
	BEGIN_TIMED_BLOCK(CollisionTest);

	bool areColliding = false;

	vec2 axes[] = { c1.p1 - c1.p2, c1.p2 - c1.p3, c2.p1 - c2.p2, c2.p2 - c2.p3 };

	for (vec2 axis : axes)
	{
		areColliding = collisionTestOverSeparatingAxis(c1.p1, c1.p2, c1.p3, c1.p4, c2.p1, c2.p2, c2.p3, c2.p4, axis);

		if (!areColliding)
		{
			break;
		}
	}

	END_TIMED_BLOCK(CollisionTest);

	return areColliding;
}

Collider * ModuleCollision::addCollider(ColliderType type, GameObject * parent)
{
	ASSERT(type != ColliderType::None);
	ASSERT(parent->sprite != nullptr);

	for (Collider &collider : colliders)
	{
		if (collider.type == ColliderType::None)
		{
			collider.type = type;
			collider.gameObject = parent;
			collidersCount++;
			return &collider;
		}
	}

	ASSERT(0); // No space for more colliders, increase MAX_COLLIDERS

	return nullptr;
}

void ModuleCollision::removeCollider(Collider *collider)
{
	if (collider->type != ColliderType::None)
	{
		collider->type = ColliderType::None;
		collider->gameObject->collider = nullptr;
		collider->gameObject = nullptr;
		collidersCount--;
	}
}

bool ModuleCollision::update()
{
	BEGIN_TIMED_BLOCK(Collisions);

	BEGIN_TIMED_BLOCK(Collisions1);
	// Pack colliders in activeColliders contiguously
	uint32 activeCollidersCount = 0;
	for (unsigned int i = 0; i < MAX_COLLIDERS && activeCollidersCount < collidersCount; ++i)
	{
		Collider *collider = &colliders[i];

		if (collider->type != ColliderType::None)
		{
			// Handle game object destruction
			GameObject *go = collider->gameObject;
			ASSERT(go != nullptr);

			if (go->state == GameObject::DESTROYING)
			{
				App->modCollision->removeCollider(collider);
				continue;
			}

			if (go->state == GameObject::UPDATING)
			{
				// Precompute collision data and store it into activeColliders
				Sprite *sprite = go->sprite;
				ASSERT(sprite != nullptr);

				vec2 size = isZero(go->size) ? (sprite->texture ? sprite->texture->size : vec2{ 100.0f, 100.0f }) : go->size;

				mat4 aWorldMatrix =
					translation(go->position) *
					rotationZ(radiansFromDegrees(go->angle)) *
					scaling(size) *
					translation(vec2{ 0.5f, 0.5f } -sprite->pivot);

				activeColliders[activeCollidersCount].collider = collider;
				activeColliders[activeCollidersCount].p1 = vec2_cast(aWorldMatrix * vec4{ -0.5f, -0.5f, 0.0f, 1.0f });
				activeColliders[activeCollidersCount].p2 = vec2_cast(aWorldMatrix * vec4{ 0.5f, -0.5f, 0.0f, 1.0f });
				activeColliders[activeCollidersCount].p3 = vec2_cast(aWorldMatrix * vec4{ 0.5f,  0.5f, 0.0f, 1.0f });
				activeColliders[activeCollidersCount].p4 = vec2_cast(aWorldMatrix * vec4{ -0.5f,  0.5f, 0.0f, 1.0f });
				activeColliders[activeCollidersCount].behaviour = (collider->isTrigger) ? collider->gameObject->behaviour : nullptr;
				activeCollidersCount++;
			}
		}
	}
	END_TIMED_BLOCK(Collisions1);

	BEGIN_TIMED_BLOCK(Collisions2);
	// Traverse all active colliders
	for (uint32 i = 0; i < activeCollidersCount; ++i)
	{
		CollisionData &c1 = activeColliders[i];

		for (uint32 j = i + 1; j < activeCollidersCount; ++j)
		{
			CollisionData &c2 = activeColliders[j];

			if ((c1.behaviour != nullptr) ||
				(c2.behaviour != nullptr))
			{
				if (collisionTest(c1, c2))
				{
					if (c1.behaviour)
					{
						c1.behaviour->onCollisionTriggered(*c1.collider, *c2.collider);
					}
					if (c2.behaviour)
					{
						c2.behaviour->onCollisionTriggered(*c2.collider, *c1.collider);
					}
				}
			}
		}
	}
	END_TIMED_BLOCK(Collisions2);

	END_TIMED_BLOCK(Collisions);
	return true;
}

bool ModuleCollision::postUpdate()
{
	return true;
}
