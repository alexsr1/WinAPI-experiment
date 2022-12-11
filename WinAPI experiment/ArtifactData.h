#pragma once
#include <Windows.h>

#include <string>

#include <tesseract/baseapi.h>
namespace artifact {
	using number = std::string;
	using SetKey = std::string;
	using SlotKey = std::string;
	using StatKey = std::string;
	using boolean = std::string;
	using ISubstat = std::string;

	SetKey setKey(tesseract::TessBaseAPI* api, unsigned short substatNum);
	SlotKey slotKey(tesseract::TessBaseAPI* api);
	number level(tesseract::TessBaseAPI* api);
	number rarity(const void* pixelData, size_t width);
	StatKey mainStatKey(tesseract::TessBaseAPI* api);
	boolean lock(const void* pixelData, size_t width);
	unsigned short numOfSubstats(const void* pixelData, size_t width);
	ISubstat* substats(tesseract::TessBaseAPI, size_t num);
}
