#pragma once

struct AccessSpecifier
{

	// TODO an operation implies an access type. e.g. if you specify SHADER_READ, it can be assumed that it is a read access
	enum ACCESS_TYPE
	{
		NO_ACCESS,
		READ,
		WRITE
	};

	enum OPERATION
	{
		NO_OPERATION,
		COLOR_ATTACHMENT_OUTPUT,
		SHADER_READ,
		TRANSFER_SOURCE,
		TRANSFER_DESTINATION,
		PREPARE_FOR_PRESENTATION,
		PRESENT
	};

	ACCESS_TYPE accessType;
	OPERATION operation;

		bool operator==(const AccessSpecifier& rightHandSide)
	{
		return rightHandSide.accessType == accessType
			&& rightHandSide.operation == operation;
	}
};
