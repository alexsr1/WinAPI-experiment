#include <Windows.h>

#include "ArtifactData.h"
#include "ArtifactAttributeEnums.h"
#include "bitmapcapture.h"

namespace artifact {
    using rects::TextBox;
	StatKey mainStatKey(tesseract::TessBaseAPI api) {
        TextBox mainStatKeyBox{ 1111, 228, 200, 20 };
        RGBQUAD red{ 0, 0, 255, 0 };
        api->SetRectangle(boxes[i].posX, boxes[i].posY,
            boxes[i].width, boxes[i].height);
        std::cout << stats[i] << ": " << api->GetUTF8Text();
	}
    size_t numOfSubstats(void* pixelData, size_t areaHeight, size_t areaWidth) {
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