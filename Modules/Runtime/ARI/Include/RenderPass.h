#pragma once

#include "Core/Core.h"
#include "Core/Globals.h"
#include "RenderPass.gen.h"

namespace Ry
{

	class SwapChain;
	class FrameBuffer;

	enum class AttachmentFormat
	{
		/** The same internal format that the target swap chain uses. */
		Color,
		Depth,
		Stencil
	};

	struct ARI_MODULE ColorAttachment
	{
		virtual ~ColorAttachment() = default;
	};

	class ARI_MODULE AttachmentDescription
	{
	public:

		int32 Samples = 1;
		AttachmentFormat Format;
		SwapChain* ReferencingSwapChain = nullptr;

		// Used if we want to share attachments between framebuffers
		const ColorAttachment* ExistingAttachment = nullptr;
	};

	class ARI_MODULE FrameBufferDescription
	{
	public:

		Ry::ArrayList<AttachmentDescription> Attachments;

		bool bHasDepthAttachment = false;
		bool bHasStencilAttachment = false;

		int32 AddSwapChainColorAttachment(SwapChain* SC)
		{
			AttachmentDescription NewDesc;
			NewDesc.Format = AttachmentFormat::Color;
			NewDesc.ReferencingSwapChain = SC;

			Attachments.Add(NewDesc);

			return Attachments.GetSize() - 1;
		}

		int32 AddColorAttachment(const ColorAttachment* Reference)
		{
			AttachmentDescription NewDesc;
			NewDesc.Format = AttachmentFormat::Color;
			NewDesc.ExistingAttachment = Reference;

			Attachments.Add(NewDesc);

			return Attachments.GetSize() - 1;
		}

		int32 AddSwapChainDepthAttachment(SwapChain* SC)
		{
			if(bHasDepthAttachment)
			{
				Ry::Log->LogError("More than one depth attachment added to framebuffer description!");
			}

			AttachmentDescription NewDesc;
			NewDesc.Format = AttachmentFormat::Depth;
			NewDesc.ReferencingSwapChain = SC;

			Attachments.Add(NewDesc);

			bHasDepthAttachment = true;

			return Attachments.GetSize() - 1;
		}

		int32 AddColorAttachment()
		{
			AttachmentDescription NewDesc;
			NewDesc.Format = AttachmentFormat::Color;

			Attachments.Add(NewDesc);

			return Attachments.GetSize() - 1;
		}

		int32 AddDepthAttachment()
		{
			if(bHasDepthAttachment)
			{
				Ry::Log->LogError("More than one depth attachment added to framebuffer description!");
			}

			AttachmentDescription NewDesc;
			NewDesc.Format = AttachmentFormat::Depth;
			Attachments.Add(NewDesc);

			bHasDepthAttachment = true;

			return Attachments.GetSize() - 1;
		}

		int32 AddStencilAttachment()
		{
			if(bHasStencilAttachment)
			{
				Ry::Log->LogError("More than one stencil attachment added to framebuffer description!");
			}

			AttachmentDescription NewDesc;
			NewDesc.Format = AttachmentFormat::Stencil;

			Attachments.Add(NewDesc);

			bHasStencilAttachment = true;

			return Attachments.GetSize() - 1;
		}
	};

	class ARI_MODULE Subpass
	{
	public:
		Ry::ArrayList<int32> UsedAttachments;
		int32 SubpassIndex;

	private:
		
	};

	// todo: rename render pass "render graph" and subpass "render node" -- i like that naming scheme better for a generic system
	class ARI_MODULE RenderPass
	{
	public:

		RenderPass(): NextSubpassIndex(0){};
		virtual ~RenderPass() = default;

		void SetFramebufferDescription(const FrameBufferDescription& Desc)
		{
			this->Description = Desc;
		}

		int32 CreateSubpass()
		{
			Subpass* NewSub = new Subpass;
			NewSub->SubpassIndex = NextSubpassIndex;
			
			NextSubpassIndex++;

			SubPasses.Add(NewSub);

			return NewSub->SubpassIndex;
		}

		void AddSubpassAttachment(int32 Subpass, int32 Attachment)
		{
			SubPasses[Subpass]->UsedAttachments.Add(Attachment);
		}

		virtual bool CreateRenderPass() = 0;

		virtual void DeleteRenderPass()
		{
			for(Ry::Subpass* Sub : SubPasses)
			{
				delete Sub;
			}
		}

		const FrameBufferDescription& GetFboDescription() const;

	protected:

		FrameBufferDescription Description;
		Ry::ArrayList<Subpass*> SubPasses;
		int32 NextSubpassIndex;
		
	};
	
}