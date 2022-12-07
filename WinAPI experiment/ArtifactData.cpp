#include <Windows.h>

#include <iostream>
#include <unordered_map>
#include <memory>

#include "ArtifactData.h"
#include "ArtifactAttributeEnums.h"
#include "bitmapcapture.h"

namespace artifact {
    using rects::TextBox;
    SlotKey slotKey(tesseract::TessBaseAPI* api) {
        TextBox slotKeyBox{ 1111, 159, 192, 18 };
        api->SetRectangle(slotKeyBox.posX, slotKeyBox.posY,
            slotKeyBox.width, slotKeyBox.height);
        
    }

    inline static void strToLower(std::string& str) {
        for (char& c : str)
            if (c >= 'A' && c <= 'Z') c += 32;
    }
    inline static void cstrToLower(char* cstr) {
        while (cstr) {
            if (*cstr >= 'A' && *cstr <= 'Z') *cstr += 32;
            cstr++;
        }
    }

    inline static std::string& ocrMainStatValue(tesseract::TessBaseAPI* api) {
        TextBox mainStatValueBox{ 1111, 252, 200, 33 };
        api->SetRectangle(mainStatValueBox.posX, mainStatValueBox.posY,
            mainStatValueBox.width, mainStatValueBox.height);

        char* outText = api->GetUTF8Text();
        std::string ocrResult(outText);
        strToLower(ocrResult);

        delete[] outText;
        return ocrResult;
    }
    inline static bool isPercentStat(std::string text) {
        for (size_t i = text.size() - 1; i >= 0; i--)
        {
            if (text[i] == '%') return true;
            if (text[i] >= '0' && text[i] <= '9') return false;
        }
        return false;
    }

	StatKey mainStatKey(tesseract::TessBaseAPI* api) {
        TextBox mainStatKeyBox{ 1111, 228, 200, 20 };
        RGBQUAD red{ 0, 0, 255, 0 };

        api->SetRectangle(mainStatKeyBox.posX, mainStatKeyBox.posY,
            mainStatKeyBox.width, mainStatKeyBox.height);
        char* outText = api->GetUTF8Text();
        StatKey StatKeyStr(outText);
        strToLower(StatKeyStr);

        std::string firstWord;
        firstWord.reserve(10);
        for (char c : StatKeyStr) {
            if (c == ' ') break;
            else firstWord += c;
        }

        //https://frzyc.github.io/genshin-optimizer/#/doc/StatKey
        std::unordered_map<std::string, const char*> StatKeyMap{
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
        auto it = StatKeyMap.find(firstWord);
        if (it != StatKeyMap.end()) return it->second;

        //Crit DMG or Crit Rate
        if (firstWord == "crit") {
            std::string secondWord = StatKeyStr.substr(sizeof("crit"));
            if (secondWord == "dmg") return std::string("critDMG");
            else return std::string("critRate");
        }

        if (isPercentStat(ocrMainStatValue(api))) {
            switch (firstWord[0]) {
            case 'h':
                return std::string("hp_");
            case 'a':
                return std::string("atk_");
            case 'd':
                return std::string("def_");
           }
        }
#ifdef _DEBUG
        std::cout << "mainStatKey" << ": " << StatKeyStr;
#endif
        delete[] outText;
        return firstWord;
	}
    size_t numOfSubstats(const void* pixelData, const size_t areaHeight, const size_t areaWidth) {
        unsigned int substatPointX = 1123, substatPointY = 410;
        constexpr int substatDistanceY = 32, textBoxDistanceX = 10, textBoxDistanceY = -8;
        size_t substatCount = 0;
        RGBQUAD targetColor{ 255, 73, 83, 102 };
        RGBQUAD* rgbData = (RGBQUAD*)pixelData;
        RGBQUAD currentPixel = rgbData[substatPointY * areaWidth + substatPointX];

        while (
            currentPixel.rgbReserved == targetColor.rgbReserved &&
            currentPixel.rgbGreen == targetColor.rgbGreen &&
            currentPixel.rgbRed == targetColor.rgbRed
            )
        {
            substatCount++;
            substatPointY += substatDistanceY;
            size_t nextIndex = substatPointY * areaWidth + substatPointX;
            currentPixel = rgbData[nextIndex];
        }

        return substatCount;
    }
}