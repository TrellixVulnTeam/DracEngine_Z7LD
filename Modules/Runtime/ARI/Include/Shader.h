#pragma once

#include "VertexFormat.h"
#include "Shader.gen.h"

namespace Ry
{

	class ResourceLayout;

	enum class ShaderStage
	{
		Vertex,
		Fragment
	};


	/**
	 * Stores shader data types and how many conceptual elements they store (not m=
	 *
	 * All matrices in this enum are in ROW MAJOR order.
	 * todo: this would be a great use case for runtime reflection. pass in a reflected struct so we can dynamically make the uniform buffer
	 */
	enum class ShaderPrimitiveDataType
	{
		None,
		
		/* 1 element types */
		Float,

		/* 2 element types */
		Float2,
		Float1x2,
		Float2x1,

		/* 3 element types */
		Float3,
		Float3x1,
		Float1x3,

		/* 4 element types */
		Float4,
		Float2x2,
		Float4x1,
		Float1x4,

		/* 6 element types */
		Float2x3,
		Float3x2,

		/* 8 element types */
		Float4x2,
		Float2x4,

		/* 9 element types */
		Float3x3,

		/* 12 element types */
		Float4x3,
		Float3x4,

		/* 16 element types*/
		Float4x4
	};

	Ry::String ARI_MODULE ToString(ShaderPrimitiveDataType DT);

	struct ShaderVariable
	{
		Ry::String Name;
		ShaderPrimitiveDataType Type = ShaderPrimitiveDataType::None;
		int32 ArraySize = 1;

		/**
		 * Includes float type.
		 */
		bool IsVectorType() const
		{
			if (Type == ShaderPrimitiveDataType::Float)
				return true;
			if (Type == ShaderPrimitiveDataType::Float2)
				return true;
			if (Type == ShaderPrimitiveDataType::Float3)
				return true;
			if (Type == ShaderPrimitiveDataType::Float4)
				return true;

			return false;
		}

		int32 GetVectorElementCount() const
		{
			if (Type == ShaderPrimitiveDataType::Float)
				return 1;
			if (Type == ShaderPrimitiveDataType::Float2)
				return 2;
			if (Type == ShaderPrimitiveDataType::Float3)
				return 3;
			if (Type == ShaderPrimitiveDataType::Float4)
				return 4;

			return -1;
		}


	};

	class ARI_MODULE ShaderReflection
	{
	public:

		void AddResourceDescription(ResourceLayout* Desc);
		void AddInputVariable(ShaderVariable Var);

		const Ry::ArrayList<ResourceLayout*>& GetResources() const;
		const Ry::ArrayList<ShaderVariable>& GetInputVars() const;

		const ResourceLayout* operator[] (int32 Index) const;

		
	private:

		Ry::ArrayList<ResourceLayout*> Resources;
		Ry::ArrayList<ShaderVariable> InputVars;
		
	};

	class ARI_MODULE Shader
	{
	public:

		Shader(const Ry::String& VSAsset, const Ry::String& FSAsset);
		
		virtual ~Shader() = default;

		/************************************************************************/
		/* Interface functions                                                  */
		/************************************************************************/
		
		/**
		 * Deletes this shader from the rendering interface. Do not use shader after calling this.
		 */
		virtual void DestroyShader() = 0;

		const VertexFormat& GetVertexFormat() const;
		const ShaderReflection& GetVertexReflectionData() const;
		const ShaderReflection& GetFragmentReflectionData() const;

	private:

		Ry::VertexFormat VertFormat;
		ShaderReflection VertReflectionData;
		ShaderReflection FragReflectionData;

	};


}
