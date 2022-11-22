#pragma once
#include "Pass.h"

class TransferPass : public Pass
{
public:
	TransferPass(Image*& outputAttachment, std::vector<ResourceAccessSpecifier> descriptors, Image*& sourceResource);
	~TransferPass();
private:
	Image*& sourceResource;

	void recordCommandBuffer(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors) override;
};