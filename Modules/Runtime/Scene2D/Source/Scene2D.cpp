#include "Scene2D.h"
#include "RenderAPI.h"
#include "SwapChain.h"
#include "RenderCommand.h"
#include "Timer.h"
#include "Camera.h"
#include "Font.h"

namespace Ry
{
	
	ScenePrimitive2D::ScenePrimitive2D(PrimitiveMobility Mobility)
	{
		ItemSet = MakeItemSet();
		PipelineId = "Texture";
		this->Mobility = Mobility;
	}

	Ry::SharedPtr<BatchItemSet> ScenePrimitive2D::GetItemSet()
	{
		return ItemSet;
	}

	PrimitiveMobility ScenePrimitive2D::GetMobility()
	{
		return Mobility;
	}

	int32 ScenePrimitive2D::GetLayer()
	{
		return Layer;
	}

	Ry::String ScenePrimitive2D::GetPipelineId()
	{
		return PipelineId;
	}

	void ScenePrimitive2D::SetLayer(int32 Layer)
	{
		this->Layer = Layer;
		bPrimitiveStateDirty = true;
	}

	Texture* ScenePrimitive2D::GetTexture()
	{
		return nullptr;
	}

	QuadScenePrimitive::QuadScenePrimitive(PrimitiveMobility Mobility, Ry::Vector2 Size):
	ScenePrimitive2D(Mobility)
	{
		this->Size = Size;
	}

	ParticleEmitterPrimitive::ParticleEmitterPrimitive(PrimitiveMobility Mobility, Texture* ParticleTexture):
	ScenePrimitive2D(Mobility)
	{
		this->ParticleTexture = ParticleTexture;

		if(ParticleTexture)
		{
			PipelineId = "Texture";
		}
		else
		{
			PipelineId = "Shape";
		}
		
	}

	Texture* ParticleEmitterPrimitive::GetTexture()
	{
		return ParticleTexture;
	}

	void ParticleEmitterPrimitive::Draw(Ry::Matrix3 Transform, Ry::Vector2 Origin)
	{		
		OAPairIterator<Particle*, Ry::SharedPtr<BatchItem>> PartItr = Particles.CreatePairIterator();
		while(PartItr)
		{
			Particle* Part = PartItr.GetKey();
			Ry::SharedPtr<BatchItem> PartItem = PartItr.GetValue();

			PartItem->Clear();

			if(ParticleTexture)
			{
				Ry::BatchTextureTransform(PartItem, Part->Tint, Part->Transform.AsMatrix(),
					Part->Texture.GetUx(),
					Part->Texture.GetVy(),
					Part->Texture.GetUw(),
					Part->Texture.GetVh(),
					0.5, // assume particles always rotate/scale around their center for now
					0.5,
					Part->Width,
					Part->Height,
					0.0f);
			}
			else
			{
				Ry::BatchRectangleTransform(PartItem, Part->Tint, Part->Transform.AsMatrix(),
					Part->Width, // assume particles always rotate/scale around their center for now
					Part->Height,
					0.5,
					0.5f,
					0.0f);
			}
			
			++PartItr;
		}
	}

	void ParticleEmitterPrimitive::AddParticle(SharedPtr<Particle> Part)
	{
		// Acquire batch item
		if(!Particles.Contains(Part.Get()))
		{
			Ry::SharedPtr<BatchItem> Result;
			if(FreeItems.GetSize() <= 0)
			{
				// Create new item
				Result = MakeItem();
			}
			else
			{
				Result = FreeItems[FreeItems.GetSize() - 1];
				FreeItems.RemoveAt(FreeItems.GetSize() - 1);
			}

			ItemSet->AddItem(Result);

			if(Result)
			{
				Particles.Insert(Part.Get(), Result);
			}
			else
			{
				std::cerr << "ERR" << std::endl;
			}
		}
	}

