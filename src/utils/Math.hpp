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

	// calculate color of the health bar based on health of entity
	// green -> yellow -> red
	inline Color HealthToColor(float healthRatio) 
	{
		healthRatio = std::clamp(healthRatio, 0.0f, 1.0f);

		if (healthRatio > 0.5f) {
			// green -> yellow
			float t = (healthRatio - 0.5f) / 0.5f;

			return Color{
				(unsigned char)(255 * (1.0f - t)), // R
				255,                               // G
				0,                                 // B
				255								   // A
			};
		} else {
			// yellow -> red
			float t = healthRatio / 0.5f;

			return Color{
				255,                               // R
				(unsigned char)(255 * t),          // G
				0,                                 // B
				255								   // A
			};
		}
	}
}