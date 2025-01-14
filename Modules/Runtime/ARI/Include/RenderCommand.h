#pragma once

#include "Core/Core.h"
#include "Core/Globals.h"
#include "SwapChain.h"
#include "RenderCommand.gen.h"

namespace Ry
{

	constexpr uint32 MAX_COMMAND_BUFFER_SIZE = 1024 * 100;

	// Use a single byte for the render opcode
	constexpr uint8 OP_NONE = 0;
	constexpr uint8 OP_DRAW_VERTEX_ARRAY = 1;
	constexpr uint8 OP_DRAW_VERTEX_ARRAY_INDEXED = 2;
	constexpr uint8 OP_BEGIN_RENDER_PASS = 3;
	constexpr uint8 OP_END_RENDER_PASS = 4;
	constexpr uint8 OP_BIND_PIPELINE = 5;
	constexpr uint8 OP_SET_VIEWPORT_SIZE = 6;
	constexpr uint8 OP_SET_SCISSOR_SIZE = 7;
	constexpr uint8 OP_BIND_RESOURCE_SET = 8;
	constexpr uint8 OP_COMMAND_BUFFER = 9;

	class Pipeline;
	class VertexArray;
	class RenderPass;
	class SwapChain;
	class FrameBuffer;
	class ResourceSet;

	struct ARI_MODULE SetViewportSizeCmd
	{
		int32 ViewportX;
		int32 ViewportY;
		int32 ViewportWidth;
		int32 ViewportHeight;
		int32 TargetWidth;
		int32 TargetHeight;
	};

	struct ARI_MODULE SetViewportScissorCmd
	{
		int32 ScissorX;
		int32 ScissorY;
		uint32 ScissorWidth;
		uint32 ScissorHeight;
		int32 TargetWidth;
		int32 TargetHeight;
	};
	
	struct ARI_MODULE BindPipelineCommand
	{
		Ry::Pipeline* Pipeline;
	};

	struct ARI_MODULE BindResourcesCommand
	{
		int32 SetCount = 0;
		ResourceSet** SetPtrs = nullptr;
	};

	struct ARI_MODULE DrawVertexArrayCommand
	{
		Ry::VertexArray* VertexArray;
		int32 FirstVertex = 0;
		int32 VertexCount = 0;
	};

	struct ARI_MODULE DrawVertexArrayIndexedCommand
	{
		Ry::VertexArray* VertexArray;
		int32 FirstIndex = 0;
		int32 IndexCount = 0;
	};

	struct ARI_MODULE CommandBufferCommand
	{
		Ry::CommandBuffer* CmdBuffer = nullptr;
	};

	struct ARI_MODULE BeginRenderPassCommand
	{
		Ry::RenderPass* RenderPass;
		FrameBuffer* SourceBuffer;
		bool bUseSecondary;
	};

	class ARI_MODULE RenderCommand
	{
	public:
		RenderCommand();
		~RenderCommand();

		virtual void Execute() = 0;
		
	};

	struct ARI_MODULE SecondaryCommandBufferInfo
	{
		bool bSecondary = false;
		RenderPass* ParentRenderPass = nullptr;
	};

	class ARI_MODULE CommandBuffer
	{
	public:

		/**
		 * Tells the command buffer to generate a buffer compatible with the swap chain frame buffer.
		 */
		CommandBuffer(SwapChain* SC, SecondaryCommandBufferInfo SecondaryInfo = {}) :
		Marker(0)
		{
			this->Swap = SC;
			this->SecondaryInfo = SecondaryInfo;
			bDirty = false;
			bOneTimeUse = false;
			bImmediate = false;
		}

		// todo: don't force early bind here, do so when beginning render pass
		CommandBuffer(SecondaryCommandBufferInfo Info = {}):
		Marker(0)
		{
			this->Swap = nullptr;
			this->SecondaryInfo = Info;
			
			bDirty = false;
			bOneTimeUse = true;
			bImmediate = false;
		}

		bool IsOneTimeUse()
		{
			return bOneTimeUse;
		}

		void SetOneTimeUse(bool bUseOnce)
		{
			this->bOneTimeUse = bUseOnce;
		}

		void UpdateParentRenderPass(RenderPass* RenderPass)
		{
			this->SecondaryInfo.ParentRenderPass = RenderPass;
		}

		template<typename T>
		T* ExtractToken(uint32& Marker, uint8* Data)
		{
			// Extract the command from the buffer
			T* ExtractedCmd = reinterpret_cast<T*>(Data + Marker);

			// Progress marker forward by command size
			Marker += sizeof(*ExtractedCmd);

			// return result
			return ExtractedCmd;
		}

		virtual void BeginRenderPass(bool bUseSecondary = false)
		{
			if(!Swap)
			{
				Ry::Log->LogError("Must have swap chain as target to specify no render pass");
				return;
			}
			
			BeginRenderPassCommand Cmd{ Swap->GetDefaultRenderPass(), Swap->GetDefaultFrameBuffer(), bUseSecondary};
			PushCmdData(&Cmd, sizeof(Cmd), OP_BEGIN_RENDER_PASS);
		}
		