	void ParticleEmitterPrimitive::RemoveParticle(SharedPtr<Particle> Part)
	{
		if(Particles.Contains(Part.Get()))
		{
			Ry::SharedPtr<BatchItem> Item = Particles.Get(Part.Get());
			ItemSet->Items.Remove(Item);

			Item->Clear();


			FreeItems.Add(Item);

			Particles.Remove(Part.Get());
		}
	}

	TextScenePrimitive::TextScenePrimitive(PrimitiveMobility Mobility, Ry::Vector2 Size, Ry::String Text, BitmapFont* Font):
	ScenePrimitive2D(Mobility)
	{
		this->Width = Size.x;
		this->Font = Font;
		this->PipelineId = "Font";
		ComputeTextData(TextData, Text);
	}

	Texture* TextScenePrimitive::GetTexture()
	{
		return Font->GetAtlasTexture();
	}

	void TextScenePrimitive::Draw(Ry::Matrix3 Transform, Ry::Vector2 Origin)
	{
		Ry::BatchText(ItemSet, WHITE, 
			Font, 
			TextData, 
			Transform[0][2], Transform[1][2], 
			Width
		);
	}

	TextureScenePrimitive::TextureScenePrimitive(PrimitiveMobility Mobility, Ry::Vector2 Size, const TextureRegion& Region):
	QuadScenePrimitive(Mobility, Size)
	{
		this->Texture = Region;

		Item = MakeItem();
		ItemSet->Items.Add(Item);
	}

	Texture* TextureScenePrimitive::GetTexture()
	{
		return Texture.Parent;
	}

	void TextureScenePrimitive::Draw(Ry::Matrix3 Transform, Ry::Vector2 Origin)
	{

		Ry::BatchTextureTransform(Item, WHITE, Transform,
			Texture.GetUx(),
			Texture.GetVy(),
			Texture.GetUw(),
			Texture.GetVh(),
			Origin.x,
			Origin.y,
			Size.x,
			Size.y,
			0.0f);
	}

	RectScenePrimitive::RectScenePrimitive(PrimitiveMobility Mobility, Ry::Vector2 Size, Color RectColor):
	QuadScenePrimitive(Mobility, Size)
	{
		PipelineId = "Shape";
		
		this->RectColor = RectColor;
		Item = MakeItem();
	}

	Texture* RectScenePrimitive::GetTexture()
	{
		return nullptr;
	}

	void RectScenePrimitive::Draw(Ry::Matrix3 Transform, Ry::Vector2 Origin)
	{
		ItemSet->Items.Clear();
		ItemSet->AddItem(Item);

		Item->Clear();

		Ry::BatchRectangleTransform(Item, RectColor, Transform,
			Size.x, Size.y,
			Origin.x, Origin.y,
			0.0f);
	}

	AnimationScenePrimitive::AnimationScenePrimitive(PrimitiveMobility Mobility, Ry::Vector2 Size, Ry::SharedPtr<Animation> Anim):
	QuadScenePrimitive(Mobility, Size)
	{
		this->Anim = Anim;
		this->FrameIndex = 0;

		AnimTimer = new Timer;

		if(Anim.IsValid())
		{
			AnimTimer->set_delay(Anim->GetSpeed());
		}

		// Create animation item
		Item = MakeItem();
		ItemSet->Items.Add(Item);

		this->bPlaying = true;
	}

	AnimationScenePrimitive::~AnimationScenePrimitive()
	{
		delete AnimTimer;
	}

	Texture* AnimationScenePrimitive::GetTexture()
	{
		return Anim->GetParent();
	}

	void AnimationScenePrimitive::Draw(Ry::Matrix3 Transform, Ry::Vector2 Origin)
	{
		if (Anim->GetNumFrames() <= 0)
			return;
		
		if(AnimTimer->is_ready())
		{
			FrameIndex++;
		}

		FrameIndex = FrameIndex % Anim->GetNumFrames();

		TextureRegion& CurFrame = Anim->GetFrame(FrameIndex);

		Ry::BatchTextureTransform(Item, WHITE, Transform,
			CurFrame.GetUx(),
			CurFrame.GetVy(),
			CurFrame.GetUw(),
			CurFrame.GetVh(),
			Origin.x,
			Origin.y,
			Size.x,
			Size.y,
			0.0f);
	}

