#include "Camera.h"

Camera::Camera(glm::vec3 position, glm::vec3 forward, float verticalFov, int pixelWidth, int pixelHeight) : position(position), forward(forward), verticalFov(verticalFov), focusDistance(3.0f), pixelWidth(pixelWidth), pixelHeight(pixelHeight)
{
	float aspectRatio = static_cast<float>(pixelWidth) / pixelHeight;
	focalPlaneHeight = 2 * tan(DEG_TO_RAD * (verticalFov / 2)) * focusDistance;
	focalPlaneWidth = aspectRatio * focalPlaneHeight;
}

CameraStruct Camera::getCameraStruct() const
{
	glm::vec3 w = -forward;
	glm::vec3 u = getRight();
	glm::vec3 v = cross(w, u);
	glm::vec3 lowerLeftCorner = position - (u * focalPlaneWidth / 2.0f) - (v * focalPlaneHeight / 2.0f) - w * focusDistance;

	return CameraStruct {
		glm::vec4(position, 0),			// position
		glm::vec4(v, 0),				// up
		glm::vec4(forward, 0),			// forward
		glm::vec4(u, 0),				// right
		pixelWidth,
		pixelHeight,
		focalPlaneWidth,				// focalPlaneWidth
		focalPlaneHeight,				// focalPlaneHeight
		glm::vec4(lowerLeftCorner, 0)	// lowerLeftCorner
	};
}

void Camera::offsetPosition(const glm::vec3& axis, float delta)
{
	position += (axis * delta);
}

void Camera::rotate(const glm::vec3& axis, float radians)
{
	glm::mat4 rotation = glm::mat4(1.0f);
	rotation = glm::rotate(rotation, radians * SENSITIVITY, axis);
	forward = rotation * glm::vec4(forward, 1);
}

void Camera::lookAt(const glm::vec3& lookAt)
{
	forward = glm::normalize(lookAt - position);
}

const glm::vec3& Camera::getForward() const
{
	return forward;
}
glm::vec3 Camera::getRight() const
{
	return glm::cross(forward, glm::vec3(0, 1, 0));
}

const glm::vec3& Camera::getPosition() const
{
	return position;
}
