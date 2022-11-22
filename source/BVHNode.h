#pragma once

#include <vector>
#include "AABB.h"
#include "GPUStructs.h"

class BVHNode
{
private:
	AABB box;
	BVHNode* left;
	BVHNode* right;
	int indexOfPrimitive;

	void destroyHelper(BVHNode* node);

	void toArrayHelper(std::vector<BVHNodeStruct>& arr, const BVHNode* node, int i) const;

public:
	BVHNode();
	// start inclusive; end exclusive
	BVHNode(const std::vector<glm::vec4>& vertexPositions, std::vector<Triangle>& triangles, size_t start, size_t end);
	~BVHNode();

	void destroy();

	BVHNodeStruct getStruct() const;

	// expects arr to be at least the length of the BVH
	void toStructArray(std::vector<BVHNodeStruct>& arr) const;

	BVHNode& operator=(const BVHNode& other);
};

