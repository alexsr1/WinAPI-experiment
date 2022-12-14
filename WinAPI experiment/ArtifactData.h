#pragma once
//#include <Windows.h>

#include <string>
#include <unordered_map>

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
		std::string value;
		ISubstat() : key(""), value("") {}
	};

	using rects::TextBox;
	constexpr TextBox mainStatKeyBox{ 1111, 224, 200, 24 };
	constexpr TextBox mainStatValueBox{ 1111, 252, 200, 33 };
	constexpr TextBox slotKeyBox{ 1111, 159, 192, 18 };
	constexpr TextBox levelBox{ 1116, 359, 44, 20 };
	constexpr TextBox substatBox{ 1133, 401, 303, 22 };
	constexpr TextBox setKeyBox{ 1112, 402, 320, 23 };


	SetKey setKey(tesseract::TessBaseAPI* api, unsigned short substatNum);
	SlotKey slotKey(tesseract::TessBaseAPI* api);
	number level(tesseract::TessBaseAPI* api);
	number rarity(const void* pixelData, size_t width);
	StatKey mainStatKey(tesseract::TessBaseAPI* api);
	boolean lock(const void* pixelData, size_t width);
	unsigned short numOfSubstats(const void* pixelData, size_t width);
	ISubstat* substats(tesseract::TessBaseAPI* api, size_t num);
}
