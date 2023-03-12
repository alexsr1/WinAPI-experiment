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

    inline static void setBox(tesseract::TessBaseAPI* api, rects::TextBox box) {
        api->SetRectangle(box.posX, box.posY, box.width, box.height);
    }

    inline static char* ocrBox(tesseract::TessBaseAPI* api, rects::TextBox box) {
        setBox(api, box);

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
        /*if (!api->SetVariable("tessedit_char_whitelist", "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ'")) {
            std::cout << "Dict failed\n";
            return "";
        }*/
        unsigned short yPos = 402 + substatNum * 32;
        using rects::TextBox;
        TextBox runtimeSetKeyBox = artifact::setKeyBox;

        using artifact::substatBoxYOffset;
        runtimeSetKeyBox.posY += substatNum * substatBoxYOffset;

        char* outText = ocrBox(api, runtimeSetKeyBox);
        /*if (!api->SetVariable("tessedit_char_whitelist", "")) {
            std::cout << "Dict failed\n";
            return "";
        }*/
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
        
        if (slotKeyOcr.empty()) return "";

        return slotKeyOcr.substr(1, slotKeyOcr.size() - 1); //substract one because of \n at the end

    }

    number rarity(const void* pixelData, size_t width) {
        size_t constexpr xDist = 28, xStartPos = 1124, yPos = 310;
        //R: 50, G: 204, B: 255
        RGBQUAD targetColor{ 255, 50, 204, 255 };
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
        Pix* statKeyProcessed = api->GetThresholdedImage();
        std::unique_ptr<char[]> statKeyBox(api->GetBoxText(0));

        pixWrite("Processed_Images\\statKey.tiff", statKeyProcessed, IFF_TIFF);

        std::ofstream boxFile;
        boxFile.open("Processed_Images\\statKey.box");
        boxFile << statKeyBox;
        boxFile.close();

        StatKey StatKeyStr(outText);

        StatKey res = ocrtextToStatKey(StatKeyStr, ocrMainStatValue(api));

        delete[] outText;
        pixDestroy(&statKeyProcessed);
        
        return res;
    }

    boolean lock(const void* pixelData, size_t width) {
        //117, 138, 255
        RGBQUAD targetColor{ 255, 117, 138, 255 };
        RGBQUAD* rgbData = (RGBQUAD*)pixelData;
        constexpr int xPos = 1457, yPos = 373;
        RGBQUAD currentColor = rgbData[yPos * width + xPos];

        if (currentColor == targetColor)
            return "true";
        else
            return "false";
    }

    unsigned short numOfSubstats(const void* pixelData, const size_t width) {
        unsigned int substatPointX = 1123, substatPointY = 410;
        constexpr int substatDistanceY = 32, textBoxDistanceX = 10, textBoxDistanceY = -8;
        unsigned short substatCount = 0;
        RGBQUAD targetColor{ 255, 102, 83, 73 };
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
            //api->SetVariable("tessedit_char_whitelist", " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ+.%0123456789");
            //api->SetVariable("user_patterns_suffix", "\\*+\\d\\d.\\d%\n\\*+\\d\\d\n\\*+\\d\\d\\d\n\\*+\\d\\d\\d\\d\n\\*+\\d.\\d%");
            char* outText = ocrBox(api, currentSubstatBox);
            //api->SetVariable("tessedit_char_whitelist", "");
            //api->SetVariable("user_patterns_suffix", "");
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

    inline void printArtifactData(tesseract::TessBaseAPI* api, Pix* genshinPix) {
        IArtifact artifactOnScreen = getArtifactData(api, genshinPix);
        printArtifact(artifactOnScreen);
    }


    void printArtifact(const IArtifact& artifact) {
        std::cout << "setkey: " << artifact.setkey << std::endl;
        std::cout << "slotkey: " << artifact.slotkey << std::endl;
        std::cout << "level: " << artifact.level << std::endl;
        std::cout << "rarity: " << artifact.rarity << std::endl;
        std::cout << "mainStatKey: " << artifact.mainStatKey << std::endl;
        std::cout << "location: " << artifact.location << std::endl;
        std::cout << "lock: " << artifact.lock << std::endl;
        std::cout << "substats: " << std::endl;
        for (const ISubstat& stat : artifact.substats) {
            std::cout << stat << std::endl;
        }
        std::cout << std::endl;
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

    void printBoxes(tesseract::ResultIterator* ri, tesseract::PageIteratorLevel level) {
        if (ri != 0) {
            do {
                const char* symbol = ri->GetUTF8Text(level);
                float conf = ri->Confidence(level);
                int x1, y1, x2, y2;
                ri->BoundingBox(level, &x1, &y1, &x2, &y2);

                x2 -= mainStatKeyBox.posX;
                y2 -= mainStatKeyBox.posY;
                x1 -= mainStatKeyBox.posX;
                y1 -= mainStatKeyBox.posY;

                std::cout << symbol << ' ' << x1 << ' ' << y1 << ' ' << x2 << ' ' << x2 << " 0\n";

                delete[] symbol;
            } while (ri->Next(level));
        }
    }

    bool generateTrainingImages(tesseract::TessBaseAPI* api, Pix* genshinPix) {
        std::ifstream trainingImgsNum;
        static uint32_t trainingImgIdx = 0;
        trainingImgsNum.open("trainingImgsNum.txt");

        /*if (!trainingImgsNum.is_open()) {
            std::ofstream trainingImgsNumOut;
            trainingImgsNumOut.open("trainingImgsNum.txt");
            if (trainingImgsNumOut.fail()) {
#ifdef _DEBUG
                std::cout << "File open failed (generateTrainingImages)\n";
#endif
                return false;
            }

            trainingImgsNumOut << '0';
            trainingImgsNumOut.close();

            trainingImgIdx = 0;
        }
        else {
            std::string line;
            std::getline(trainingImgsNum, line);

            trainingImgIdx = std::stoul(line);
            trainingImgsNum.close();
        }*/

        for (size_t i = 0; i < boxesSize; i++)
        {
            setBox(api, boxes[i]);
            pixWrite(("Processed_Images\\" + std::string(boxStrings[i]) + std::to_string(trainingImgIdx) + ".png").c_str(), api->GetThresholdedImage(), IFF_PNG);
        } 

        TextBox runtimeSetBox = setKeyBox;
        runtimeSetBox.posY += substatBoxYOffset * numOfSubstats(pixGetData(genshinPix), pixGetWidth(genshinPix));

        setBox(api, runtimeSetBox);
        pixWrite(("Processed_Images\\runtimeSetBox" + std::to_string(trainingImgIdx) + ".png").c_str(), api->GetThresholdedImage(), IFF_PNG);

        trainingImgIdx++;
        return true;
    }

    void initializeGood(std::ofstream& outputFile) {
        outputFile << "{\"format\":\"GOOD\",\"version\": 2,\"source\":\"Genshin Scanner\",\"artifacts\":[";
    }

    void writeArtifact(std::ofstream& outputFile, IArtifact artifact) {
        outputFile <<
            R"({"setKey":")" << artifact.setkey << 
            R"(","slotKey":")" << artifact.slotkey <<
            R"(","level":)" << artifact.level << 
            R"(,"rarity":)" << artifact.rarity <<
            R"(,"mainStatKey":")" << artifact.mainStatKey << 
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