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

	// advanced distance between two entities based on their circle colliders
	inline float DistanceCircleCircle(const Entity* a, const Entity* b) 
	{
		float centerDist = Vector2Distance(a->getPosition(), b->getPosition());
		return centerDist - (a->getCircleCollider().radius + b->getCircleCollider().radius);
	}

	inline float DistanceEntities(const Entity* a, const Entity* b) 
	{
		return DistanceCircleCircle(a, b);

    	//return FLT_MAX;
	}
}
