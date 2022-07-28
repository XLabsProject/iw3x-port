#include "STDInclude.hpp"

namespace Components
{
	int Isnd_alias_list_t::channelLookupTable[Game::IW3::SND_CHANNEL_COUNT]
	{
		Game::IW4::SND_CHANNEL_PHYSICS,
		Game::IW4::SND_CHANNEL_AUTO,
		Game::IW4::SND_CHANNEL_AUTO2,
		Game::IW4::SND_CHANNEL_AUTODOG,
		Game::IW4::SND_CHANNEL_BULLETIMPACT,
		Game::IW4::SND_CHANNEL_BULLETWHIZBY,
		Game::IW4::SND_CHANNEL_ELEMENT,
		Game::IW4::SND_CHANNEL_AUTO2D,
		Game::IW4::SND_CHANNEL_VEHICLE,
		Game::IW4::SND_CHANNEL_VEHICLELIMITED,
		Game::IW4::SND_CHANNEL_MENU,
		Game::IW4::SND_CHANNEL_BODY,
		Game::IW4::SND_CHANNEL_BODY2D,
		Game::IW4::SND_CHANNEL_RELOAD,
		Game::IW4::SND_CHANNEL_RELOAD2D,
		Game::IW4::SND_CHANNEL_ITEM,
		Game::IW4::SND_CHANNEL_EFFECTS1,
		Game::IW4::SND_CHANNEL_EFFECTS2,
		Game::IW4::SND_CHANNEL_WEAPON,
		Game::IW4::SND_CHANNEL_WEAPON2D,
		Game::IW4::SND_CHANNEL_NONSHOCK,
		Game::IW4::SND_CHANNEL_VOICE,
		Game::IW4::SND_CHANNEL_LOCAL,
		Game::IW4::SND_CHANNEL_LOCAL2,
		Game::IW4::SND_CHANNEL_AMBIENT,
		Game::IW4::SND_CHANNEL_HURT,
		Game::IW4::SND_CHANNEL_PLAYER1,
		Game::IW4::SND_CHANNEL_PLAYER2,
		Game::IW4::SND_CHANNEL_MUSIC,
		Game::IW4::SND_CHANNEL_MUSICNOPAUSE,
		Game::IW4::SND_CHANNEL_MISSION,
		Game::IW4::SND_CHANNEL_ANNOUNCER,
		Game::IW4::SND_CHANNEL_SHELLSHOCK
	};

