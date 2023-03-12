#pragma once
//#include <Windows.h>
#include <array>
#include <string>
#include <unordered_map>
#include <vector>
#include <fstream>

#include <tesseract/baseapi.h>

#include "ArtifactAttributeEnums.h"

namespace artifact {
	using number = std::string;
	using SetKey = std::string;
	using SlotKey = std::string;
	using StatKey = std::string;
	using boolean = std::string;
	struct ISubstat {
		StatKey key;
		number value;
		ISubstat() : key(""), value("") {}
	};

	struct IArtifact {
		SetKey setkey;
		SlotKey slotkey;
		number level;
		number rarity;
		StatKey mainStatKey;
		std::string location = "";
		boolean lock;
		std::vector<ISubstat> substats;
	};

	struct GOOD {
		std::string format = "GOOD";
		number version = "2";
		std::string source = "Genshin Scanner";
		std::vector<IArtifact>& artifacts;
	};

	using rects::TextBox;
	constexpr TextBox mainStatKeyBox{ 1111, 224, 200, 24 };
	constexpr TextBox mainStatValueBox{ 1111, 252, 200, 33 };
	constexpr TextBox slotKeyBox{ 1111, 159, 192, 18 };
	constexpr TextBox levelBox{ 1116, 359, 44, 20 };
	constexpr TextBox substatBox{ 1131, 400, 303, 25 };

	constexpr TextBox setKeyBox{ 1112, 400, 320, 25 };
	constexpr int substatBoxYOffset = 32;

	constexpr TextBox boxes[] = { mainStatKeyBox, mainStatValueBox, slotKeyBox, levelBox, substatBox };
	constexpr const char* boxStrings[] = { "mainStatKeyBox", "mainStatValueBox", "slotKeyBox", "levelBox", "substatBox" };
	constexpr size_t boxesSize = sizeof(boxes) / sizeof(boxes[0]);

	//https://frzyc.github.io/genshin-optimizer/#/doc/ArtifactSetKey
	std::unordered_map<std::string, std::string> SetKeyMap{
		{ "adve", "Adventurer" },
		{ "arch", "ArchaicPetra" },
		{ "bers", "Berserker" },
		{ "bliz", "BlizzardStrayer" },
		{ "bloo", "BloodstainedChivalry" },
		{ "brav", "BraveHeart" },
		{ "crim", "CrimsonWitchOfFlames" },
		{ "deep", "DeepwoodMemories" },
		{ "defe", "DefendersWill" },
		{ "dese", "DesertPavilionChronicle" },
		{ "echo", "EchoesOfAnOffering" },
		{ "embl", "EmblemOfSeveredFate" },
		{ "flow", "FlowerOfParadiseLost" },
		{ "gamb", "Gambler" },
		{ "gild", "GildedDreams" },
		{ "glad", "GladiatorsFinale" },
		{ "hear", "HeartOfDepth" },
		{ "husk", "HuskOfOpulentDreams" },
		{ "inst", "Instructor" },
		{ "lava", "Lavawalker" },
		{ "luck", "LuckyDog" },
		{ "maid", "MaidenBeloved" },
		{ "mart", "MartialArtist" },
		{ "nobl", "NoblesseOblige" },
		{ "ocea", "OceanHuedClam" },
		{ "pale", "PaleFlame" },
		//{ "pray", "PrayersForDestiny" },
		//{ "pray", "PrayersForIllumination" },
		//{ "pray", "PrayersForWisdom" },
		//{ "pray", "PrayersToSpringtime" },
		{ "reso", "ResolutionOfSojourner" },
		{ "retr", "RetracingBolide" },
		{ "scho", "Scholar" },
		{ "shim", "ShimenawasReminiscence" },
		{ "tena", "TenacityOfTheMillelith" },
		{ "thee", "TheExile" },
		//{ "thun", "ThunderingFury" },
		//{ "thun", "Thundersoother" },
		{ "tiny", "TinyMiracle" },
		{ "trav", "TravelingDoctor" },
		{ "verm", "VermillionHereafter" },
		{ "viri", "ViridescentVenerer" },
		{ "wand", "WanderersTroupe" }
	};

	//https://frzyc.github.io/genshin-optimizer/#/doc/StatKey
	std::unordered_map<std::string, std::string> StatKeyMap{
		{"elemental", "eleMas"},
		{"energy", "enerRech_"},
		{"healing", "heal_"},
		{"physical", "physical_dmg_"},
		{"anemo", "anemo_dmg_"},
		{"geo", "geo_dmg_"},
		{"electro", "electro_dmg_"},
		{"hydro", "hydro_dmg_"},
		{"pyro", "pyro_dmg_"},
		{"cryo", "cryo_dmg_"},
		{"dendro", "dendro_dmg_"}
	};

	SetKey setKey(tesseract::TessBaseAPI* api, unsigned short substatNum);
	SlotKey slotKey(tesseract::TessBaseAPI* api);
	number level(tesseract::TessBaseAPI* api);
	number rarity(const void* pixelData, size_t width);
	StatKey mainStatKey(tesseract::TessBaseAPI* api);
	boolean lock(const void* pixelData, size_t width);
	unsigned short numOfSubstats(const void* pixelData, size_t width);
	std::vector<ISubstat> substats(tesseract::TessBaseAPI* api, size_t num);

	void printArtifactData(tesseract::TessBaseAPI* api, Pix* genshinPix);
	void printArtifact(const IArtifact& artifact);
	IArtifact getArtifactData(tesseract::TessBaseAPI* api, Pix* genshinPix);
	bool generateTrainingImages(tesseract::TessBaseAPI* api, Pix* genshinPix);

	void writeArtifact(std::ofstream& outputFile, IArtifact artifact);

	void initializeGood(std::ofstream& outputFile);
}
