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

	SetKey setKey(tesseract::TessBaseAPI* api, size_t substatNum);
	SlotKey slotKey(tesseract::TessBaseAPI* api);
	number level(tesseract::TessBaseAPI* api);
	number rarity(const void* pixelData, size_t areaHeight, size_t areaWidth);
	StatKey mainStatKey(tesseract::TessBaseAPI* api);
	boolean lock(const void* pixelData, size_t areaHeight, size_t areaWidth);
	size_t numOfSubstats(const void* pixelData, size_t areaHeight, size_t areaWidth);
	ISubstat* substats(tesseract::TessBaseAPI, size_t num);
}
