#pragma once
#include "Core/Platform.h"
#include "Core/Globals.h"
#include "Manager/AssetManager.h"
#include "Factory/ObjMeshFactory.h"
#include "Factory/TrueTypeFontFactory.h"
#include "Factory/TextureFactory.h"
#include "Factory/AudioFactory.h"
#include "GLRenderAPI.h"
#include "Window.h"
#include "Factory/TextFileFactory.h"
#include "Widget/Layout/GridPanel.h"
#include "EditorWindow.h"
#include "File/File.h"
#include "VulkanRenderAPI.h"
#include "Core/PlatformProcess.h"
#include "MainWindow/MainEditorWindow.h"

namespace Ry
{

	class Editor
	{
	public:

		Editor(Ry::RenderingPlatform Platform)
		{
			Ry::rplatform = new Ry::RenderingPlatform(Platform);
		}

		void InitFileSystem()
		{
			Ry::String EngineRoot = Ry::File::GetParentPath(Ry::File::GetParentPath(GetPlatformModulePath()));
			Ry::String ResourcesRoot = Ry::File::Join(EngineRoot, "Resources");
			Ry::String ShadersRoot = Ry::File::Join(EngineRoot, "Shaders");

			Ry::File::MountDirectory(ResourcesRoot, "Engine");

			// Todo: Support custom game shaders as well
			Ry::File::MountDirectory(ShadersRoot, "Shaders");
		}

		void InitAssetSystem()
		{
			AssetMan = new Ry::AssetManager;

			// Register asset factories
			// TODO: these should be moved
			TextureFactory* TextureImporter = new TextureFactory;

			AssetMan->RegisterFactory("text", new TextFileFactory);
			AssetMan->RegisterFactory("mesh/obj", new ObjMeshFactory);
			AssetMan->RegisterFactory("font/truetype", new TrueTypeFontFactory);
			AssetMan->RegisterFactory("sound", new AudioFactory);
			AssetMan->RegisterFactory("image", TextureImporter);

			AssetMan->RegisterFactory("png", TextureImporter);
		}

		void InitRenderAPI()
		{

			if (*Ry::rplatform == Ry::RenderingPlatform::OpenGL)
			{
				if (!Ry::InitOGLRendering())
				{
					Ry::Log->LogError("Failed to initialize OpenGL");
				}
				else
				{
					Ry::Log->Log("Initialized OpenGL");
				}
			}

			if (*Ry::rplatform == Ry::RenderingPlatform::Vulkan)
			{
				if (!Ry::InitVulkanAPI())
				{
					Ry::Log->LogError("Failed to initialize Vulkan API");
					std::abort();
				}
			}

		}

		void ShutdownRenderAPI()
		{
			if (*Ry::rplatform == Ry::RenderingPlatform::Vulkan)
			{
				ShutdownVulkanAPI();
			}
		}

		void InitLogger()
		{
			Ry::Log = new Ry::Logger;
		}

		void Init()
		{
			InitLogger();
			InitFileSystem();
			InitWindowing();
			InitRenderAPI();
			InitAssetSystem();

			PrimaryWindow = new MainEditorWindow;
			PrimaryWindow->Init();

			//CreateSecondaryWindow<Ry::EditorWindow>();

			//SecondaryWindow = new EditorWindow;
			//SecondaryWindow->Init();

			// Startup initial editor window

			//Wnd->GetSwapChain()->OnSwapChainDirty.AddMemberFunction(this, &ContentBrowser::SwapChainDirty);
		}

		void Update()
		{


		}

		void Run()
		{
			Init();

			// LastFps = std::chrono::high_resolution_clock::now();
			// while (!EditorMainWindow->ShouldClose())
			// {
			// 	FPS++;
			// 	UpdateEditor();
			// 	RenderEditor();
			//
			// 	std::chrono::duration<double> Delta = std::chrono::high_resolution_clock::now() - LastFps;
			// 	if (std::chrono::duration_cast<std::chrono::seconds>(Delta).count() >= 1.0)
			// 	{
			// 		LastFps = std::chrono::high_resolution_clock::now();
			// 		Ry::Log->Logf("FPS: %d", FPS);
			// 		FPS = 0;
			// 	}
			// }


			while (!PrimaryWindow->WantsClose())
			{
				// Get current frame time point
				auto CurFrame = std::chrono::high_resolution_clock::now();

				// Calculate delta
				std::chrono::duration<double> Delta = CurFrame - LastFrame;
				float DeltaSeconds = static_cast<float>(std::chrono::duration_cast<std::chrono::nanoseconds>(Delta).count() / 1e9);

				// Reset last frame time point
				LastFrame = CurFrame;

				// Update and render primary editor window
				PrimaryWindow->Update(DeltaSeconds);
				PrimaryWindow->Render();

				// Update and render secondary windows
				int32 SecondaryWindowIndex = 0;
				while(SecondaryWindowIndex < SecondaryWindows.GetSize())
				{
					EditorWindow* Wnd = SecondaryWindows[SecondaryWindowIndex];
					if(Wnd->WantsClose())
					{
						DestroySecondaryWindow(Wnd);
					}
					else
					{
						Wnd->Update(DeltaSeconds);
						Wnd->Render();

						SecondaryWindowIndex++;
					}
				}

				// todo: secondary editor windows (dock)?

				std::chrono::duration<double> DeltaLastFPS = CurFrame - LastFPS;
				if(std::chrono::duration_cast<std::chrono::seconds>(DeltaLastFPS).count() >= 1)
				{
					std::cout << FPS << std::endl;
					FPS = 0;
					LastFPS = CurFrame;
				}

				FPS++;
			}

			ShutdownWindowing();
		}

		template<typename T>
		T* CreateSecondaryWindow()
		{
			T* NewWindow = new T;
			NewWindow->Init();

			SecondaryWindows.Add(NewWindow);

			return NewWindow;
		}

		void DestroySecondaryWindow(EditorWindow* Window)
		{
			Window->GetWindow()->Destroy();
			SecondaryWindows.Remove(Window);

			delete Window;
		}

	private:

		Ry::MainEditorWindow* PrimaryWindow;
		Ry::ArrayList<EditorWindow*> SecondaryWindows;

		int32 FPS = 0;

		std::chrono::high_resolution_clock::time_point LastFrame = std::chrono::high_resolution_clock::now();
		std::chrono::high_resolution_clock::time_point LastFPS = std::chrono::high_resolution_clock::now();

	};
	
}
