#include "AABB.h"



AABB::AABB()
{
	cornerMin = glm::vec3(0);
	cornerMax = glm::vec3(0);
}

AABB::AABB(const glm::vec3& cornerMin, const glm::vec3& cornerMax) : cornerMin(cornerMin), cornerMax(cornerMax)
{
	
}

AABB surroundingBox(AABB box1, AABB box2)
{
	glm::vec3 minCorner(fmin(box1.getMinCorner().x, box2.getMinCorner().x),
		fmin(box1.getMinCorner().y, box2.getMinCorner().y),
		fmin(box1.getMinCorner().z, box2.getMinCorner().z));

	glm::vec3 maxCorner(fmax(box1.getMaxCorner().x, box2.getMaxCorner().x),
		fmax(box1.getMaxCorner().y, box2.getMaxCorner().y),
		fmax(box1.getMaxCorner().z, box2.getMaxCorner().z));

	return AABB(minCorner, maxCorner);
}

AABB::AABB(const std::vector<glm::vec4>& vertexPositions, const Triangle& triangle)
{
	cornerMin = glm::vec3(0);
	cornerMin.x = vertexPositions[triangle.v0].x;
	cornerMin.x = fmin(cornerMin.x, vertexPositions[triangle.v1].x);
	cornerMin.x = fmin(cornerMin.x, vertexPositions[triangle.v2].x);
	cornerMin.y = vertexPositions[triangle.v0].y;
	cornerMin.y = fmin(cornerMin.y, vertexPositions[triangle.v1].y);
	cornerMin.y = fmin(cornerMin.y, vertexPositions[triangle.v2].y);
	cornerMin.z = vertexPositions[triangle.v0].z;
	cornerMin.z = fmin(cornerMin.z, vertexPositions[triangle.v1].z);
	cornerMin.z = fmin(cornerMin.z, vertexPositions[triangle.v2].z);

	cornerMax = glm::vec3(0);
	cornerMax.x = vertexPositions[triangle.v0].x;
	cornerMax.x = fmax(cornerMax.x, vertexPositions[triangle.v1].x);
	cornerMax.x = fmax(cornerMax.x, vertexPositions[triangle.v2].x);
	cornerMax.y = vertexPositions[triangle.v0].y;
	cornerMax.y = fmax(cornerMax.y, vertexPositions[triangle.v1].y);
	cornerMax.y = fmax(cornerMax.y, vertexPositions[triangle.v2].y);
	cornerMax.z = vertexPositions[triangle.v0].z;
	cornerMax.z = fmax(cornerMax.z, vertexPositions[triangle.v1].z);
	cornerMax.z = fmax(cornerMax.z, vertexPositions[triangle.v2].z);

	// add padding to ensure box has some volume. Ray box intersection has problems with 0-volume boxes
	cornerMin -= glm::vec3(0.001f, 0.001f, 0.001f);
	cornerMax += glm::vec3(0.001f, 0.001f, 0.001f);
}

AABB::AABB(const std::vector<glm::vec4>& vertexPositions, const std::vector<Triangle>& triangles, size_t start, size_t end)
{
	AABB tempBox = AABB(vertexPositions, triangles[start]);

	for (size_t i = start + 1; i < end; i++)
	{
		Triangle nextPrimitive = triangles[i];
		AABB nextBox = AABB(vertexPositions, nextPrimitive);
		tempBox = surroundingBox(tempBox, nextBox);
	}

	cornerMin = tempBox.cornerMin;
	cornerMax = tempBox.cornerMax;
}

AABB::~AABB()
{
}

const glm::vec3& AABB::getMinCorner() const
{
	return cornerMin;
}
const glm::vec3& AABB::getMaxCorner() const
{
	return cornerMax;
}
