#include "STDInclude.hpp"

#define IW4X_MAT_VERSION 1

namespace Components
{
	std::unordered_map<char, char> IMaterial::sortKeysTable = {
		{0, 43},	// Distortion
					// Opaque water (never used)
					// Boat hull (never used)
		{3, 0},		// Opaque ambient
		{4, 1},		// Opaque
		{5, 2},		// Sky
		{6, 3},		// Skybox sun/moon
		{7, 4},		// Clouds  NOT SURE
		{8, 5},		// Horizon NOT SURE
		{9, 6},		// Decal bottom 1
		{10, 7},	// Decal bottom 2
		{11, 8},	// Decal bottom 3
		{12, 9},	// Decal static
		{13, 10},	// Decal mid 1
		{14, 11},	// Decal mid 2
		{15, 12},	// Decal mid 3
		{24, 13},	// Weapon Impact
		{29, 14},	// Decal top 1
		{30, 15},	// Decal top 2
		{31, 16},	// Decal top 3
		{32, 41},	// Multiplicative
		{33, 17},	// Banner/ Curtain (wild guess!)
		{34, 4 },	// Hair. I matched it with german shepherd material sortkey, i hope its ok.
		{35, 19},	// Underwater
		{36, 20},	// Transparent water
		{37, 33},	// Corona (wild guess)
		{38, 24},	// Window inside
		{39, 25},	// Window outside
		{40, 44},	// Before effects 1 (wild guess)
		{41, 45},	// Before effects 2 (wild guess)
		{42, 46},	// Before effects 3 (extremely wild guess)
		{43, 29},	// Blend / additive => to a decal layer
		{48, 48},	// Effect auto sort!
		{56, 49},	// AE Bottom
		{57, 50},	// AE Middle
		{58, 51},	// AE top
		{59, 53}	// Viewmodel effect
	};

