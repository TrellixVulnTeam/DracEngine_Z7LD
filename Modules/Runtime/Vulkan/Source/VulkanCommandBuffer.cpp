#include "VulkanCommandBuffer.h"
#include "VulkanContext.h"
#include "VulkanRenderPass.h"
#include "Core/Globals.h"
#include "VulkanFramebuffer.h"
#include "VulkanVertexArray.h"
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanResources.h"

namespace Ry
{

	VulkanCommandBuffer::VulkanCommandBuffer(SwapChain* SC, SecondaryCommandBufferInfo Info) :
	CommandBuffer(SC, Info)
	{
		VulkanSwapChain* VkSC = dynamic_cast<VulkanSwapChain*>(Swap);

		if (!VkSC)
		{
			Ry::Log->LogError("Non vulkan swap chain used with vulkan cmd buffer");
			return;
		}

		CreateBuffers();

		this->SwapChainVersion = VkSC->GetSwapchainVersion();		
	}

	VulkanCommandBuffer::VulkanCommandBuffer(SecondaryCommandBufferInfo Info):
	CommandBuffer(Info)
	{
		// Just create a single command buffer
		VkCommandBuffer NewCmdBuffer;
		if (!CreateCmdBufferResource(NewCmdBuffer))
		{
			Ry::Log->LogError("Could not complete recording of Vulkan render command: failed to create command buffer");
			return;
		}

		GeneratedBuffers.Add(NewCmdBuffer);

	}


	VulkanCommandBuffer::~VulkanCommandBuffer()
	{
		
	}

	void VulkanCommandBuffer::CreateBuffers()
	{
		VulkanSwapChain* VkSC = dynamic_cast<VulkanSwapChain*>(Swap);

		// Create as many command buffers as there are frame buffer images
		for (uint32 Fb = 0; Fb < VkSC->SwapChainImageViews.GetSize(); Fb++)
		{
			VkCommandBuffer NewCmdBuffer;
			if (!CreateCmdBufferResource(NewCmdBuffer))
			{
				Ry::Log->LogError("Could not complete recording of Vulkan render command: failed to create command buffer");
				return;
			}

			GeneratedBuffers.Add(NewCmdBuffer);
		}
	}

	void VulkanCommandBuffer::FreeBuffers()
	{
		for (VkCommandBuffer Buff : GeneratedBuffers)
		{
			FreeCmdBufferResource(Buff);
		}

		GeneratedBuffers.Clear();
	}

