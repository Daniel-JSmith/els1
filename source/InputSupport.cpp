#include "InputSupport.h"

namespace
{
	double mouseXDelta, mouseYDelta;

	// true = down; false = up
	bool previousKeyState[GLFW_KEY_LAST + 1]{};
	bool currentKeyState[GLFW_KEY_LAST + 1]{};

	// true only on the first frame the user presses the key
	bool keyClicked[GLFW_KEY_LAST + 1]{};

	// record how many frames it has been since the cursor callback was envoked.
	// discard dx/dy if it has been too long since they were updated
	int framesSinceCursorUpdate = 0;
}

void InputSupport::initialize(GLFWwindow* window)
{
	glfwSetCursorPos(window, RenderParameters::RENDER_CENTER_X, RenderParameters::RENDER_CENTER_X);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);

	glfwSetKeyCallback(window, InputSupport::keyCallback);
	glfwSetCursorPosCallback(window, InputSupport::cursorPositionCallback);
}

void InputSupport::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
	if (action == GLFW_PRESS)
	{
		currentKeyState[key] = true;
		keyClicked[key] = true;
	}
	else if (action == GLFW_RELEASE)
	{
		currentKeyState[key] = false;
		keyClicked[key] = false;
	}
	else if (action == GLFW_REPEAT)
	{
		currentKeyState[key] = true;
		keyClicked[key] = false;
	}
}

void InputSupport::cursorPositionCallback(GLFWwindow* window, double xpos, double ypos)
{
	framesSinceCursorUpdate = 0;
	mouseXDelta = RenderParameters::RENDER_CENTER_X - xpos;
	mouseYDelta = RenderParameters::RENDER_CENTER_Y - ypos;

	glfwSetCursorPos(window, RenderParameters::RENDER_CENTER_X, RenderParameters::RENDER_CENTER_Y);
}

void updateKeyClicked(int keyCode)
{
	if (previousKeyState[keyCode])
	{
		keyClicked[keyCode] = false;
	}

	previousKeyState[keyCode] = currentKeyState[keyCode];
}

bool InputSupport::isKeyClicked(int keyCode)
{
	updateKeyClicked(keyCode);
	return keyClicked[keyCode];
}

bool InputSupport::isKeyPressed(int keyCode)
{
	return currentKeyState[keyCode];
}

void InputSupport::getMouseDelta(double& dX, double& dY)
{
	// Make sure cursor position is up to date.
	// The mouse callback may not always be called. In those cases, we discard
	// the mouse position and assume that mouse movement is 0
	if (framesSinceCursorUpdate < 1)
	{
		++framesSinceCursorUpdate;
	}

	else
	{
		mouseXDelta = mouseYDelta = 0;
	}

	dX = mouseXDelta;
	dY = mouseYDelta;
}