	void IMaterial::SaveConvertedMaterial(Game::IW4::Material* asset)
	{
		Utils::Memory::Allocator strDuplicator;
		rapidjson::Document output(rapidjson::kObjectType);
		auto& allocator = output.GetAllocator();

		output.AddMember("version", IW4X_MAT_VERSION, allocator);

		output.AddMember("name", RAPIDJSON_STR(asset->name), allocator);

		const auto gameFlags = std::format("{:b}", asset->gameFlags.packed);
		output.AddMember("gameFlags", RAPIDJSON_STR(gameFlags.c_str()), allocator);

		const auto stateFlags = std::format("{:b}", static_cast<char>(asset->stateFlags));
		output.AddMember("stateFlags", RAPIDJSON_STR(stateFlags.c_str()), allocator);

#define SAME_NAME_JSON_MEMBER(x) output.AddMember(#x, asset->x, allocator)

		SAME_NAME_JSON_MEMBER(sortKey);

		if (asset->techniqueSet)
		{
			output.AddMember("techniqueSet", RAPIDJSON_STR(asset->techniqueSet->name), allocator);
			AssetHandler::Dump(Game::XAssetType::ASSET_TYPE_TECHNIQUE_SET, { asset->techniqueSet });
		}


		SAME_NAME_JSON_MEMBER(textureAtlasRowCount);
		SAME_NAME_JSON_MEMBER(textureAtlasColumnCount);

		const auto surfaceTypeBits = std::format("{:b}", asset->surfaceTypeBits);
		output.AddMember("surfaceTypeBits", RAPIDJSON_STR(surfaceTypeBits.c_str()), allocator);

		rapidjson::Value textureTable(rapidjson::kArrayType);

		if (asset->textureTable)
		{
			for (char i = 0; i < asset->textureCount; ++i)
			{
				Game::IW3::MaterialTextureDef* textureDef = &asset->textureTable[i];
				rapidjson::Value textureJson(rapidjson::kObjectType);

				textureJson.AddMember("nameStart", textureDef->nameStart, allocator);
				textureJson.AddMember("nameEnd", textureDef->nameEnd, allocator);
				textureJson.AddMember("nameHash", textureDef->nameHash, allocator);
				textureJson.AddMember("samplerState", textureDef->samplerState, allocator); // $6961E030A9677F7C86FC6FF9B5901495
				textureJson.AddMember("semantic", textureDef->semantic, allocator);

				if (textureDef->semantic == TS_WATER_MAP)
				{
					AssertSize(Game::IW3::water_t, 68);

					Game::IW3::water_t* water = textureDef->u.water;

					if (water)
					{
						rapidjson::Value waterJson(rapidjson::kObjectType);

						if (water->image)
						{
							waterJson.AddMember("image", RAPIDJSON_STR(water->image->name), allocator);
							AssetHandler::Dump(Game::XAssetType::ASSET_TYPE_IMAGE, { water->image });
						}

						// Save_water_t
						if (water->H0)
						{
							auto buffer = std::vector<uint8_t>(water->M * water->N * sizeof(Game::IW3::complex_s));
							memcpy_s(&buffer, buffer.size(), water->H0, buffer.size());

							waterJson.AddMember("H0", RAPIDJSON_STR(strDuplicator.duplicateString(Utils::Base64::Encode(buffer))), allocator);
						}

						if (water->wTerm)
						{
							auto buffer = std::vector<uint8_t>(water->M * water->N * sizeof(float));
							memcpy_s(&buffer, buffer.size(), water->wTerm, buffer.size());

							waterJson.AddMember("wTerm", RAPIDJSON_STR(strDuplicator.duplicateString(Utils::Base64::Encode(buffer))), allocator);
						}

						#define SAME_NAME_WATER_MEMBER(x) waterJson.AddMember("x", water->x, allocator)

						SAME_NAME_WATER_MEMBER(M);
						SAME_NAME_WATER_MEMBER(N);
						SAME_NAME_WATER_MEMBER(Lx);
						SAME_NAME_WATER_MEMBER(Lz);
						SAME_NAME_WATER_MEMBER(gravity);
						SAME_NAME_WATER_MEMBER(windvel);
						waterJson.AddMember("winddir", Utils::MakeJsonArray(water->winddir, 2, allocator), allocator);

						SAME_NAME_WATER_MEMBER(amplitude);
						waterJson.AddMember("codeConstant", Utils::MakeJsonArray(water->codeConstant, 4, allocator), allocator);


						textureJson.AddMember("water", waterJson, allocator);
					}
				}
				else if (textureDef->u.image)
				{
					textureJson.AddMember("image", RAPIDJSON_STR(textureDef->u.image->name), allocator);
					AssetHandler::Dump(Game::XAssetType::ASSET_TYPE_IMAGE, { textureDef->u.image });
				}

				textureTable.PushBack(textureJson, allocator);
			}
		}

		output.AddMember("textureTable", textureTable, allocator);

		rapidjson::Value gfxDrawSurface(rapidjson::kObjectType);

#define SAME_NAME_GFXDRAWSURF_MEMBER(x) gfxDrawSurface.AddMember(#x, asset->drawSurf.fields.##x##, allocator)

		SAME_NAME_GFXDRAWSURF_MEMBER(objectId);
		SAME_NAME_GFXDRAWSURF_MEMBER(reflectionProbeIndex);
		SAME_NAME_GFXDRAWSURF_MEMBER(hasGfxEntIndex);
		SAME_NAME_GFXDRAWSURF_MEMBER(customIndex);
		SAME_NAME_GFXDRAWSURF_MEMBER(materialSortedIndex);
		SAME_NAME_GFXDRAWSURF_MEMBER(prepass);
		SAME_NAME_GFXDRAWSURF_MEMBER(useHeroLighting);
		SAME_NAME_GFXDRAWSURF_MEMBER(sceneLightIndex);
		SAME_NAME_GFXDRAWSURF_MEMBER(surfType);
		SAME_NAME_GFXDRAWSURF_MEMBER(primarySortKey);
		SAME_NAME_GFXDRAWSURF_MEMBER(unused);

		output.AddMember("gfxDrawSurface", gfxDrawSurface, allocator);

		SAME_NAME_JSON_MEMBER(hashIndex);

		rapidjson::Value stateBitsEntry(rapidjson::kArrayType);
		for (size_t i = 0; i < 48; i++)
		{
			stateBitsEntry.PushBack(asset->stateBitsEntry[i], allocator);
		}

		output.AddMember("stateBitsEntry", stateBitsEntry, allocator);

		SAME_NAME_JSON_MEMBER(cameraRegion);

		if (asset->constantTable)
		{
			rapidjson::Value constantTable(rapidjson::kArrayType);

			for (char i = 0; i < asset->constantCount; ++i)
			{
				Game::IW3::MaterialConstantDef constantDef;
				std::memcpy(&constantDef, &asset->constantTable[i], sizeof Game::IW3::MaterialConstantDef);

				rapidjson::Value constantDefJson(rapidjson::kObjectType);

				// This is like the ugliest fix i could come up with.
				// I'm way to sick to even understand what MaterialConstantDef actually contain
				// And for now, this shit seems to work
				if (constantDef.name == "envMapParms"s)
				{
					constantDef.literal[0] *= 0.0875f;
					constantDef.literal[1] *= 0.165f;
					constantDef.literal[2] *= 1.4f;
					constantDef.literal[3] *= 3.2f;
				}

				// "detailScale" might need some work too 🤔

				constantDefJson.AddMember("nameHash", constantDef.nameHash, allocator);
				constantDefJson.AddMember("name", RAPIDJSON_STR(strDuplicator.duplicateString(constantDef.name)), allocator);
				constantDefJson.AddMember("literal", Utils::MakeJsonArray(constantDef.literal, 4, allocator), allocator);

				constantTable.PushBack(constantDefJson, allocator);
			}

			output.AddMember("constantTable", constantTable, allocator);
		}

		std::string b64;
		if (asset->stateBitTable)
		{
			auto size = asset->stateBitsCount * sizeof(Game::IW3::GfxStateBits);
			auto* statebitsHead = reinterpret_cast<std::uint8_t*>(asset->stateBitTable);
			auto buffer = std::vector<uint8_t>(statebitsHead, statebitsHead+size);

			b64 = Utils::Base64::Encode(buffer);
			const char* data = b64.data();
			output.AddMember("stateBitsTable", RAPIDJSON_STR(data), allocator);
		}

		rapidjson::StringBuffer buff;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
		output.Accept(writer);

		Utils::WriteFile(Utils::VA("%s/materials/%s.iw4x.json", AssetHandler::GetExportPath().data(), asset->name), buff.GetString());
	}

