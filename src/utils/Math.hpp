#pragma once 

#include <cmath>
#include <raymath.h>
#include <vector>

#include "../core/Entity.hpp"
#include <cfloat>

namespace math
{
	// simple distance between two points
	inline float distance(Vector2 pos1, Vector2 pos2)
	{
		return Vector2Distance(pos1, pos2);
	}

	// advanced distance between two entities based on their colliders
	inline float DistanceCircleRect(const Entity* circle, const Entity* rect) 
	{
		Vector2 delta = {
			fabs(circle->getPosition().x - rect->getPosition().x),
			fabs(circle->getPosition().y - rect->getPosition().y)
		};

		Vector2 closest = {
			fmaxf(delta.x - rect->getRectCollider().halfSize.x, 0.0f),
			fmaxf(delta.y - rect->getRectCollider().halfSize.y, 0.0f)
		};

		float dist = sqrtf(closest.x * closest.x + closest.y * closest.y);
		return dist - circle->getCircleCollider().radius;
	}

	inline float DistanceCircleCircle(const Entity* a, const Entity* b) 
	{
		float centerDist = Vector2Distance(a->getPosition(), b->getPosition());
		return centerDist - (a->getCircleCollider().radius + b->getCircleCollider().radius);
	}

	inline float DistanceRectRect(const Entity* a, const Entity* b) 
	{
		float dx = fabs(a->getPosition().x - b->getPosition().x)
				- (a->getRectCollider().halfSize.x + b->getRectCollider().halfSize.x);

		float dy = fabs(a->getPosition().y - b->getPosition().y)
				- (a->getRectCollider().halfSize.y + b->getRectCollider().halfSize.y);

		float clampedX = fmaxf(dx, 0.0f);
		float clampedY = fmaxf(dy, 0.0f);

		return sqrtf(clampedX * clampedX + clampedY * clampedY);
	}


	inline float DistanceEntities(const Entity* a, const Entity* b) 
	{
		if (a->getCollidorType() == ColliderType::Circle &&
			b->getCollidorType() == ColliderType::Circle)
			return DistanceCircleCircle(a, b);

		if (a->getCollidorType() == ColliderType::Circle &&
			b->getCollidorType() == ColliderType::Rect)
			return DistanceCircleRect(a, b);

		if (a->getCollidorType() == ColliderType::Rect &&
			b->getCollidorType() == ColliderType::Circle)
			return DistanceCircleRect(b, a);

		if (a->getCollidorType() == ColliderType::Rect &&
			b->getCollidorType() == ColliderType::Rect)
			return DistanceRectRect(a, b);

    	return FLT_MAX;
	}
}
