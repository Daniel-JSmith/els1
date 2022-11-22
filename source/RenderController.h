#pragma once

#include "InputSupport.h"

// updates RenderParameters according to user input
class RenderController
{
public:
	RenderController();
	~RenderController();

	void update();

private:
	const int SAMPLES_INCREASE_STEP = 1;
};

