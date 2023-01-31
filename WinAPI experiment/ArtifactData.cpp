#include <Windows.h>
#include <iostream>
#include <unordered_map>
#include <memory>
#include <algorithm>

#include <leptonica/allheaders.h>

#include "ArtifactData.h"
#include "ArtifactAttributeEnums.h"
#include "bitmapcapture.h"

namespace artifact {
    using rects::TextBox;

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

    //https://frzyc.github.io/genshin-optimizer/#/doc/ArtifactSetKey
    //NEEDS FIX
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
        char* outText = ocrBox(api, mainStatValueBox);
        std::string ocrResult(outText);
        strToLower(ocrResult);

        delete[] outText;
        return ocrResult;
    }

    inline static bool isPercentStat(std::string& text) {
        for (size_t i = text.size() - 1; i < text.size(); i--)
        {
            if (text[i] == '%' || text[i] == '/') return true;
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
        using rects::TextBox;
        TextBox runtimeSetKeyBox = artifact::setKeyBox;

        using artifact::substatBoxYOffset;
        runtimeSetKeyBox.posY += substatNum * substatBoxYOffset;

        char* outText = ocrBox(api, runtimeSetKeyBox);
        std::string slotKeyOcr(outText);
        strToLower(slotKeyOcr);
        std::string key = slotKeyOcr.substr(0, 4);

        auto it = SetKeyMap.find(key);
        if (it != SetKeyMap.end()) return (SetKey)it->second;

        if (key == "pray") {
            std::unordered_map<std::string, std::string> prayersSets {
                { "for destin", "PrayersForDestiny" },
                { "for illumi", "PrayersForIllumination" },
                { "for wisdom", "PrayersForWisdom" },
                { "to springt", "PrayersToSpringtime" }
            };

            it = prayersSets.find(slotKeyOcr.substr(8, 10));
            if (it != prayersSets.end()) return (SetKey)it->second;
        }

        if (key == "thun") {
            switch (slotKeyOcr[7]) {
            case 'i':
                return (SetKey)"ThunderingFury";
            case 's':
                return (SetKey)"Thundersoother";
            }
        }
        return "";
    }

    SlotKey slotKey(tesseract::TessBaseAPI* api) {

        char* outText = ocrBox(api, slotKeyBox);
        std::string slotKeyOcr(outText);
        strToLower(slotKeyOcr);

        delete[] outText;
        return getFirstWord(slotKeyOcr);
    }

    number level(tesseract::TessBaseAPI* api) {

        char* outText = ocrBox(api, levelBox);
        std::string slotKeyOcr(outText);
        size_t plusCharIdx = 0;
        for (; plusCharIdx < slotKeyOcr.size(); plusCharIdx++)
            if (slotKeyOcr[plusCharIdx] == '+')
                break;
        delete[] outText;
        return slotKeyOcr.substr(plusCharIdx + 1, slotKeyOcr.size()-1 - plusCharIdx - 1);
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

    StatKey ocrtextToStatKey(std::string StatKeyStr, std::string mainStatValue) {
        strToLower(StatKeyStr);
        std::string firstWord = getFirstWord(StatKeyStr);

        auto it = StatKeyMap.find(firstWord);
        if (it != StatKeyMap.end()) return (StatKey)it->second;

        //Crit DMG or Crit Rate
        if (StatKeyStr.substr(0, 4) == "crit") {
            switch (StatKeyStr[5]) {
            case 'r':
                return (StatKey)"critRate_";
            case 'd':
                return (StatKey)"critDMG_";
            }
        }

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
        return "";
    }

    StatKey mainStatKey(tesseract::TessBaseAPI* api) {

        api->SetRectangle(mainStatKeyBox.posX, mainStatKeyBox.posY,
            mainStatKeyBox.width, mainStatKeyBox.height);
        char* outText = api->GetUTF8Text();
        StatKey StatKeyStr(outText);

        StatKey res = ocrtextToStatKey(StatKeyStr, ocrMainStatValue(api));

        delete[] outText;
        if (res != "") return res;
        return "";
    }

    boolean lock(const void* pixelData, size_t width) {
        //117, 138, 255
        RGBQUAD targetColor{ 255, 255, 138, 117 };
        RGBQUAD* rgbData = (RGBQUAD*)pixelData;
        constexpr int xPos = 1457, yPos = 373;

        if (rgbData[yPos * width + xPos] == targetColor)
            return "true";
        else
            return "false";
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

    std::vector<ISubstat> substats(tesseract::TessBaseAPI* api, size_t num) {
        //TO DO: CRIT RATE AND CRIT DMG FIX
        TextBox currentSubstatBox = artifact::substatBox;
        std::vector<ISubstat> substatsBuffer;
        substatsBuffer.reserve(4);

        const size_t yOffset = 32;
        
        for (size_t i = 0; i < num; i++) {
            char* outText = ocrBox(api, currentSubstatBox);
            std::string substatStr(outText);

            size_t plusIdx = 0;
            for (; plusIdx < substatStr.size(); plusIdx++)
                if (substatStr[plusIdx] == '+') break;
            
            size_t percentIdx = 0;
            for (; percentIdx < substatStr.size(); percentIdx++)
                if (substatStr[percentIdx] == '%' || substatStr[percentIdx] == '/' || substatStr[percentIdx] == '\n')
                    break;

            std::string substatStatkey = substatStr.substr(0, plusIdx);
            std::string substatValue = substatStr.substr(plusIdx + 1, percentIdx - plusIdx);

            ISubstat temp;
            substatsBuffer.push_back(temp);
            substatsBuffer[i].key = ocrtextToStatKey(substatStatkey, substatValue);
            substatsBuffer[i].value = substatStr.substr(plusIdx + 1, percentIdx - (plusIdx + 1));

            currentSubstatBox.posY += yOffset;
            delete[] outText;
        }

        return substatsBuffer;
    }

    std::ostream& operator<<(std::ostream& out, ISubstat substat) {
        return out << substat.key << ": " << substat.value;
    }

    void printArtifactData(tesseract::TessBaseAPI* api, Pix* genshinPix) {
        void* pixelData = (void*)pixGetData(genshinPix);
        size_t width = pixGetWidth(genshinPix);

        std::string keys[] = { "setKey", "slotKey", "level", "rarity", "mainStatKey", "location", "lock" };
        unsigned short substatsNum = numOfSubstats(pixelData, width);
        std::string values[]{ setKey(api, substatsNum), slotKey(api), level(api), rarity(pixelData, width), mainStatKey(api), "", lock(pixelData, width) };
        std::vector<ISubstat> stats = substats(api, substatsNum);

        for (size_t i = 0; i < sizeof(keys) / sizeof(keys[0]); i++)
        {
            std::cout << keys[i] << ": " << values[i] << std::endl;
        }
        
        std::cout << "substats: \n";
        for (size_t i = 0; i < stats.size(); i++)
        {
            std::cout << stats[i] << std::endl;
        }
    }

    IArtifact getArtifactData(tesseract::TessBaseAPI* api, Pix* genshinPix) {
        IArtifact artifactData;

        void* pixelData = (void*)pixGetData(genshinPix);
        size_t width = pixGetWidth(genshinPix);

        unsigned short substatNum = numOfSubstats(pixelData, width);

        artifactData.setkey = setKey(api, substatNum);
        artifactData.slotkey = slotKey(api);
        artifactData.level = level(api);
        artifactData.rarity = rarity(pixelData, width);
        artifactData.mainStatKey = mainStatKey(api);
        artifactData.location = "";
        artifactData.lock = lock(pixelData, width);
        artifactData.substats = substats(api, substatNum);

        return artifactData;
    }

    void initializeGood(std::ofstream& outputFile) {
        outputFile << "{\"format\": \"GOOD\", \"version\": 2, \"source\": \"Genshin Scanner\", \"artifacts\": [";
    }

    void writeArtifact(std::ofstream& outputFile, IArtifact artifact) {
        outputFile <<
            R"({"setKey":")" << artifact.setkey << 
            R"(","slotKey":")" << artifact.slotkey <<
            R"(","level":")" << artifact.level << 
            R"(","rarity":")" << artifact.rarity <<
            R"(","mainStatKey":")" << artifact.mainStatKey << 
            R"(","location":")" << artifact.location <<
            R"(","lock":")" << artifact.lock << 
            R"(","substats":[)";

        for (size_t i = 0; i < artifact.substats.size(); i++)
        {
            outputFile << 
                R"({"key":")" << artifact.substats[i].key <<
                R"(","value":)" << artifact.substats[i].value << '}';
            if (i != artifact.substats.size() - 1) outputFile << ',';
        }

        outputFile << "]}";
    }
}