	void VulkanCommandBuffer::RecordBeginRenderPass(VkCommandBuffer CmdBuffer, VkFramebuffer Resource, VkExtent2D BufferExtent, Ry::RenderPass* RenderPass, bool bUseSecondary)
	{
		VulkanRenderPass* VkRenderPass = dynamic_cast<VulkanRenderPass*>(RenderPass);

		if (!VkRenderPass)
		{
			Ry::Log->LogError("A non-Vulkan render pass was passed into BeginRenderPass()");
			return;
		}

		VkRenderPassBeginInfo RenderPassInfo{};
		RenderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		RenderPassInfo.renderPass = VkRenderPass->GetRenderPass();
		RenderPassInfo.framebuffer = Resource;
		RenderPassInfo.renderArea.offset = { 0, 0 };
		RenderPassInfo.renderArea.extent = BufferExtent;

		Ry::ArrayList<VkClearValue> ClearValues;
		FrameBufferDescription Description = RenderPass->GetFboDescription();
		for(const AttachmentDescription& Desc : Description.Attachments)
		{
			VkClearValue Clear;
			if (Desc.Format == AttachmentFormat::Color)
			{
				Clear.color = { 0.0f, 0.0f, 0.0f, 1.0f };
			}
			else
			{
				Clear.depthStencil = { 1.0f, 0 };
			}

			ClearValues.Add(Clear);
		}

		RenderPassInfo.clearValueCount = ClearValues.GetSize();
		RenderPassInfo.pClearValues = ClearValues.GetData();

		if(bUseSecondary)
		{
			vkCmdBeginRenderPass(CmdBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);
		}
		else
		{
			vkCmdBeginRenderPass(CmdBuffer, &RenderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
		}
	}

	void VulkanCommandBuffer::RecordEndRenderPass(VkCommandBuffer CmdBuffer)
	{
		vkCmdEndRenderPass(CmdBuffer);
	}

	void VulkanCommandBuffer::RecordBindPipeline(VkCommandBuffer CmdBuffer, Pipeline* Pipeline)
	{
		Ry::VulkanPipeline* VPipeline = dynamic_cast<Ry::VulkanPipeline*>(Pipeline);

		if (!VPipeline)
		{
			Ry::Log->LogError("Passed in a non vulkan pipeline to BindPipeline()");
			return;
		}

		vkCmdBindPipeline(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, VPipeline->GraphicsPipeline);

		BoundPipeline = VPipeline;
	}

	void VulkanCommandBuffer::RecordSetScissorSize(VkCommandBuffer CmdBuffer, int32 ScissorX, int32 ScissorY, uint32 ScissorWidth, uint32 ScissorHeight, int32 TargetWidth, int32 TargetHeight)
	{
		int32 ConvertedY = std::max((int32) (TargetHeight - (ScissorY + ScissorHeight)), 0);

		VkRect2D Scissor;
		Scissor.offset = { std::max(ScissorX, 0), ConvertedY};
		Scissor.extent = VkExtent2D{std::max(ScissorWidth, (uint32)0), std::max(ScissorHeight, (uint32)0)};

		vkCmdSetScissor(CmdBuffer, 0, 1, &Scissor);
	}

	void VulkanCommandBuffer::RecordSetViewportSize(VkCommandBuffer CmdBuffer, int32 ViewportX, int32 ViewportY, int32 ViewportWidth, int32 ViewportHeight, int32 TargetWidth, int32 TargetHeight)
	{
		VkViewport ViewportParams;
		ViewportParams.x = (float) ViewportX;
		ViewportParams.y = (float) (TargetHeight + ViewportY);// ViewportY;
		ViewportParams.minDepth = 0.0f;
		ViewportParams.maxDepth = 1.0f;
		ViewportParams.width = (float) ViewportWidth;
		ViewportParams.height = (float) -ViewportHeight;

		vkCmdSetViewport(CmdBuffer, 0, 1, &ViewportParams);
	}

	void VulkanCommandBuffer::RecordBindResourceSet(VkCommandBuffer CmdBuffer, int32 CmdBufferIndex, BindResourcesCommand* Cmd)
	{
		Ry::ArrayList<VkDescriptorSet> DescriptorSets;
		for(int32 Set = 0; Set < Cmd->SetCount; Set++)
		{
			ResourceSet* ResSet = Cmd->SetPtrs[Set];
			
			Ry::VulkanResourceSet* VResSet = dynamic_cast<Ry::VulkanResourceSet*>(ResSet);

			if (!VResSet)
			{
				Ry::Log->LogError("Passed in a non vulkan uniform to BindUniformBuffer()");
				return;
			}

			DescriptorSets.Add(VResSet->DescriptorSets[CmdBufferIndex]);
		}

		if(BoundPipeline)
		{
			vkCmdBindDescriptorSets(CmdBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, BoundPipeline->PipelineLayout, 0, DescriptorSets.GetSize(), DescriptorSets.GetData(), 0, nullptr);
		}
		else
		{
			Ry::Log->LogError("Must bind pipeline before binding uniform descriptor sets");
		}
	}

	void VulkanCommandBuffer::RecordDrawVertexArray(VkCommandBuffer CmdBuffer, VertexArray* VertArray, uint32 FirstVertex, uint32 Count)
	{
		VulkanVertexArray* VkArray = dynamic_cast<VulkanVertexArray*>(VertArray);

		// Bind the vertex buffer
		VkBuffer VertexBuffers[] = { VkArray->StagingVertexBuffer->GetBufferObject() };
		VkDeviceSize Offsets[] = { 0 };
		int32 VertexCount = Count > 0 ? Count : VkArray->GetVertexCount(); // Draw all elements if count is zero

		vkCmdBindVertexBuffers(CmdBuffer, 0, 1, VertexBuffers, Offsets);
		vkCmdDraw(CmdBuffer, VertexCount, 1, FirstVertex, 0);
	}

	void VulkanCommandBuffer::RecordDrawVertexArrayIndexed(VkCommandBuffer CmdBuffer, VertexArray* VertArray,
		uint32 FirstIndex, uint32 Count)
	{

		VulkanVertexArray* VkArray = dynamic_cast<VulkanVertexArray*>(VertArray);

		if (!VkArray->StagingIndexBuffer)
		{
			Ry::Log->LogError("Vulkan vertex buffer had no index buffer in DrawVertexArrayIndexed()");
			return;
		}

		if(VkArray->GetVertexCount() > 0 && VkArray->GetIndexCount() > 0)
		{
			VkBuffer VertexBuffers[] = { VkArray->StagingVertexBuffer->GetBufferObject() };
			VkDeviceSize Offsets[] = { 0 };

			vkCmdBindVertexBuffers(CmdBuffer, 0, 1, VertexBuffers, Offsets);
			{
				vkCmdBindIndexBuffer(CmdBuffer, VkArray->StagingIndexBuffer->GetBufferObject(), 0, VK_INDEX_TYPE_UINT32);
				{
					int32 IndexCount = Count > 0 ? Count : VkArray->GetIndexCount(); // Draw all elements if count is zero

					vkCmdDrawIndexed(CmdBuffer, IndexCount, 1, FirstIndex, 0, 0);
				}
			}
		}

	}

	void VulkanCommandBuffer::RecordCommandBuffer(VkCommandBuffer CmdBuffer, int32 Index, VulkanCommandBuffer* Secondary)
	{

		vkCmdExecuteCommands(CmdBuffer, 1, &Secondary->GeneratedBuffers[Index]);

	}

	void VulkanCommandBuffer::ParseOp(VkCommandBuffer CurrentCmdBuffer, int32 CmdBufferIndex, uint8* Data, uint32& Marker)
	{
		uint8 NextOpCode = Data[Marker];

		Marker++;

		if (NextOpCode == OP_BEGIN_RENDER_PASS)
		{
			BeginRenderPassCommand* BeginCmd = ExtractToken<BeginRenderPassCommand>(Marker, Data);
			if(VulkanFrameBuffer* SourceBuffer = dynamic_cast<VulkanFrameBuffer*>(BeginCmd->SourceBuffer))
			{
				RecordBeginRenderPass(CurrentCmdBuffer, SourceBuffer->GetFrameBufferForFrame(CmdBufferIndex), SourceBuffer->GetFrameBufferExtent(), BeginCmd->RenderPass, BeginCmd->bUseSecondary);
			}
			else
			{
				Ry::Log->LogError("VulkanCommandBuffer2::ParseOp: Specified non vulkan frame buffer for begin render pass");
			}

		}

		if (NextOpCode == OP_END_RENDER_PASS)
		{
			// No data for this command
			RecordEndRenderPass(CurrentCmdBuffer);
		}

		if (NextOpCode == OP_BIND_PIPELINE)
		{
			BindPipelineCommand* Cmd = ExtractToken<BindPipelineCommand>(Marker, Data);
			RecordBindPipeline(CurrentCmdBuffer, Cmd->Pipeline);
		}

		if (NextOpCode == OP_BIND_RESOURCE_SET)
		{
			BindResourcesCommand* Cmd = ExtractToken<BindResourcesCommand>(Marker, Data);
			RecordBindResourceSet(CurrentCmdBuffer, CmdBufferIndex, Cmd);
		}

		if (NextOpCode == OP_SET_VIEWPORT_SIZE)
		{
			SetViewportSizeCmd* Cmd = ExtractToken<SetViewportSizeCmd>(Marker, Data);
			RecordSetViewportSize(CurrentCmdBuffer, Cmd->ViewportX, Cmd->ViewportY, Cmd->ViewportWidth, Cmd->ViewportHeight, Cmd->TargetWidth, Cmd->TargetHeight);
		}

		if (NextOpCode == OP_SET_SCISSOR_SIZE)
		{
			SetViewportScissorCmd* Cmd = ExtractToken<SetViewportScissorCmd>(Marker, Data);
			RecordSetScissorSize(CurrentCmdBuffer, Cmd->ScissorX, Cmd->ScissorY, Cmd->ScissorWidth, Cmd->ScissorHeight, Cmd->TargetWidth, Cmd->TargetHeight);
		}

		if (NextOpCode == OP_DRAW_VERTEX_ARRAY)
		{
			DrawVertexArrayCommand* Cmd = ExtractToken<DrawVertexArrayCommand>(Marker, Data);
			RecordDrawVertexArray(CurrentCmdBuffer, Cmd->VertexArray, Cmd->FirstVertex, Cmd->VertexCount);
		}

		if (NextOpCode == OP_DRAW_VERTEX_ARRAY_INDEXED)
		{
			DrawVertexArrayIndexedCommand* Cmd = ExtractToken<DrawVertexArrayIndexedCommand>(Marker, Data);
			RecordDrawVertexArrayIndexed(CurrentCmdBuffer, Cmd->VertexArray, Cmd->FirstIndex, Cmd->IndexCount);
		}

		if (NextOpCode == OP_COMMAND_BUFFER)
		{
			CommandBufferCommand* Cmd = ExtractToken<CommandBufferCommand>(Marker, Data);
			RecordCommandBuffer(CurrentCmdBuffer, CmdBufferIndex, dynamic_cast<VulkanCommandBuffer*>(Cmd->CmdBuffer));
		}

	}

	bool VulkanCommandBuffer::CheckDirty()
	{
		VulkanSwapChain* VkSC = dynamic_cast<VulkanSwapChain*>(Swap);		
		int32 FrameIndex = VkSC->GetCurrentImageIndex();

		//CORE_ASSERT(FrameIndex >= 0);

		if (DirtyImages.Contains(FrameIndex))
		{
			// Rebuild this buffer
			RecordForIndex(FrameIndex);

			DirtyImages.Remove(FrameIndex);

			return true;
		}

		return false;
	}

	void VulkanCommandBuffer::ForceRecord()
	{
		VulkanSwapChain* VkSC = dynamic_cast<VulkanSwapChain*>(Swap);

		// TODO: CAN CRASH WITH INVALID RENDERPASS - should recreate as soon as screen is resized
		vkDeviceWaitIdle(GVulkanContext->GetLogicalDevice());

		// Recreate command buffers (swap chain image count could have changed for all we know)
		//FreeBuffers();
		//CreateBuffers();

		// Re-record all children buffers 

		for (uint32 Fb = 0; Fb < VkSC->SwapChainImageViews.GetSize(); Fb++)
		{
			RecordForIndex(Fb);
		}

		SwapChainVersion = VkSC->GetSwapchainVersion();
	}

	void VulkanCommandBuffer::Submit()
	{
		VulkanSwapChain* VkSC = dynamic_cast<VulkanSwapChain*>(Swap);

		if(SwapChainVersion != VkSC->GetSwapchainVersion())
		{
			// Force re-create all children
			for(CommandBuffer* Child : SecondaryBuffers)
			{
				VulkanCommandBuffer* VkCmd = dynamic_cast<VulkanCommandBuffer*>(Child);

				VkCmd->ForceRecord();
			}
			
			ForceRecord();
		}
		else if (!bHasUpdatedThisFrame)
		{
			int32 FrameIndex = VkSC->GetCurrentImageIndex();

			if(DirtyImages.Contains(FrameIndex))
			{
				// Rebuild this buffer
				RecordForIndex(FrameIndex);
				
				DirtyImages.Remove(FrameIndex);
			}
			// Need to rebuild old
		}

		VkSC->SubmitBuffer(GeneratedBuffers[VkSC->GetCurrentImageIndex()]);

		bHasUpdatedThisFrame = false;
	}

	void VulkanCommandBuffer::BeginCmd()
	{
		Reset(); // Always implicitly reset command buffer
	}

	void VulkanCommandBuffer::EndCmd()
	{
		if (!Swap)
		{
			Ry::Log->LogError("Only command buffers targeting swap chain framebuffers are supported for Vulkan the moment");
			return;
		}

		// Record a primary command buffer for each framebuffer within the swap chain
		if (Swap)
		{
			VulkanSwapChain* VkSC = dynamic_cast<VulkanSwapChain*>(Swap);

			// First check if we're recording this during a frame
			int32 CurrentImageIndex = VkSC->GetCurrentImageIndex();

			Ry::ArrayList<uint32> BuffersToRecord;

			if(CurrentImageIndex < 0)
			{
				// Record all buffers, have to perform device wait to ensure no operations
				vkDeviceWaitIdle(GVulkanContext->GetLogicalDevice());

				for(uint32 Fb = 0; Fb < VkSC->SwapChainImageViews.GetSize(); Fb++)
				{
					BuffersToRecord.Add(Fb);
				}

			}
			else
			{
				// Synchronization is guaranteed, record to the current command buffer and update next frames later if needed
				BuffersToRecord.Add(CurrentImageIndex);

				// Dirty the images that are not the current one
				for (int32 Fb = 0; Fb < VkSC->SwapChainImageViews.GetSize(); Fb++)
				{
					if(Fb != CurrentImageIndex && !DirtyImages.Contains(Fb)) // todo: this should be an efficient set
					{
						DirtyImages.Add(Fb);
					}
				}

			}

			for(uint32 Buffer = 0; Buffer < BuffersToRecord.GetSize(); Buffer++)
			{
				RecordForIndex(BuffersToRecord[Buffer]);
			}

		}

		if(bImmediate)
		{
			// Submit
		}

		if(bOneTimeUse)
		{
			// Free command
			FreeBuffers();
		}

		bHasUpdatedThisFrame = true;
		
	}

	void VulkanCommandBuffer::VkBeginCmd(VkCommandBuffer NewBuffer)
	{
		//vkResetCommandBuffer(NewBuffer, 0);
		
		VkCommandBufferBeginInfo BeginInfo{};
		BeginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		BeginInfo.pInheritanceInfo = nullptr;

		// Pass in information if a secondary command buffer
		if(SecondaryInfo.bSecondary)
		{
			VulkanRenderPass* VkRenderPass = dynamic_cast<VulkanRenderPass*>(SecondaryInfo.ParentRenderPass);
			
			// Setup renderpass inheritance info			
			VkCommandBufferInheritanceInfo InheritanceInfo{};
			InheritanceInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO;
			InheritanceInfo.pNext = nullptr;
			InheritanceInfo.framebuffer = VK_NULL_HANDLE;
			InheritanceInfo.occlusionQueryEnable = false;
			InheritanceInfo.subpass = 0; // TODO: need to specify this as well
			InheritanceInfo.renderPass = VkRenderPass->GetRenderPass();
			
			BeginInfo.pInheritanceInfo = &InheritanceInfo;
			BeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT;
		}

		if (bOneTimeUse)
		{
			BeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		}
		else
		{
			BeginInfo.flags |= VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;
		}

		if (vkBeginCommandBuffer(NewBuffer, &BeginInfo) != VK_SUCCESS)
		{
			Ry::Log->LogError("Failed to begin recording command buffer");
		}
	}

	void VulkanCommandBuffer::VkEndCmd(VkCommandBuffer NewBuffer)
	{
		if (vkEndCommandBuffer(NewBuffer) != VK_SUCCESS)
		{
			Ry::Log->LogError("Failed to end recording command buffer");
		}

		//SubmitBuffer(bIsImmediate);
		//vkFreeCommandBuffers(GVulkanContext->GetLogicalDevice(), GVulkanContext->CommandPool, 1, &CmdBuffer);


		// if (bIsImmediate)
		// {
		// 	SubmitBuffer(bIsImmediate);
		// 	vkFreeCommandBuffers(GVulkanContext->GetLogicalDevice(), GVulkanContext->CommandPool, 1, &CmdBuffer);
		// }
	}

	void VulkanCommandBuffer::RecordForIndex(int32 Index)
	{
		VulkanSwapChain* VkSC = dynamic_cast<VulkanSwapChain*>(Swap);

		VkCommandBuffer BufferToRecord = GeneratedBuffers[Index];

		VkBeginCmd(BufferToRecord);
		{
			// Begin readback of the recorded command
			uint32 ReadBack = 0;

			while (ReadBack < Marker)
			{
				// Parse op will directly change marker val
				ParseOp(BufferToRecord, Index, CmdBuffer, ReadBack);
			}
		}
		VkEndCmd(BufferToRecord);
	}

	bool VulkanCommandBuffer::CreateCmdBufferResource(VkCommandBuffer& Result)
	{
		VkCommandBufferAllocateInfo AllocInfo{};
		AllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		AllocInfo.commandPool = GVulkanContext->GetCommandPool();
		AllocInfo.commandBufferCount = 1;// (uint32_t)CommandBuffers.size();

		if(SecondaryInfo.bSecondary)
		{
			AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		}
		else
		{
			AllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		}

		if (vkAllocateCommandBuffers(GVulkanContext->GetLogicalDevice(), &AllocInfo, &Result) == VK_SUCCESS)
		{
			return true;
		}
		else
		{
			return false;
		}
	}

	void VulkanCommandBuffer::FreeCmdBufferResource(VkCommandBuffer Buff)
	{
		vkFreeCommandBuffers(GVulkanContext->GetLogicalDevice(), GVulkanContext->GetCommandPool(), 1, &Buff);
	}


}
