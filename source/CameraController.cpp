#include "CameraController.h"

CameraController::CameraController()
{
	camera1 = new Camera(glm::vec3(0, 0, 0), glm::vec3(0, 0, -1), 60, RenderParameters::RENDER_RESOLUTION.width, RenderParameters::RENDER_RESOLUTION.height);
}

CameraController::~CameraController()
{
	delete camera1;
}

void CameraController::update()
{
	// mouse input
	double mouseXDifference, mouseYDifference;
	InputSupport::getMouseDelta(mouseXDifference, mouseYDifference);

	camera1->rotate(camera1->getRight(), glm::radians(static_cast<float>(mouseYDifference)));
	camera1->rotate(WORLD_UP, glm::radians(static_cast<float>(mouseXDifference)));

	// key input

	if (InputSupport::isKeyPressed(GLFW_KEY_A))
	{
		camera1->offsetPosition(camera1->getRight(), -UNITS_MOVED_PER_SECOND * TimeSupport::getFrameTime());
	}
	if (InputSupport::isKeyPressed(GLFW_KEY_S))
	{
		camera1->offsetPosition(camera1->getForward(), -UNITS_MOVED_PER_SECOND * TimeSupport::getFrameTime());
	}
	if (InputSupport::isKeyPressed(GLFW_KEY_D))
	{
		camera1->offsetPosition(camera1->getRight(), UNITS_MOVED_PER_SECOND * TimeSupport::getFrameTime());
	}
	if (InputSupport::isKeyPressed(GLFW_KEY_W))
	{
		camera1->offsetPosition(camera1->getForward(), UNITS_MOVED_PER_SECOND * TimeSupport::getFrameTime());
	}
}

Camera& CameraController::getCamera1()
{
	return *camera1;
}