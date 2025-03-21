#pragma once

#include "Asset.h"
#include "RenderAssetGen.h"

namespace Ry
{
	class MeshData;
	class Mesh;

	class RENDERASSET_MODULE MeshAsset : public Asset
	{
	public:

		MeshAsset(Ry::MeshData* MeshData);
		
		void UnloadAsset() override;

		Mesh* CreateRuntimeMesh() const;

	private:
		Ry::MeshData* Data;
		mutable Ry::ArrayList<Mesh*> RuntimeMeshes;
		
	};
}