	void Isnd_alias_list_t::Dump(Game::IW3::snd_alias_list_t* ents)
	{
		if (ents->count > 32)
		{
			// This is probably garbage data
			return;
		}

		if (ents->count == 0) {
			// Very weird but it happens, notably on mp_crash_snow
			// Soundaliases with a zero-count list crash iw4 so we skip them
			// They should be empty anyway
			return;
		}

		rapidjson::Document output(rapidjson::kObjectType);
		auto& allocator = output.GetAllocator();
		Utils::Memory::Allocator strDuplicator;

		// Format is pretty transparent from iw3 to iw4, so no conversion is necessary!
		rapidjson::Value head(rapidjson::kArrayType);

		for (size_t i = 0; i < ents->count; i++)
		{
			Game::IW3::snd_alias_t iw3Alias = ents->head[i];

			rapidjson::Value channelMaps(rapidjson::kArrayType);

			for (size_t j = 0; j < 2; j++)
			{
				for (size_t k = 0; k < 2; k++)
				{
					auto iw3ChannelMap = iw3Alias.speakerMap->channelMaps[j][k];
					rapidjson::Value speakers(rapidjson::kArrayType);

					for (size_t speakerIndex = 0; speakerIndex < iw3ChannelMap.speakerCount; speakerIndex++)
					{
						auto iw3Speaker = iw3ChannelMap.speakers[speakerIndex];

						rapidjson::Value speaker(rapidjson::kObjectType);
						speaker.AddMember("levels0", iw3Speaker.numLevels > 0 ? iw3Speaker.levels[0] : 0, allocator);
						speaker.AddMember("levels1", iw3Speaker.numLevels > 1 ? iw3Speaker.levels[1] : 0, allocator);
						speaker.AddMember("numLevels", iw3Speaker.numLevels, allocator);
						speaker.AddMember("speaker", iw3Speaker.speaker, allocator);
						speakers.PushBack(speaker, allocator);
					}

					rapidjson::Value channelMap(rapidjson::kObjectType);
					channelMap.AddMember("entryCount", iw3ChannelMap.speakerCount, allocator);
					channelMap.AddMember("speakers", speakers, allocator);
					channelMaps.PushBack(channelMap, allocator);
				}
			}

			rapidjson::Value speakerMap(rapidjson::kObjectType);
			speakerMap.AddMember("channelMaps", channelMaps, allocator);
			speakerMap.AddMember("isDefault", iw3Alias.speakerMap->isDefault, allocator);
			speakerMap.AddMember("name", RAPIDJSON_STR(iw3Alias.speakerMap->name), allocator);

			std::string soundFile("");
			if (iw3Alias.soundFile)
			{
				switch (iw3Alias.soundFile->type)
				{
					// LOADED
				case Game::snd_alias_type_t::SAT_LOADED:
					// Save the LoadedSound subasset
					soundFile = iw3Alias.soundFile->u.loadSnd->name;
					auto header = Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_LOADED_SOUND, soundFile.c_str());
					AssetHandler::Dump(Game::XAssetType::ASSET_TYPE_LOADED_SOUND, header);
					break;

					// STREAMED
				case Game::snd_alias_type_t::SAT_STREAMED:
				{
					soundFile = iw3Alias.soundFile->u.streamSnd.filename.info.raw.name;

					if (iw3Alias.soundFile->u.streamSnd.filename.info.raw.dir)
					{
						soundFile = Utils::VA("%s/%s", iw3Alias.soundFile->u.streamSnd.filename.info.raw.dir, soundFile.c_str());
					}

					auto fullPath = Utils::VA("%s/sound/%s", AssetHandler::GetExportPath().c_str(), soundFile.c_str());
					auto destinationDirectory = Utils::VA("%s/sound/%s", AssetHandler::GetExportPath().c_str(), iw3Alias.soundFile->u.streamSnd.filename.info.raw.dir);
					auto internalPath = Utils::VA("sound/%s", soundFile.c_str());

					Utils::CreateDir(destinationDirectory);
					std::ofstream destination(fullPath, std::ios::binary);

					int handle;
					Game::FS_FOpenFileRead(internalPath, &handle);

					if (handle != 0)
					{
						char buffer[1024];
						int bytesRead;

						while ((bytesRead = Game::FS_Read(buffer, sizeof(buffer), handle)) > 0)
						{
							destination.write(buffer, bytesRead);
						}

						destination.flush();
						destination.close();

						Game::FS_FCloseFile(handle);
					}
					break;
				}

				// I DON'T KNOW :(
				default:
					Components::Logger::Print("Error dumping sound alias %s: unknown format %d\n", iw3Alias.aliasName, iw3Alias.soundFile->type);
					return;
				}
			}
			else
			{
				Components::Logger::Print("Error dumping sound alias %s: NULL soundfile!\n", iw3Alias.aliasName);
				return;
			}

			// Convert flags
			IW3SoundAliasFlags iw3Flags;
			iw3Flags.intValue = iw3Alias.flags;

			IW4SoundAliasFlags outputFlags;
			outputFlags.intValue = 0;
			outputFlags.looping = iw3Flags.looping;
			outputFlags.isMaster = iw3Flags.isMaster;
			outputFlags.isSlave = iw3Flags.isSlave;
			outputFlags.fullDryLevel = iw3Flags.fullDryLevel;
			outputFlags.noWetLevel = iw3Flags.noWetLevel;
			outputFlags.type = iw3Flags.type;

			auto channel = iw3Flags.channel;

			// Channel conversion
			channel = Isnd_alias_list_t::channelLookupTable[channel];

			outputFlags.channel = channel;

			auto iw4Flags = outputFlags.intValue;

			rapidjson::Value alias(rapidjson::kObjectType);
			alias.AddMember("aliasName", RAPIDJSON_STR(iw3Alias.aliasName), allocator);
			alias.AddMember("centerPercentage",iw3Alias.centerPercentage, allocator);
			alias.AddMember("chainAliasName", RAPIDJSON_STR(iw3Alias.chainAliasName), allocator);
			alias.AddMember("distMax", iw3Alias.distMax, allocator);
			alias.AddMember("distMin", iw3Alias.distMin, allocator);
			alias.AddMember("envelopMax", iw3Alias.envelopMax, allocator);
			alias.AddMember("envelopMin", iw3Alias.envelopMin, allocator);
			alias.AddMember("envelopPercentage", iw3Alias.envelopPercentage, allocator);
			alias.AddMember("flags", iw4Flags, allocator);
			alias.AddMember("lfePercentage", iw3Alias.lfePercentage, allocator);
			alias.AddMember("mixerGroup", rapidjson::Value(rapidjson::kNullType), allocator);
			alias.AddMember("pitchMax", iw3Alias.pitchMax, allocator);
			alias.AddMember("pitchMin", iw3Alias.pitchMin, allocator);
			alias.AddMember("probability", iw3Alias.probability, allocator);
			alias.AddMember("secondaryAliasName", RAPIDJSON_STR(iw3Alias.secondaryAliasName), allocator);
			alias.AddMember("sequence", iw3Alias.sequence, allocator);
			alias.AddMember("slavePercentage", iw3Alias.slavePercentage, allocator);
			alias.AddMember("speakerMap",speakerMap, allocator);
			alias.AddMember("soundFile", RAPIDJSON_STR(strDuplicator.duplicateString(soundFile)), allocator);
			alias.AddMember("startDelay", iw3Alias.startDelay, allocator);
			alias.AddMember("subtitle", RAPIDJSON_STR(iw3Alias.subtitle), allocator);
			alias.AddMember("type", iw3Alias.soundFile->type, allocator);
			alias.AddMember("volMax", iw3Alias.volMax, allocator);
			alias.AddMember("volMin", iw3Alias.volMin, allocator);
			alias.AddMember("volumeFalloffCurve", RAPIDJSON_STR(iw3Alias.volumeFalloffCurve->filename), allocator);

			head.PushBack(alias, allocator);
		}

		output.AddMember("aliasName", RAPIDJSON_STR(ents->aliasName), allocator);
		output.AddMember("count", ents->count, allocator);
		output.AddMember("head", head, allocator);

		rapidjson::StringBuffer buff;
		rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buff);
		output.Accept(writer);
		
		const auto& dump = buff.GetString();

		Utils::WriteFile(Utils::VA("%s/sounds/%s.json", AssetHandler::GetExportPath().data(), ents->aliasName), dump);
	}


	Isnd_alias_list_t::Isnd_alias_list_t()
	{
		Command::Add("dumpSound", [](Command::Params params)
			{
				if (params.Length() < 2) return;
				Isnd_alias_list_t::Dump(Game::DB_FindXAssetHeader(Game::XAssetType::ASSET_TYPE_SOUND, params[1]).sound);
			});
	}

	Isnd_alias_list_t::~Isnd_alias_list_t()
	{
	}
}