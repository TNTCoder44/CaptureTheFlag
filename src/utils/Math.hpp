#pragma once 

#include <cmath>
#include <raymath.h>
#include <vector>

class Entity;

namespace math
{
	inline float distance(Vector2 pos1, Vector2 pos2)
	{
		return Vector2Distance(pos1, pos2);
	}

	inline Entity* bestEnt(std::vector<Entity*> entities, Vector2 position, float range)
	{
		float dist = 1000.f;
		Entity* bestEnt = (Entity*)nullptr;
		for (auto& entity : entities)
		{
			float newDist = distance(entity->getPosition(), position) <= range;

			if (newDist < dist)
			{
				dist = newDist;
				bestEnt = entity;
			}
		}

		if (bestEnt != nullptr)
			return bestEnt;
		return nullptr;
	}
}
