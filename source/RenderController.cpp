#include "RenderController.h"



RenderController::RenderController()
{
}


RenderController::~RenderController()
{
}

void RenderController::update()
{
	if (InputSupport::isKeyClicked(GLFW_KEY_C))
	{
		RenderParameters::getInstance().changeNumSamplesPerPixel(-SAMPLES_INCREASE_STEP);
	}
	if (InputSupport::isKeyClicked(GLFW_KEY_V))
	{
		RenderParameters::getInstance().changeNumSamplesPerPixel(SAMPLES_INCREASE_STEP);
	}
}