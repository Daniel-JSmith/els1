#pragma once
#include <iostream>
#include "RenderParameters.h"


class InputSupport
{
public:

	// initializes curser and registers GLFW callbacks
	static void initialize(GLFWwindow* window);

	static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

	static void cursorPositionCallback(GLFWwindow* window, double xpos, double ypos);

	// A key is clicked on only the first frame the user presses the key
	static bool isKeyClicked(int keyCode);
	// Returns true if the key specified by keyCode is currently pressed
	static bool isKeyPressed(int keyCode);

	static void getMouseDelta(double& dX, double& dY);
};