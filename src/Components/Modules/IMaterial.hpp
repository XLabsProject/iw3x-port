#pragma once

namespace Components
{
	class IMaterial : public AssetHandler::AssetInterface
	{
	public:
		IMaterial();
		~IMaterial();

		const char* getName() override { return "IMaterial"; };
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_MATERIAL; };
		void dump(Game::IW3::XAssetHeader header) override { Dump(header.material); };

	private:
		static std::unordered_map<char, char> sortKeysTable;

		static void Dump(Game::IW3::Material* material);

		static void SaveConvertedMaterial(Game::IW4::Material* asset);

		static char GetConvertedSortKey(Game::IW3::Material* material);
	};
}
