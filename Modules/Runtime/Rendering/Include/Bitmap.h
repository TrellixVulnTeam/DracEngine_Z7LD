#pragma once

#include "Core/Core.h"
#include "Bitmap.gen.h"

namespace Ry
{
	class Text;
	class Vector4;
	class Vector3;

	/**
	 * Specifies a pixel format for textures.
	 */
	enum class PixelFormat2
	{
		NONE, RGB, RGBA, GRAYSCALE, DEPTH
	};

	enum class PixelStorage
	{
		NONE, FOUR_BYTE_RGBA, THREE_BYTE_RGB, RED8, FLOAT
	};
	
	class PixelBuffer
	{
	public:
		PixelBuffer(int32 InWidth, int32 InHeight, PixelFormat2 InFormat, PixelStorage Storage): Format(InFormat), Storage(Storage), Width(InWidth), Height(InHeight), Data(nullptr) {};
		virtual ~PixelBuffer() {};

		int32 GetWidth() const { return Width; }
		int32 GetHeight() const { return Height; }

		/**
		 *
		 * @return uint32 The pixel in R8G8B8A8 format.
		 */
		virtual uint32 GetPixel(int32 X, int32 Y) const = 0;

		/**
		 *
		 * @param Value The pixel in R8G8B8A8 format.
		 */
		virtual void SetPixel(int32 X, int32 Y, uint32 Value) = 0;

		PixelFormat2 GetPixelFormat()
		{
			return Format;
		}

		PixelStorage GetPixelStorage()
		{
			return Storage;
		}

		template <typename T>
		T* GetData() const
		{
			return static_cast<T*>(Data);
		}

		/**
		 *
		 * Creates a new array in RGBA pixel format. The callee must delete the array afterwards.
		 * 
		 */
		uint32* GetAsRGBA()
		{
			uint32* Result = new uint32[Width * Height * sizeof(uint32)];
			for(int32 Col = 0; Col < Width; Col++)
			{
				for(int32 Row = 0; Row < Height; Row++)
				{
					Result[Col + Row * Width] = GetPixel(Col, Row);
				}
			}

			return Result;
		}

		void SetData(void* Data)
		{
			this->Data = Data;
		}
		
	protected:
		PixelFormat2 Format;
		PixelStorage Storage;
		int32 Width;
		int32 Height;
		void* Data;
	};

	class PixelBufferFourByteRGBA : public PixelBuffer
	{
	public:
		PixelBufferFourByteRGBA(int32 Width, int32 Height);
		~PixelBufferFourByteRGBA() { delete[] (reinterpret_cast<uint8*>(Data)); };

		uint32 GetPixel(int32 X, int32 Y) const override;
		void SetPixel(int32 X, int32 Y, uint32 Value) override;
	};

	class PixelBufferThreeByteRGB : public PixelBuffer
	{
	public:
		PixelBufferThreeByteRGB(int32 Width, int32 Height);
		~PixelBufferThreeByteRGB(){ delete[](reinterpret_cast<uint8*>(Data)); };

		uint32 GetPixel(int32 X, int32 Y) const override;
		void SetPixel(int32 X, int32 Y, uint32 Value) override;
	};

	class PixelBufferRed8 : public PixelBuffer
	{
	public:
		PixelBufferRed8(int32 Width, int32 Height);
		~PixelBufferRed8() { delete[](reinterpret_cast<uint8*>(Data)); };

		uint32 GetPixel(int32 X, int32 Y) const override;
		void SetPixel(int32 X, int32 Y, uint32 Value) override;
	};
	
	class RENDERING_MODULE Bitmap
	{
	public:

		Bitmap(uint32 Width, uint32 Height, PixelStorage Storage);
		~Bitmap();

		/**
		 *
		 * @return uint32 The pixel in R8G8B8A8 format.
		 */
		uint32 GetPixel(int32 X, int32 Y) const;

		/**
		 *
		 * @param Value The pixel in R8G8B8A8 format.
		 */
		void SetPixel(int32 X, int32 Y, uint32 Value);
		
		template <typename T>
		T* GetData() const
		{
			return Buffer->GetData<T>();
		}

		void SetData(void* Data)
		{
			Buffer->SetData(Data);
		}

		void FlipY();

		void SetPixelBuffer(PixelBuffer* Buffer);
		PixelBuffer* GetPixelBuffer() const;

		int32 GetWidth() const;
		int32 GetHeight() const;

		void DrawTexture(Bitmap* Other, int32 X, int32 Y);
		void DrawBox(int32 X, int32 Y, int32 W, int32 H, const Ry::Vector4& Color);

	private:

		PixelBuffer* Buffer;

	};

	uint32 VectorToColor(const Ry::Vector4& Color);
}
