cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 17)

project (els)

add_executable(els 

"source/AABB.cpp" 
"source/AABB.h" 
"source/AccessSpecifier.h" 
"source/AssetManager.h" 
"source/Buffer.cpp" 
"source/Buffer.h" 
"source/BVHNode.cpp" 
"source/BVHNode.h" 
"source/Camera.cpp" 
"source/Camera.h" 
"source/CameraController.cpp" 
"source/CameraController.h" 
"source/DrawPass.cpp" 
"source/DrawPass.h" 
"source/GPUStructs.h" 
"source/Image.cpp" 
"source/Image.h" 
"source/InputSupport.cpp" 
"source/InputSupport.h" 
"source/main.cpp" 
"source/Pass.cpp" 
"source/Pass.h" 
"source/PresentationController.cpp" 
"source/PresentationController.h" 
"source/PresentPass.cpp" 
"source/PresentPass.h" 
"source/RenderController.cpp" 
"source/RenderController.h" 
"source/RenderParameters.cpp" 
"source/RenderParameters.h" 
"source/Resource.cpp" 
"source/Resource.h" 
"source/ResourceAccessSpecifier.h" 
"source/Scene.h" 
"source/Shader.cpp" 
"source/Shader.h" 
"source/TimeSupport.cpp" 
"source/TimeSupport.h" 
"source/Tracer.cpp" 
"source/Tracer.h" 
"source/TransferPass.h" 
"source/VulkanCoreSupport.cpp" 
"source/VulkanCoreSupport.h" 

 )

find_package(Vulkan REQUIRED)

file(GLOB SHADER_SOURCES
	"${PROJECT_SOURCE_DIR}/assets/shaders/*.vert"
	"${PROJECT_SOURCE_DIR}/assets/shaders/*.frag"
	"${PROJECT_SOURCE_DIR}/assets/shaders/*.comp"
)

foreach(SHADER_SOURCE ${SHADER_SOURCES})
	get_filename_component(FILE_NAME ${SHADER_SOURCE} NAME)
	set(SHADER_SPIRV "${PROJECT_SOURCE_DIR}/assets/shaders/${FILE_NAME}.spv")
	add_custom_command(
		OUTPUT ${SHADER_SPIRV}
		COMMAND ${Vulkan_GLSLC_EXECUTABLE} ${SHADER_SOURCE} -o ${SHADER_SPIRV}
		DEPENDS ${SHADER_SOURCE}
		)
		list(APPEND SHADER_SPIRVS ${SHADER_SPIRV})
endforeach(SHADER_SOURCE)

target_include_directories(els PUBLIC ${Vulkan_INCLUDE_DIR})
target_include_directories(els PUBLIC ${PROJECT_SOURCE_DIR}/include)

target_link_directories(els PUBLIC ${PROJECT_SOURCE_DIR}/lib)

target_link_libraries(els ${Vulkan_LIBRARY})
target_link_libraries(els glfw3)

add_custom_target(shaders DEPENDS ${SHADER_SPIRVS})

add_dependencies(els shaders)