	void AnimationScenePrimitive::Pause(int32 AtFrame)
	{
		this->bPlaying = false;
		this->FrameIndex = AtFrame;
	}

	void AnimationScenePrimitive::Play()
	{
		this->bPlaying = true;
	}

	void AnimationScenePrimitive::SetAnim(SharedPtr<Animation> Anim)
	{
		this->Anim = Anim;
		if (Anim.IsValid())
		{
			AnimTimer->set_delay(Anim->GetSpeed());
		}
	}

	void AnimationScenePrimitive::SetDelay(float Delay)
	{
		AnimTimer->set_delay(Delay);
	}

	Scene2D::Scene2D(Ry::SwapChain* Parent)
	{

	}

	void Scene2D::AddPrimitive(Ry::SharedPtr<ScenePrimitive2D> Primitive)
	{
		OnPrimitiveAdded.Broadcast(Primitive);
	}

	void Scene2D::RemovePrimitive(Ry::SharedPtr<ScenePrimitive2D> Primitive)
	{
		OnPrimitiveRemoved.Broadcast(Primitive);
	}

	/*void Scene2D::OnItemSetDirty(Ry::ScenePrimitive2D* Prim)
	{
		Batch* Bat = nullptr;
		if(Prim->GetMobility() == Ry::PrimitiveMobility::Movable)
			Bat = DynamicBatch;
		else if (Prim->GetMobility() == Ry::PrimitiveMobility::Static)
			Bat = StaticBatch;

		if (Bat)
		{
			PipelineState State;
			Bat->RemoveItemSet(Prim->GetItemSet());
			Bat->AddItemSet(Prim->GetItemSet(), Prim->GetPipelineId(), State, Prim->GetTexture(), Prim->GetLayer());
		}

	}*/

	Scene2DRenderer::Scene2DRenderer(Scene2D* Target, SwapChain* ParentSC, uint32 Width, uint32 Height)
	{
		this->TargetScene = Target;

		this->SC = ParentSC;

		Target->OnPrimitiveAdded.AddMemberFunction(this, &Scene2DRenderer::AddPrimitive);
		Target->OnPrimitiveRemoved.AddMemberFunction(this, &Scene2DRenderer::RemovePrimitive);

		// Init the resources required to render this scene
		InitResources(Width, Height);

	}

	void Scene2DRenderer::UpdateStatic()
	{
		StaticBatch->Update();
	}

	const ColorAttachment* Scene2DRenderer::GetSceneTexture() const
	{
		return OffScreenFbo->GetColorAttachment(ColorAttachment);
	}

	void Scene2DRenderer::InitResources(uint32 Width, uint32 Height)
	{
		ColorAttachment = OffScreenDesc.AddColorAttachment(); // Extra color attachment

		// Create off-screen pass. Shouldn't need to be re-created on re-size.
		OffScreenPass = Ry::RendAPI->CreateRenderPass();
		OffScreenPass->SetFramebufferDescription(OffScreenDesc);
		{
			int32 OffScreenSub = OffScreenPass->CreateSubpass();
			OffScreenPass->AddSubpassAttachment(OffScreenSub, ColorAttachment);
		}
		OffScreenPass->CreateRenderPass();

		OffScreenFbo = Ry::RendAPI->CreateFrameBuffer(Width, Height, OffScreenPass, OffScreenDesc);

		// Create batches
		DynamicBatch = new Batch(SC, OffScreenPass);
		StaticBatch = new Batch(SC, OffScreenPass);

		AllBatches = { StaticBatch, DynamicBatch };
	}

	void Scene2DRenderer::Update(float Delta)
	{
		// Update dynamic primitives and dynamic batch

		DynamicBatch->Update();
	}

	void Scene2DRenderer::Render(Ry::Camera2D& Cam)
	{
		DynamicBatch->Camera(&Cam);
		StaticBatch->Camera(&Cam);

		// Render batches
		DynamicBatch->Render();
		StaticBatch->Render();
	}