	void IMaterial::Dump(Game::IW3::Material* material)
	{
		if (!material) return;
		Utils::Memory::Allocator allocator;

        Game::IW4::GfxDrawSurf newSurf;

		Game::IW4::Material mat;
		ZeroMemory(&mat, sizeof mat);

		mat.name                    = material->info.name;
		mat.gameFlags.packed               = material->info.gameFlags.packed;


		mat.gameFlags.fields.unk8 = material->info.gameFlags.fields.unk7;
		mat.gameFlags.fields.unk7 = material->info.gameFlags.fields.unk8;
			
		mat.sortKey                 = GetConvertedSortKey(material); // Might be recalculated in IW4
		mat.textureAtlasRowCount    = material->info.textureAtlasRowCount;
		mat.textureAtlasColumnCount = material->info.textureAtlasColumnCount;

        newSurf.fields.objectId = material->info.drawSurf.fields.objectId;
        newSurf.fields.reflectionProbeIndex = material->info.drawSurf.fields.reflectionProbeIndex;
        newSurf.fields.customIndex = material->info.drawSurf.fields.customIndex;
        newSurf.fields.materialSortedIndex = material->info.drawSurf.fields.materialSortedIndex;
        newSurf.fields.prepass = material->info.drawSurf.fields.prepass;
        newSurf.fields.sceneLightIndex = material->info.drawSurf.fields.primaryLightIndex;
        newSurf.fields.surfType = material->info.drawSurf.fields.surfType;
        newSurf.fields.primarySortKey = material->info.drawSurf.fields.primarySortKey;
        mat.drawSurf.packed = newSurf.packed;

		mat.surfaceTypeBits         = material->info.surfaceTypeBits;
		mat.hashIndex               = material->info.hashIndex;

		// Set them all to -1 so they're not used if they dont exist in iw3
		std::memset(mat.stateBitsEntry, 0xFF, sizeof(mat.stateBitsEntry));

		mat.stateBitsEntry[Game::IW4::TECHNIQUE_DEPTH_PREPASS]                  = material->stateBitsEntry[Game::IW3::TECHNIQUE_DEPTH_PREPASS];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_BUILD_FLOAT_Z]                  = material->stateBitsEntry[Game::IW3::TECHNIQUE_BUILD_FLOAT_Z];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_BUILD_SHADOWMAP_DEPTH]          = material->stateBitsEntry[Game::IW3::TECHNIQUE_BUILD_SHADOWMAP_DEPTH];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_BUILD_SHADOWMAP_COLOR]          = material->stateBitsEntry[Game::IW3::TECHNIQUE_BUILD_SHADOWMAP_COLOR];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_UNLIT]                          = material->stateBitsEntry[Game::IW3::TECHNIQUE_UNLIT];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_EMISSIVE]                       = material->stateBitsEntry[Game::IW3::TECHNIQUE_EMISSIVE];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_EMISSIVE_DFOG]                  = material->stateBitsEntry[Game::IW3::TECHNIQUE_EMISSIVE];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_EMISSIVE_SHADOW]                = material->stateBitsEntry[Game::IW3::TECHNIQUE_EMISSIVE_SHADOW];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_EMISSIVE_SHADOW_DFOG]           = material->stateBitsEntry[Game::IW3::TECHNIQUE_EMISSIVE_SHADOW];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_BEGIN]                      = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_BEGIN];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT]                            = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_DFOG]                       = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_SUN]                        = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_SUN];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_SUN_DFOG]                   = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_SUN];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_SUN_SHADOW]                 = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_SUN_SHADOW];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_SUN_SHADOW_DFOG]            = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_SUN_SHADOW];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_SPOT]                       = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_SPOT];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_SPOT_DFOG]                  = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_SPOT];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_SPOT_SHADOW]                = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_SPOT_SHADOW];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_SPOT_SHADOW_DFOG]           = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_SPOT_SHADOW];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_OMNI]                       = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_OMNI];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_OMNI_DFOG]                  = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_OMNI];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_OMNI_SHADOW]                = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_OMNI_SHADOW];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_OMNI_SHADOW_DFOG]           = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_OMNI_SHADOW];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED]                  = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED_DFOG]             = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED_SUN]              = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED_SUN];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED_SUN_DFOG]         = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED_SUN];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED_SUN_SHADOW]       = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED_SUN_SHADOW];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED_SUN_SHADOW_DFOG]  = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED_SUN_SHADOW];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED_SPOT]             = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED_SPOT];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED_SPOT_DFOG]        = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED_SPOT];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW]      = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW_DFOG] = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED_SPOT_SHADOW];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED_OMNI]             = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED_OMNI];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED_OMNI_DFOG]        = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED_OMNI];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW]      = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW];
		//mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW_DFOG] = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_INSTANCED_OMNI_SHADOW];

		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIT_END]                        = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIT_END];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIGHT_SPOT]                     = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIGHT_SPOT];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIGHT_OMNI]                     = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIGHT_OMNI];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_LIGHT_SPOT_SHADOW]              = material->stateBitsEntry[Game::IW3::TECHNIQUE_LIGHT_SPOT_SHADOW];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_FAKELIGHT_NORMAL]               = material->stateBitsEntry[Game::IW3::TECHNIQUE_FAKELIGHT_NORMAL];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_FAKELIGHT_VIEW]                 = material->stateBitsEntry[Game::IW3::TECHNIQUE_FAKELIGHT_VIEW];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_SUNLIGHT_PREVIEW]               = material->stateBitsEntry[Game::IW3::TECHNIQUE_SUNLIGHT_PREVIEW];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_CASE_TEXTURE]                   = material->stateBitsEntry[Game::IW3::TECHNIQUE_CASE_TEXTURE];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_WIREFRAME_SOLID]                = material->stateBitsEntry[Game::IW3::TECHNIQUE_WIREFRAME_SOLID];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_WIREFRAME_SHADED]               = material->stateBitsEntry[Game::IW3::TECHNIQUE_WIREFRAME_SHADED];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_DEBUG_BUMPMAP]                  = material->stateBitsEntry[Game::IW3::TECHNIQUE_DEBUG_BUMPMAP];
		mat.stateBitsEntry[Game::IW4::TECHNIQUE_DEBUG_BUMPMAP_INSTANCED]        = material->stateBitsEntry[Game::IW3::TECHNIQUE_DEBUG_BUMPMAP_INSTANCED];

		mat.textureCount   = material->textureCount;
		mat.constantCount  = material->constantCount;
		mat.stateBitsCount = material->stateBitsCount;
		mat.stateFlags = static_cast<Game::IW4::StateFlags>(material->stateFlags); // Correspondance is identical
		mat.cameraRegion   = material->cameraRegion;

		if (mat.cameraRegion == 0x3) {
			// 0x3 is NONE in iw3, but DEPTH_HACK in iw4
			// In iw4 NONE is 0x4
			Logger::Print(Utils::VA("Swapped material %s camera region from 0x3 to 0x4 (NONE)\n", mat.name));
			mat.cameraRegion = 0x4;
		}

		mat.techniqueSet   = material->techniqueSet;
		mat.textureTable   = material->textureTable;
		mat.constantTable  = material->constantTable;
		mat.stateBitTable  = material->stateBitsTable;

		IMaterial::SaveConvertedMaterial(&mat);
	}

	char IMaterial::GetConvertedSortKey(Game::IW3::Material* material) 
	{
		char iw3Key = material->info.sortKey;

		const std::string& name = material->info.name;

		if (Utils::StartsWith(name, "compass"s)) 
		{
			//"blend / additive" => SORTKEY_BLEND_ADDITIVE
			return 47;
		}

		if (Utils::StartsWith(name, "mc/mtl_facade"))
		{
			// Opaque => Opaque_Ambient
			return 0;
		}

		if (IMaterial::sortKeysTable.contains(iw3Key)) {
			return IMaterial::sortKeysTable[iw3Key];
		}

		Components::Logger::Print(Utils::VA("Could not find a fitting key for sortkey %i in material %s\n", iw3Key, material->info.name));

		return iw3Key;
	}


	IMaterial::IMaterial()
	{
		Command::Add("dumpMaterial", [](Command::Params params)
		{
			if (params.Length() < 2) return;
			IMaterial::Dump(Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_MATERIAL, params[1]).material);
		});
	}

	IMaterial::~IMaterial()
	{
	}
}
