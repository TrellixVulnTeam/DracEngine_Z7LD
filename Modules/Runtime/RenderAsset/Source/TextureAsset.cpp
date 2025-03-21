#include "TextureAsset.h"
#include "Core/Globals.h"
#include "Bitmap.h"
#include "RenderAPI.h"
#include "Texture.h"

namespace Ry
{

	TextureAsset::TextureAsset(Bitmap* Resource)
	{
		this->Resource = Resource;
	}

	Texture* TextureAsset::CreateRuntimeTexture()
	{
		Texture* NewTexture = Ry::RendAPI->CreateTexture(TextureFiltering::Nearest);
		NewTexture->Data(Resource->GetData<uint8>(), Resource->GetWidth(), Resource->GetHeight(), PixelFormat::R8G8B8A8);

		RuntimeResources.Add(NewTexture);

		return NewTexture;
	}

	Ry::Bitmap* TextureAsset::GetResource()
	{
		return Resource;
	}

	void TextureAsset::UnloadAsset()
	{
		for(Texture* RuntimeResource : RuntimeResources)
		{
			RuntimeResource->DeleteTexture();
			delete RuntimeResource;
		}
		
		delete Resource;
	}
	
}
