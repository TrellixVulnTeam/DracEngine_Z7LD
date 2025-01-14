#include "VulkanShader.h"
#include "Manager/AssetManager.h"
#include "TextFileAsset.h"
#include "VulkanContext.h"
#include "Core/Globals.h"
#include "Language/ShaderCompiler.h"

namespace Ry
{

	bool VulkanShader::CreateSingleShader(const Ry::String& ShaderLoc, ShaderStage Stage, VkShaderModule& OutModule)
	{
		uint8* SpirV = nullptr;
		int32 Size = 0;
		Ry::String ErrWarnMsg;

		if(!Ry::CompileToSpirV(ShaderLoc, SpirV, Size, ErrWarnMsg, Stage))
		{
			Ry::Log->LogErrorf("Failed to compile HLSL shader: %s\n%s", *ShaderLoc, *ErrWarnMsg);
			return false;
		}

		VkShaderModuleCreateInfo ShaderCreateInfo {};
		ShaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		ShaderCreateInfo.codeSize = Size;
		ShaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(SpirV);

		if (vkCreateShaderModule(GVulkanContext->GetLogicalDevice(), &ShaderCreateInfo, nullptr, &OutModule) != VK_SUCCESS) 
		{
			Ry::Log->LogError("Failed to create Vulkan shader");
			return false;
		}
		else
		{
			Ry::Log->Logf("Compiled %s successfully", *ShaderLoc);
			return true;
		}
	}

	VulkanShader::VulkanShader(const Ry::String& VSAsset, const Ry::String& FSAsset):
	Shader(VSAsset, FSAsset)
	{
		CreateSingleShader(VSAsset, ShaderStage::Vertex, VSShaderModule);
		CreateSingleShader(FSAsset, ShaderStage::Fragment, FSShaderModule);

		VSShaderPipeline.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		VSShaderPipeline.pNext = nullptr;
		VSShaderPipeline.flags = 0;
		VSShaderPipeline.stage = VK_SHADER_STAGE_VERTEX_BIT;
		VSShaderPipeline.module = VSShaderModule;
		VSShaderPipeline.pName = "main";
		VSShaderPipeline.pSpecializationInfo = nullptr;

		FSShaderPipeline.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		FSShaderPipeline.pNext = nullptr;
		FSShaderPipeline.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		FSShaderPipeline.flags = 0;
		FSShaderPipeline.module = FSShaderModule;
		FSShaderPipeline.pName = "main";
		FSShaderPipeline.pSpecializationInfo = nullptr;
	}

	VkPipelineShaderStageCreateInfo VulkanShader::GetVSPipelineCreateInfo()
	{
		return VSShaderPipeline;
	}

	VkPipelineShaderStageCreateInfo VulkanShader::GetFSPipelineCreateInfo()
	{
		return FSShaderPipeline;
	}

	void VulkanShader::DestroyShader()
	{
		vkDestroyShaderModule(GVulkanContext->GetLogicalDevice(), VSShaderModule, nullptr);
		vkDestroyShaderModule(GVulkanContext->GetLogicalDevice(), FSShaderModule, nullptr);
	}

}