	void Scene2DRenderer::DrawCommands(CommandBuffer* DstBuffer)
	{
		DstBuffer->BeginRenderPass(OffScreenPass, OffScreenFbo, true);
		{
			int32 MaxBatchLength = 0;
			for (Batch* Custom : AllBatches)
				if (Custom->GetLayerCount() > MaxBatchLength)
					MaxBatchLength = Custom->GetLayerCount();

			int32 CurrentLayer = 0;
			while (CurrentLayer < MaxBatchLength)
			{
				for (int32 BatchIndex = 0; BatchIndex < AllBatches.GetSize(); BatchIndex++)
				{
					Batch* Bat = AllBatches[BatchIndex];
					if (CurrentLayer < Bat->GetLayerCount())
					{
						CommandBuffer* BatBuffer = Bat->GetCommandBuffer(CurrentLayer);
						DstBuffer->DrawCommandBuffer(BatBuffer);
					}
				}

				CurrentLayer++;
			}
		}
		DstBuffer->EndRenderPass();
	}

	void Scene2DRenderer::Resize(int32 Width, int32 Height)
	{
		OffScreenFbo->Recreate(Width, Height, OffScreenPass);

		// Update batches
		//StaticBatch->SetRenderPass(OffScreenPass);
		//DynamicBatch->SetRenderPass(OffScreenPass);

		StaticBatch->Resize(Width, Height);
		DynamicBatch->Resize(Width, Height);
	}

	void Scene2DRenderer::AddPrimitive(Ry::SharedPtr<ScenePrimitive2D> Primitive)
	{
		if (Primitive->GetMobility() == Ry::PrimitiveMobility::Movable)
		{
			// Dynamic batch, primitive's position can update on the fly
			AddDynamicPrimitive(Primitive);
		}
		else if (Primitive->GetMobility() == Ry::PrimitiveMobility::Static)
		{
			// Static batch, primitive's position won't be updated
			AddStaticPrimitive(Primitive);
		}
	}

	void Scene2DRenderer::RemovePrimitive(Ry::SharedPtr<ScenePrimitive2D> Primitive)
	{
		if (Primitive->GetMobility() == Ry::PrimitiveMobility::Static)
		{
			RemoveStaticPrimitive(Primitive);
		}
		else if (Primitive->GetMobility() == Ry::PrimitiveMobility::Movable)
		{
			RemoveDynamicPrimitive(Primitive);
		}
	}

	void Scene2DRenderer::AddCustomBatch(Batch* Batch)
	{
		CustomBatches.Add(Batch);
		AllBatches.Add(Batch);
	}

	void Scene2DRenderer::AddDynamicPrimitive(Ry::SharedPtr<ScenePrimitive2D> Primitive)
	{
		PipelineState State;
		DynamicBatch->AddItemSet(Primitive->GetItemSet(), Primitive->GetPipelineId(), State, Primitive->GetTexture(), Primitive->GetLayer());
		DynamicBatch->Update();
	}

	void Scene2DRenderer::RemoveDynamicPrimitive(Ry::SharedPtr<ScenePrimitive2D> Primitive)
	{
		DynamicBatch->RemoveItemSet(Primitive->GetItemSet());
	}

	void Scene2DRenderer::AddStaticPrimitive(Ry::SharedPtr<ScenePrimitive2D> Primitive)
	{
		PipelineState State;
		StaticBatch->AddItemSet(Primitive->GetItemSet(), Primitive->GetPipelineId(), State, Primitive->GetTexture(), Primitive->GetLayer());

		StaticBatch->Update(); // Immediately update static batch
	}

	void Scene2DRenderer::RemoveStaticPrimitive(Ry::SharedPtr<ScenePrimitive2D> Primitive)
	{
		StaticBatch->RemoveItemSet(Primitive->GetItemSet());

		StaticBatch->Update(); // Immediately update static batch
	}


	
}
