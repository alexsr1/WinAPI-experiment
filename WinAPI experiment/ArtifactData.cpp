#include <Windows.h>

#include <iostream>
#include <unordered_map>
#include <memory>
#include <algorithm>

#include "ArtifactData.h"
#include "ArtifactAttributeEnums.h"
#include "bitmapcapture.h"

namespace artifact {
    using rects::TextBox;

    inline static void strToLower(std::string& str) {
        std::transform(str.begin(), str.end(), str.begin(),
            [](unsigned char c) { return std::tolower(c); } 
        );
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

    inline static bool isPercentStat(std::string& text) {
        for (size_t i = text.size() - 1; i < text.size(); i--)
        {
            if (text[i] == '%') return true;
            if (text[i] >= '0' && text[i] <= '9') return false;
        }
        return false;
    }

    static inline bool operator==(RGBQUAD first, RGBQUAD second) {
        return
            first.rgbBlue == second.rgbBlue &&
            first.rgbGreen == second.rgbGreen &&
            first.rgbRed == second.rgbRed &&
            first.rgbReserved == second.rgbReserved;
    }

    static inline void lettersOnly(std::string& s) {
        for (auto it = s.begin(); it != s.end(); it++)
        {
            if (!((*it >= 'a' && *it <= 'z') || (*it >= 'A' && *it <= 'Z')))
                s.erase(it);
        }
    }

    SetKey setKey(tesseract::TessBaseAPI* api, unsigned short substatNum) {
        unsigned short yPos = 402 + substatNum * 32;
        TextBox slotKeyBox{ 1112, yPos, 320, 23 };

        char* outText = ocrBox(api, slotKeyBox);
        std::string slotKeyOcr(outText);
        lettersOnly(slotKeyOcr);

        return slotKeyOcr;
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

    number rarity(const void* pixelData, size_t width) {
        size_t constexpr xDist = 28, xStartPos = 1124, yPos = 310;
        //R: 50, G: 204, B: 255
        RGBQUAD targetColor{ 255, 255, 204, 50 };
        RGBQUAD* rgbData = (RGBQUAD*)pixelData;

        RGBQUAD currentColor = rgbData[yPos * width + xStartPos];
        LONG count = 0;
        while (currentColor == targetColor) {
            count++;
            size_t nextIndex = yPos * width + xStartPos + xDist * count;
            currentColor = rgbData[nextIndex];
        }

        return (number)std::to_string(count);
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
        if (it != StatKeyMap.end()) return (StatKey)it->second;

        //Crit DMG or Crit Rate
        if (StatKeyStr.substr(0, 4) == "crit") {
            switch (StatKeyStr[4]) {
            case 'r':
                return (StatKey)"critRate_";
            case 'd':
                return (StatKey)"critDMG_";
            }
        }

        std::string mainStatValue = ocrMainStatValue(api);
        bool isPercent = isPercentStat(mainStatValue);
		switch (firstWord[0]) {
		case 'h':
            if (isPercent) return (StatKey)"hp_";
            return (StatKey)"hp";
		case 'a':
            if (isPercent) return (StatKey)"atk_";
            return (StatKey)"atk";
		case 'd':
            if (isPercent) return (StatKey)"def_";
            return (StatKey)"def";
		}

        delete[] outText;
        return "";
    }

    unsigned short numOfSubstats(const void* pixelData, const size_t width) {
        unsigned int substatPointX = 1123, substatPointY = 410;
        constexpr int substatDistanceY = 32, textBoxDistanceX = 10, textBoxDistanceY = -8;
        unsigned short substatCount = 0;
        RGBQUAD targetColor{ 255, 73, 83, 102 };
        RGBQUAD* rgbData = (RGBQUAD*)pixelData;
        RGBQUAD currentPixel = rgbData[substatPointY * width + substatPointX];

        while (currentPixel == targetColor)
        {
            substatCount++;
            substatPointY += substatDistanceY;
            size_t nextIndex = substatPointY * width + substatPointX;
            currentPixel = rgbData[nextIndex];
        }

        return substatCount;
    }
}