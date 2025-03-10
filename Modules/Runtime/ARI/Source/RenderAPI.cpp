#include "RenderAPI.h"
#include "Pipeline.h"
#include "Shader.h"
#include "Core/Globals.h"

namespace Ry
{
	Pipeline* RenderAPI::CreatePipelineFromShader(const PipelineCreateInfo& CreateInfo, Shader* Src)
	{
		PipelineCreateInfo New = CreateInfo;

		const ShaderReflection& VertRef = Src->GetVertexReflectionData();
		const ShaderReflection& FragRef = Src->GetFragmentReflectionData();

		for(ResourceLayout* Desc : VertRef.GetResources())
		{
			New.ResourceDescriptions.Add(Desc);
		}

		for (ResourceLayout* Desc : FragRef.GetResources())
		{
			New.ResourceDescriptions.Add(Desc);
		}

		// Determine vertex format
		const Ry::ArrayList<ShaderVariable>& InputVars = VertRef.GetInputVars();

		Ry::ArrayList<VertexAttrib> Attribs;

		if(InputVars.GetSize() > 0)
		{
			New.VertFormat = Src->GetVertexFormat();
		}
		else
		{
			Ry::Log->LogError("Input variables for vertex shader had size zero, can't create pipeline");
			return nullptr;
		}
		
		return CreatePipeline(New);
	}
}
