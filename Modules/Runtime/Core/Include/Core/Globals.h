#pragma once

//#include "Core/Application/Application.h"
#include "Core/Delegate.h"

//#include "Input/Input.h"
//#include "Rendering/Interface/Rendering.h"
//#include "Asset/Manager/IAssetManager.h"
#include "Globals.gen.h"

namespace Ry
{
	class AbstractGame;
	class Application;
	class Renderer;
	class Logger;
	class InputHandler;
	class RenderAPI;
	enum class RenderingPlatform;

	
	extern Ry::String ResourcesDirectory;

	/**
	 * Application globals
	 */
	extern CORE_MODULE Ry::Logger* Log;
	extern CORE_MODULE MulticastDelegate<int32, int32> OnWindowResize;
	extern CORE_MODULE Application* app;
	extern CORE_MODULE Ry::Delegate<int32> ViewportWidthDel;
	extern CORE_MODULE Ry::Delegate<int32> ViewportHeightDel;
	CORE_MODULE int32 GetViewportWidth();
	CORE_MODULE int32 GetViewportHeight();

	/**
	 * Input globals
	 */
	extern CORE_MODULE InputHandler* input_handler;

	/**
	 * Rendering globals
	 */	
	extern CORE_MODULE RenderAPI* RendAPI;

	extern CORE_MODULE RenderingPlatform* rplatform;
	
	/**
	 * The asset manager
	 */

}
