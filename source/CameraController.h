#pragma once

#include "Camera.h"
#include "InputSupport.h"
#include "RenderParameters.h"
#include "TimeSupport.h"

class CameraController
{
public:

	const float UNITS_MOVED_PER_SECOND = 1.0f;

	CameraController();

	~CameraController();

	void update();

	Camera& getCamera1();

private:

	Camera* camera1;
};