		virtual void BeginRenderPass(Ry::RenderPass* RenderPass, FrameBuffer* SourceBuffer, bool bUseSecondary)
		{
			BeginRenderPassCommand Cmd{ RenderPass, SourceBuffer, bUseSecondary};
			PushCmdData(&Cmd, sizeof(Cmd), OP_BEGIN_RENDER_PASS);
		}
		
		virtual void EndRenderPass()
		{
			PushCmdDataNoData(OP_END_RENDER_PASS);
		}

		virtual void SetViewportSize(int32 ViewportX, int32 ViewportY, int32 ViewportWidth, int32 ViewportHeight, int32 TargetWidth, int32 TargetHeight)
		{
			SetViewportSizeCmd Cmd{ ViewportX, ViewportY, ViewportWidth, ViewportHeight, TargetWidth, TargetHeight};
			PushCmdData(&Cmd, sizeof(Cmd), OP_SET_VIEWPORT_SIZE);
		}

		virtual void SetScissorSize(int32 ScissorX, int32 ScissorY, uint32 ScissorWidth, uint32 ScissorHeight, int32 TargetWidth, int32 TargetHeight)
		{
			SetViewportScissorCmd Cmd{ ScissorX, ScissorY, ScissorWidth, ScissorHeight, TargetWidth, TargetHeight};
			PushCmdData(&Cmd, sizeof(Cmd), OP_SET_SCISSOR_SIZE);
		}

		virtual void BindPipeline(Ry::Pipeline* Pipeline)
		{
			BindPipelineCommand Cmd{ Pipeline };
			PushCmdData(&Cmd, sizeof(Cmd), OP_BIND_PIPELINE);
		}

		virtual void BindResources(ResourceSet** SetPtrs, int32 SetCount)
		{
			BindResourcesCommand Cmd;
			Cmd.SetPtrs = SetPtrs;
			Cmd.SetCount = SetCount;
			
			PushCmdData(&Cmd, sizeof(Cmd), OP_BIND_RESOURCE_SET);
		}

		virtual void DrawVertexArray(Ry::VertexArray* VertexArray, int32 FirstVertex = 0, int32 Count = 0)
		{
			DrawVertexArrayCommand Cmd{ VertexArray, FirstVertex, Count};
			PushCmdData(&Cmd, sizeof(Cmd), OP_DRAW_VERTEX_ARRAY);
		}

		virtual void DrawVertexArrayIndexed(Ry::VertexArray* VertexArray, int32 FirstIndex = 0, int32 Count = 0)
		{
			DrawVertexArrayIndexedCommand Cmd{ VertexArray, FirstIndex, Count};
			PushCmdData(&Cmd, sizeof(Cmd), OP_DRAW_VERTEX_ARRAY_INDEXED);
		}

		virtual void DrawCommandBuffer(Ry::CommandBuffer* CmdBuffer)
		{
			CommandBufferCommand Cmd{ CmdBuffer };
			PushCmdData(&Cmd, sizeof(Cmd), OP_COMMAND_BUFFER);

			SecondaryBuffers.Add(CmdBuffer);
		}

		virtual void Reset()
		{
			Marker = 0;
			bDirty = true;

			SecondaryBuffers.SoftClear();
		}

		virtual bool CheckDirty() = 0;
		virtual void Submit() = 0;
		virtual void BeginCmd() = 0;
		virtual void EndCmd() = 0;

		SwapChain* GetSwapChain() const
		{
			return Swap;
		}

	protected:

		Ry::ArrayList<CommandBuffer*> SecondaryBuffers;

		SecondaryCommandBufferInfo SecondaryInfo;
		
		bool bDirty;
		SwapChain* Swap;

		void PushCmdDataNoData(uint8 OpCode)
		{
			if (Marker + 1 /* OpCode byte */ >= MAX_COMMAND_BUFFER_SIZE)
			{
				Ry::Log->LogError("Cmd exceeded buffer size");
				return;
			}

			// Set the command OpCode
			CmdBuffer[Marker] = OpCode;
			Marker++;

			bDirty = true;
		}
		
		void PushCmdData(void* Data, int32 DataSize, uint8 OpCode)
		{
			if (Marker + DataSize + 1 /* OpCode byte */ >= MAX_COMMAND_BUFFER_SIZE)
			{
				Ry::Log->LogError("Cmd exceeded buffer size");
				return;
			}

			// Set the command OpCode
			CmdBuffer[Marker] = OpCode;
			Marker++;

			MemCpy(CmdBuffer + Marker, MAX_COMMAND_BUFFER_SIZE - Marker, Data, DataSize);

			Marker += DataSize;

			bDirty = false;
		}
		
		uint8 CmdBuffer [MAX_COMMAND_BUFFER_SIZE];
		uint32 Marker;

		bool bOneTimeUse;
		bool bImmediate;


	};
	
}
