#pragma once

#include "Pass.h"

class PresentPass : public Pass
{
public:
	PresentPass(Image& sourceImage, Image& swapChainImage);
	~PresentPass();

	void prepareExecution(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors) override;
private:
	Image& sourceImage;
	Image& swapChainImage;

	void recordCommandBuffer(const std::vector<Pass*>& predecessors, const std::vector<Pass*>& successors) override;
};