#include "Interface/Texture.h"

namespace Ry
{

	// void Texture::AllocateColor(uint32 Width, uint32 Height)
	// {
	// 	this->Data(nullptr, PixelFormat::RGBA, PixelStorage::NONE, Width, Height);
	// }
	//
	// void Texture::AllocateDepth(uint32 Width, uint32 Height)
	// {
	// 	this->Data(nullptr, PixelFormat::DEPTH, PixelStorage::NONE, Width, Height);
	// }
	//
	// void Texture::Data(const Bitmap* Bitmap)
	// {
	// 	this->Data(Bitmap->GetData<void*>(), Bitmap->GetPixelBuffer()->GetPixelFormat(), Bitmap->GetPixelBuffer()->GetPixelStorage(), Bitmap->GetWidth(), Bitmap->GetHeight());
	// }
	//
	// void Texture::Bind()
	// {
	// 	this->Bind(0);
	// }

	Texture::Texture(TextureUsage InUsage)
	{
		this->Usage = InUsage;
	}
	
	TextureUsage Texture::GetUsage() const
	{
		return Usage;
	}
}