#include <Windows.h>

#include <iostream>
#include <cmath>
#include <algorithm>
#include <memory> 

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include "ArtifactAttributeEnums.h"
#include "bitmapcapture.h"
#include "TesseractFunctions.h"
#include "WinapiMacros.h"
#include "ArtifactData.h"

inline long ilerp(UINT a, UINT b, float t) {
    return std::lround(a + (b - a) * t);
}

inline void printErr(DWORD err, const char errSrc[]) {
    std::cout << errSrc <<  " failed, code: " << err << std::endl;
}


Pix* bmpToPix(const BITMAPINFOHEADER& info, l_uint32* pxlData, const size_t dataSize) {
    Pix* bmpPix = pixCreate(info.biWidth, info.biHeight, info.biBitCount);

    if ((bmpPix = pixCreateHeader(info.biWidth, info.biHeight, info.biBitCount)) == NULL)
        return nullptr;
    //Flip vertically
    /*for (size_t iTop = 0, iBottom = info.biHeight - 1; iTop < info.biHeight / 2; iTop++, iBottom--)
        for (size_t j = 0; j < info.biWidth; j++)
            std::swap(
                pxlData[iTop * info.biWidth + j],
                pxlData[iBottom * info.biWidth + j]
            );*/
    pixSetInputFormat(bmpPix, IFF_BMP);
	pixSetData(bmpPix, pxlData);
	pixSetPadBits(bmpPix, 0);

	bool readerror = 0;
    l_int32 fileBpl = dataSize / info.biHeight;
    l_int32 pixWpl = pixGetWpl(bmpPix);
	l_int32 extrabytes = fileBpl - 3 * info.biWidth;
	l_uint32* line = pixGetData(bmpPix) + pixWpl * (info.biHeight - 1);

	pixEndianByteSwap(bmpPix);

	return bmpPix;
}

int main()
{
    /*EnumWindows(bmp::enumWindowCallback, NULL);

    if (!bmp::genshinWnd) {
        std::cout << "Genshin not found" << std::endl;
        return EXIT_FAILURE;
    }

    for (size_t i = 0; i < 49; i++)
        mouseWheel(-1);

    if (!SetForegroundWindow(bmp::genshinWnd)) {
        printErr(GetLastError(), "SetForegroundWindow");
        return EXIT_FAILURE;
    }

    RECT rect;
    if (!GetWindowRect(bmp::genshinWnd, &rect)) {
        printErr(GetLastError(), "GetWindowRect");
        return EXIT_FAILURE;
    }
#ifdef _DEBUG
    std::cout << "Rect: \nx: " << rect.left << " y: " << rect.top << "\n";
    std::cout << "x: " << rect.right << " y: " << rect.bottom << "\n";
#endif
    char* outText;

    BITMAPINFOHEADER bmih;
    SIZE_T dwBmpSize;
    void* data;

    bmp::getBmpData(bmp::genshinWnd, bmih, data, dwBmpSize);
    bmih.biHeight *= -1;
    PIX* screenPix = bmpToPix(bmih, (l_uint32*)data, dwBmpSize);
    PIX* drivePix = pixRead("rect.bmp");

    using rects::TextBox;
    TextBox boxes[] = {
        TextBox{ 1111, 228, 200, 20 },
        TextBox{ 1111, 252, 200, 33 },
        TextBox{ 1111, 159, 192, 18 },
        TextBox{ 1116, 359, 44, 20 }
    };

    constexpr size_t numOfTextboxes = sizeof(boxes) / sizeof(TextBox);
    std::string stats[numOfTextboxes] = {
        "Main stat type",
        "Main stat value",
        "Artifact type",
        "Level"
    };

    for (size_t i = 0; i < sizeof(stats) / sizeof(stats[0]); i++)
    {
        using namespace rects;
        RGBQUAD red{ 0, 0, 255, 0 };
        bmp::drawRect(boxes[i], red, bmih, (RGBQUAD*)data);
    }*/
    PIX* drivePix = pixRead("rect.bmp");
    tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
    if (api->Init("tessdata", "eng")) {
        std::cerr << "Could not initialize tesseract.\n";
        return EXIT_FAILURE;
    }
    api->SetImage(drivePix);
    void* data = pixGetData(drivePix);
#ifdef _DEBUG
    std::cout << "substatCount: " << artifact::numOfSubstats(data, pixGetHeight(drivePix), pixGetWidth(drivePix)) << std::endl;
    std::cout << "mainStatKey: " << artifact::mainStatKey(api) << std::endl;
#endif
    //pixWrite("rect.bmp", screenPix, IFF_BMP);
    //const char type[] = "HI this is a test im testing out this new shit";
    

    //Origin
    /*const int xOrigin = ilerp(rect.left, rect.right, 0.09464508094f);
    const int yOrigin = ilerp(rect.top, rect.bottom, 0.18837459634f);

    const int xOffset = std::lround(143.f / 1900.f * (float)(rect.right - rect.left));
    const int yOffset = std::lround(149.f / 929.f * (float)(rect.bottom - rect.top));

    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 8; j++) {
            const int tempX = xOrigin + xOffset * j;
            const int tempY = yOrigin + yOffset * i;
            Sleep(100);

            if (!SetCursorPos(tempX, tempY)) {
                printErr(GetLastError(), "SetCursorPos");
                return EXIT_FAILURE;
            }
            cursorClick(tempX, tempY);
        }
        mouseWheel(-5);
    }*/

    /*while (1) {
        POINT mousePos; 
        if (!GetCursorPos(&mousePos)) {
            printErr(GetLastError(), "GetCursorPos");
            return EXIT_FAILURE;
        }
        ScreenToClient(genshinWnd, &mousePos);
        std::cout << "x: " << mousePos.x << " y: " << mousePos.y << std::endl;
    }*/
    //Cleanup
    api->End();
    delete api;
    //delete[] outText;
    
    pixDestroy(&drivePix);
    return 0;
}
