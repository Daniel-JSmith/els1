#pragma once

#include "Image.h"

// An interface between client code and the core rendering engine
class Scene
{
public:
	// Called by the core rendering engine before the first frame
	virtual void initialize() = 0;

	// Called by the core rendering engine before first frame. Returns the image to be presented
	virtual Image& getPresentedImage() const = 0;

	// Called by the core rendering engine every frame. Updates scene state
	virtual void update() = 0;

	// Called by the core rendering engine every frame. Stores output in the Image returned by getPresentedImage. Does not present
	virtual void run() = 0;

private:

};