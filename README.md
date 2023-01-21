# els1

GPU path tracer written in C++ with Vulkan.

# features

* triangle primitives
* sphere primitives
* bounding volume hierarchy
* area lights
* pbr material system
* transparent materials
* .obj scene loading

# motivation

I was studying graphics from [Peter Shirley's ray tracing books](https://raytracing.github.io/). This project is an extension of the code I wrote following along. Extensions include:
* moving the path tracer to the GPU using Vulkan
* ray-triangle intersections
* model loading

# limitations

* This project doesn't use denoising, reservoir sampling, or next event estimation. Using those technologies would improve performance. 
