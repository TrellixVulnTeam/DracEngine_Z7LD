#include "RenderingEngine.h"
#include "Asset.h"
#include "RenderAPI.h"
#include "Bitmap.h"
#include "Texture.h"
#include "Core/Globals.h"

namespace Ry
{

	Ry::Texture* DefaultTexture = nullptr;

	// Shaders
	Ry::Map<String, Ry::Shader*> CompiledShaders;

	void InitRenderingEngine()
	{		
		// Compile standard shaders
		Ry::Log->Log("Compiling shaders");
		{
			CompileShader("Texture", "Vertex/Texture", "Fragment/Texture");
			CompileShader("Shape", "Vertex/Shape", "Fragment/Shape");
			CompileShader("Font", "Vertex/Font", "Fragment/Font");
		}

		// Create 1x1 default texture
		Ry::Bitmap DefaultTextureBmp (1, 1, PixelStorage::FOUR_BYTE_RGBA);
		DefaultTextureBmp.SetPixel(0, 0, 0xFFFFFFFF);
		DefaultTexture = Ry::RendAPI->CreateTexture(TextureFiltering::Nearest);
		DefaultTexture->Data(DefaultTextureBmp.GetData<uint8>(), 1, 1, PixelFormat::R8G8B8A8);
	}

	void QuitRenderingEngine()
	{
		DefaultTexture->DeleteTexture();
		delete DefaultTexture;
	}

	Shader* GetOrCompileShader(const String& Name, Ry::String Vertex, Ry::String Fragment)
	{
		Shader* Cur = GetShader(Name);

		if(!Cur)
		{
			Cur = CompileShader(Name, Vertex, Fragment);
		}

		return Cur;
	}

	Shader* CompileShader(const String& Name, Ry::String VertexLoc, Ry::String FragmentLoc)
	{
		Ry::Shader* Result = Ry::RendAPI->CreateShader(VertexLoc, FragmentLoc);
		CompiledShaders.insert(Name, Result);

		return Result;
	}

	Shader* GetShader(const String& Name)
	{
		if(CompiledShaders.contains(Name))
		{
			return *CompiledShaders.get(Name);
		}
		else
		{
			return nullptr;
		}
	}
	
}