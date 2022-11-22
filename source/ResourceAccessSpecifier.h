#pragma once
#include "AccessSpecifier.h"
#include "Resource.h"

struct ResourceAccessSpecifier
{
	Resource* resource;
	AccessSpecifier accessSpecifier;

	bool operator==(const ResourceAccessSpecifier& rightHandSide)
	{
		return resource == rightHandSide.resource
			&& accessSpecifier == rightHandSide.accessSpecifier;
	}
};
