#pragma once

#include "Interface/RenderingResource.h"
#include "glew.h"
#include "Data/Map.h"
#include "OpenGLGen.h"

namespace Ry
{

	int32 GLCalcAlignmentRequirement(const BufferMember* Root);
	int32 GLCalcAlignmentRequirement(ShaderPrimitiveDataType Primitive);

	int32 GLCalcBufferSize(const ConstantBuffer* Buffer);
	void GLCalcMemberSize(const BufferMember* Layout, int32& Offset);
	int32 GLSizeOf(const ShaderPrimitiveDataType& PrimData);

	class OPENGL_MODULE GLResourceDescription : public ResourceLayout
	{
	public:

		GLResourceDescription(const Ry::ArrayList<ShaderStage>& Stages, int32 SetIndex = 0):
		ResourceLayout(Stages, SetIndex)
		{
			
		}

		virtual ~GLResourceDescription() = default;
		
		void CreateDescription() override
		{
			
		}
		
		void DeleteDescription() override
		{
			
		}
		
	};

	struct UniformRef
	{
		bool bIsPrimitive;
		int32 Offset;
		int32 SlotSize = 0;
	};

	struct MappedUniformBuffer
	{
		ConstantBuffer* ConstBuffer = nullptr;
		Ry::Map<Ry::String, UniformRef> UniformRefs;
		uint8* HostDataBuffer;
		uint32 HostBufferSize;
		GLuint Ubo;
	};

	struct MappedTexture
	{
		int32 Slot;
		GLuint Texture;
		GLuint Target;
	};

	class OPENGL_MODULE GLResources : public ResourceSet
	{
	public:

		GLResources(const ResourceLayout* CreateInfo, SwapChain* SC);

		void CreateBuffer() override;
		void DeleteBuffer() override;
		bool Update() override;
		void FlushBuffer(int32 Frame) override;

		void BindTexture(Ry::String TextureName, const Ry::Texture* Resource) override;

		void SetConstant(Ry::String BufferName, Ry::String Id, const void* Data) override;

		MappedTexture* GetMappedTexture(const Ry::String& Name)
		{
			return *MappedTextures.get(Name);
		}

	private:

		void MapData(MappedUniformBuffer* Dst, const ConstantBuffer* Src);
		void MapData_Helper(MappedUniformBuffer* Dst, const BufferMember* Member, Ry::String ParentId, int32& Marker);


		Map<Ry::String, MappedUniformBuffer*> MappedUniformBuffers;

		int32 Textures = 0;
		Ry::Map<Ry::String, MappedTexture*> MappedTextures;

	};
	
}