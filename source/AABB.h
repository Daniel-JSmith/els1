#pragma once

#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "GPUStructs.h"

// axis-aligned bounding box
class AABB
{
private:
	glm::vec3 cornerMin;
	glm::vec3 cornerMax;

public:
	AABB();
	AABB(const glm::vec3& cornerMin, const glm::vec3& cornerMax);
	AABB(const std::vector<glm::vec4>& vertexPositions, const Triangle& triangle);
	AABB(const std::vector<glm::vec4>& vertexPositions, const std::vector<Triangle>& trianlges, size_t start, size_t end);
	~AABB();

	const glm::vec3& getMinCorner() const;
	const glm::vec3& getMaxCorner() const;
};

