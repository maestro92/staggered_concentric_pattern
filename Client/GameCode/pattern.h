#pragma once

#include "../PlatformShared/platform_shared.h"
#include <vector>

struct Circle
{
	glm::vec2 center;
	float radius;
};

class Pattern
{
public:
	void Init()
	{
		float pointRadius = 20.0f;

		// Center of circle A
		centerA = glm::vec2{ 50, 0 };
		startingRadiusA = 1.0f; // Starting radius of circle A

		// Center of circle B
		centerB = glm::vec2{ -50, 0 };
		startingRadiusB = 1.0f; // Starting radius of circle B

		// Distance between concentric rings
		distBetweenCircles = pointRadius;

		int numSteps = 10; // 128
		// number of steps of A
		for (int i = 0; i < numSteps; i++)
		{
			float radiusA = startingRadiusA + distBetweenCircles * (float)i;

			// number of steps of B
			for (int j = 0; j < numSteps; j++)
			{
				float radiusB = startingRadiusB + distBetweenCircles * (float)j;

				//			float usedRadiusA = startingRadiusA + ((i % 3) ? 0.0f : 0.3f * distBetweenCircles);
				//			float usedRadiusB = startingRadiusB + ((j % 3) ? 0.0f : 0.3f * distBetweenCircles);

				float usedRadiusA = radiusA;
				float usedRadiusB = radiusB;

				circlesA.push_back({ centerA, usedRadiusA });
				circlesB.push_back({ centerB, usedRadiusB });

				// Intersect circle Ac,UseAr and Bc,UseBr
				// Add the resulting points if they are within the pattern bounds (the bounds were [[-1,1]] on both axes for all prior screenshots)
			}
		}
	}

	glm::vec2 centerA;
	glm::vec2 centerB;

	float startingRadiusA;
	float startingRadiusB;

	float distBetweenCircles;

	std::vector<Circle> circlesA;
	std::vector<Circle> circlesB;
};
