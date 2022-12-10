#include <Windows.h>

#include <iostream>
#include <unordered_map>
#include <memory>

#include "ArtifactData.h"
#include "ArtifactAttributeEnums.h"
#include "bitmapcapture.h"

namespace artifact {
    using rects::TextBox;

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

    inline static std::string getFirstWord(const std::string& text) {
        std::string firstWord;
        firstWord.reserve(10);

        for (char c : text) {
            if (c == ' ') break;
            else firstWord += c;
        }

        return firstWord;
    }

    inline static char* ocrBox(tesseract::TessBaseAPI* api, rects::TextBox box) {
        api->SetRectangle(box.posX, box.posY,
            box.width, box.height);
        return api->GetUTF8Text();
    }

    inline static std::string ocrMainStatValue(tesseract::TessBaseAPI* api) {
        TextBox mainStatValueBox{ 1111, 252, 200, 33 };

        char* outText = ocrBox(api, mainStatValueBox);
        std::string ocrResult(outText);
        strToLower(ocrResult);

        delete[] outText;
        return ocrResult;
    }

    inline static bool isPercentStat(std::string text) {
        for (size_t i = text.size() - 1; i < text.size(); i--)
        {
            if (text[i] == '%') return true;
            if (text[i] >= '0' && text[i] <= '9') return false;
        }
        return false;
    }

    SlotKey slotKey(tesseract::TessBaseAPI* api) {
        TextBox slotKeyBox{ 1111, 159, 192, 18 };

        char* outText = ocrBox(api, slotKeyBox);
        std::string slotKeyOcr(outText);
        strToLower(slotKeyOcr);

        delete[] outText;
        return getFirstWord(slotKeyOcr);
    }

    number level(tesseract::TessBaseAPI* api) {
        TextBox levelBox{ 1116, 359, 44, 20 };

        char* outText = ocrBox(api, levelBox);
        std::string slotKeyOcr(outText);
        size_t plusCharIdx = (size_t)-1;
        for (size_t i = 0; i < slotKeyOcr.size(); i++)
        {
            if (slotKeyOcr[i] == '+')
                plusCharIdx = i;
        }

        delete[] outText;
        return slotKeyOcr.substr(0, plusCharIdx);
    }

    StatKey mainStatKey(tesseract::TessBaseAPI* api) {
        TextBox mainStatKeyBox{ 1111, 228, 200, 20 };
        RGBQUAD red{ 0, 0, 255, 0 };

        api->SetRectangle(mainStatKeyBox.posX, mainStatKeyBox.posY,
            mainStatKeyBox.width, mainStatKeyBox.height);
        char* outText = api->GetUTF8Text();
        StatKey StatKeyStr(outText);
        strToLower(StatKeyStr);

        std::string firstWord = getFirstWord(StatKeyStr);

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
        delete[] outText;
        return firstWord;
    }

    static inline bool operator==(RGBQUAD first, RGBQUAD second) {
        return
            first.rgbBlue == second.rgbBlue &&
            first.rgbGreen == second.rgbGreen &&
            first.rgbRed == second.rgbRed &&
            first.rgbReserved == second.rgbReserved;
    }
    size_t numOfSubstats(const void* pixelData, const size_t areaHeight, const size_t areaWidth) {
        unsigned int substatPointX = 1123, substatPointY = 410;
        constexpr int substatDistanceY = 32, textBoxDistanceX = 10, textBoxDistanceY = -8;
        size_t substatCount = 0;
        RGBQUAD targetColor{ 255, 73, 83, 102 };
        RGBQUAD* rgbData = (RGBQUAD*)pixelData;
        RGBQUAD currentPixel = rgbData[substatPointY * areaWidth + substatPointX];

        while (currentPixel == targetColor)
        {
            substatCount++;
            substatPointY += substatDistanceY;
            size_t nextIndex = substatPointY * areaWidth + substatPointX;
            currentPixel = rgbData[nextIndex];
        }

        return substatCount;
    }
}