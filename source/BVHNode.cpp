#include "BVHNode.h"
#include <iostream>
#include <algorithm>
#include "GPUStructs.h"



BVHNode::BVHNode() : indexOfPrimitive(-1), left(nullptr), right(nullptr), box(AABB{})
{
}

const std::vector<glm::vec4>* positions;

inline bool compareSpherePosition(const Sphere& sphere1, const Sphere& sphere2, glm::vec3 axis)
{
	return glm::dot(glm::vec3(sphere1.position), axis) < glm::dot(glm::vec3(sphere2.position), axis);
}

bool compareSphereX(const Sphere& sphere1, const Sphere& sphere2)
{
	return compareSpherePosition(sphere1, sphere2, glm::vec3(1, 0, 0));
}

bool compareSphereY(const Sphere& sphere1, const Sphere& sphere2)
{
	return compareSpherePosition(sphere1, sphere2, glm::vec3(0, 1, 0));
}

bool compareSphereZ(const Sphere& sphere1, const Sphere& sphere2)
{
	return compareSpherePosition(sphere1, sphere2, glm::vec3(0, 0, 1));
}

inline bool compareTrianglePosition(const Triangle& triangle1, const Triangle& triangle2, glm::vec3 axis)
{
	glm::vec3 avgPosition1 = glm::vec3((positions->at(triangle1.v0) + positions->at(triangle1.v1) + positions->at(triangle1.v2)) / 3.0f);
	glm::vec3 avgPosition2 = glm::vec3((positions->at(triangle2.v0) + positions->at(triangle2.v1) + positions->at(triangle2.v2)) / 3.0f);
	return glm::dot(glm::vec3(avgPosition1), axis) < glm::dot(glm::vec3(avgPosition2), axis);
}

bool compareTriangleX(const Triangle& triangle1, const Triangle& triangle2)
{
	return compareTrianglePosition(triangle1, triangle2, glm::vec3(1, 0, 0));
}

bool compareTriangleY(const Triangle& triangle1, const Triangle& triangle2)
{
	return compareTrianglePosition(triangle1, triangle2, glm::vec3(0, 1, 0));
}

bool compareTriangleZ(const Triangle& triangle1, const Triangle& triangle2)
{
	return compareTrianglePosition(triangle1, triangle2, glm::vec3(0, 0, 1));
}



BVHNode::BVHNode(const std::vector<glm::vec4>& vertexPositions, std::vector<Triangle>& triangles, size_t start, size_t end)
{
	positions = &vertexPositions;

	// one element left
	if (end - start < 2)
	{
		indexOfPrimitive = start;
		left = nullptr;
		right = nullptr;
	}
	else
	{
		indexOfPrimitive = -1;
		box = AABB(vertexPositions, triangles, start, end);

		auto comparator = compareTriangleX;
		int axis = rand() % 3;
		if (axis == 0)
		{
			comparator = compareTriangleY;
			//std::cout << "y" << std::endl;
		}
		else if (axis == 1)
		{
			comparator = compareTriangleZ;
			//std::cout << "z" << std::endl;
		}
		else
		{
			//std::cout << "x" << std::endl;
		}

		std::sort(triangles.begin() + start, triangles.begin() + end, comparator);
		size_t mid = start + ((end - start) / 2);
 		left = new BVHNode(vertexPositions, triangles, start, mid);
		right = new BVHNode(vertexPositions, triangles, mid, end);
	}
}


BVHNode::~BVHNode()
{
	destroy();
}

void BVHNode::destroyHelper(BVHNode* node)
{
	if (!node)
	{
		return;
	}

	//destroyHelper(node->left);
	//destroyHelper(node->right);
	delete(node->left);
	delete(node->right);
}

void BVHNode::destroy()
{
	destroyHelper(this);
}

BVHNodeStruct BVHNode::getStruct() const
{
	return BVHNodeStruct{
		glm::vec4(box.getMinCorner(), 0),
		glm::vec4(box.getMaxCorner(), 0),
		indexOfPrimitive
	};
}

void BVHNode::toArrayHelper(std::vector<BVHNodeStruct>& arr, const BVHNode* node, int i) const
{
	if (node == nullptr)
	{
		return;
	}

	arr[i] = node->getStruct();
	toArrayHelper(arr, node->left, (i * 2) + 1);
	toArrayHelper(arr, node->right, (i * 2) + 2);
}

void BVHNode::toStructArray(std::vector<BVHNodeStruct>& arr) const
{
	toArrayHelper(arr, this, 0);
}

BVHNode& BVHNode::operator=(const BVHNode& other)
{
	box = other.box;
	indexOfPrimitive = other.indexOfPrimitive;
	
	if (!other.left)
	{
		left = nullptr;
	}
	else
	{
		left = new BVHNode;
		*left = *other.left;
	}

	if (!other.right)
	{
		right = nullptr;
	}
	else
	{
		right = new BVHNode;
		*right = *other.right;
	}

	return *this;
}