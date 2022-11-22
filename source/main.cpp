#include "Tracer.h"
#include "RenderController.h"
#include "Pass.h"
#include "PresentationController.h"
#include "TimeSupport.h"

void mainLoop(Scene& scene, PresentationController& presentationController)
{
	while (!glfwWindowShouldClose(VulkanCoreSupport::getInstance().window))
	{
		glfwPollEvents();

		scene.update();

		scene.run();
		presentationController.present();

		TimeSupport::update();
	}
}

int main()
{
	try {
		// initialize render loop
		Tracer* tracer = new Tracer();
		PresentationController* presentationController = new PresentationController(tracer->getPresentedImage());
		presentationController->prepareExecution();

		// run render loop
		mainLoop(*tracer, *presentationController);

		// clean up render loop
		vkDeviceWaitIdle(VulkanCoreSupport::getInstance().getDevice());
		delete tracer;
		delete presentationController;
	}
	catch (const std::exception& e) {
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}
