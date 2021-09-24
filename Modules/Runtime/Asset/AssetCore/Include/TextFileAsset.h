#pragma once

#include "Asset.h"

namespace Ry
{
	class ASSETCORE_MODULE TextFileAsset : public Asset
	{
	public:

		TextFileAsset() = default;
		TextFileAsset(const Ry::String& Content);
		virtual ~TextFileAsset() = default;

		const Ry::String& GetContents() const;
		void UnloadAsset() override;

	private:
		Ry::String Contents;

	};
}