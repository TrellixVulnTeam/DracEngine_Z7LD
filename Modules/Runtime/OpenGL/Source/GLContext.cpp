#include "GLContext.h"
#include "glfw3.h"

namespace Ry
{

	GLContext::GLContext()
	{
		
	}
	
	bool GLContext::PreWindowCreation(::GLFWwindow* Window)
	{
		// TODO: do we want these to be configurable?

		return true;
	}

	bool GLContext::InitContextGL(::GLFWwindow* Window, SwapChain* SwapChains)
	{
		glfwMakeContextCurrent(Window);

		return true;
	}

	void GLContext::CleanupContext()
	{
		// Do nothing here - GLFW will cleanup context when destroy window is called
	}
	
}
