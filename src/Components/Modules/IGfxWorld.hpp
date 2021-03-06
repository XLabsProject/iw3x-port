#pragma once

namespace Components
{
	class IGfxWorld : public AssetHandler::AssetInterface
	{
	public:
		IGfxWorld();
		~IGfxWorld();

		const char* getName() override { return "IGfxWorld"; };
		Game::XAssetType getType() override { return Game::XAssetType::ASSET_TYPE_GFXWORLD; };
		void dump(Game::IW3::XAssetHeader header) override { Dump(header.gfxWorld); };

	private:
		static void Dump(Game::IW3::GfxWorld* world);

		static void SaveConvertedWorld(Game::IW4::GfxWorld* asset);
		static void SaveGfxWorldDpvsPlanes(Game::IW4::GfxWorld* world, Game::IW3::GfxWorldDpvsPlanes* asset, Utils::Stream* buffer);
		static void SaveGfxWorldDraw(Game::IW4::GfxWorldDraw* asset, Utils::Stream* buffer);
		static void SaveGfxLightGrid(Game::IW3::GfxLightGrid* asset, Utils::Stream* buffer);
		static void SaveGfxWorldDpvsStatic(Game::IW4::GfxWorld* world, Game::IW4::GfxWorldDpvsStatic* asset, Utils::Stream* buffer);
	};
}
