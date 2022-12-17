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


Pix* bmpToPix(const BITMAPINFOHEADER& info, l_uint32* pxlData, const LONG dataSize) {
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

std::ostream& operator<<(std::ostream& out, artifact::ISubstat substat) {
    return out << substat.key << ": " << substat.value;
}

int main()
{
    EnumWindows(bmp::enumWindowCallback, NULL);

    if (!bmp::genshinWnd) {
        std::cout << "Genshin not found" << std::endl;
        return EXIT_FAILURE;
    }

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
    LONG dwBmpSize;
    void* data;

    bmp::getBmpData(bmp::genshinWnd, bmih, data, dwBmpSize);
    bmih.biHeight *= -1;
    PIX* screenPix = bmpToPix(bmih, (l_uint32*)data, dwBmpSize);
    //PIX* drivePix = pixRead("rect.bmp");

    using rects::TextBox;
    TextBox runtimeSetKeyBox = artifact::setKeyBox;
    unsigned int substatsNum = artifact::numOfSubstats(data, bmih.biWidth);
    runtimeSetKeyBox.posY += substatsNum * 32;

    TextBox boxes[] = {
        artifact::mainStatKeyBox,
        artifact::mainStatValueBox,
        artifact::slotKeyBox,
        artifact::levelBox,
        runtimeSetKeyBox,
    };
    TextBox substats[4];
    constexpr size_t numOfTextboxes = sizeof(boxes) / sizeof(TextBox);
    std::string stats[numOfTextboxes] = {
        "Main stat type",
        "Main stat value",
        "Artifact type",
        "Level", 
        "Set",
    };

    for (size_t i = 0; i < sizeof(stats) / sizeof(stats[0]); i++)
    {
        using namespace rects;
        RGBQUAD red{ 0, 0, 255, 0 };
        bmp::drawRect(boxes[i], red, bmih.biWidth, (RGBQUAD*)data);
    }
    pixWrite("rect.bmp", screenPix, IFF_BMP);
    //PIX* drivePix = pixRead("rect.bmp");
    tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
    if (api->Init("tessdata", "eng")) {
        std::cerr << "Could not initialize tesseract.\n";
        return EXIT_FAILURE;
    }
    api->SetImage(screenPix);
    //void* data = pixGetData(drivePix);
//#ifdef _DEBUG
    unsigned short substatNum = artifact::numOfSubstats(data, bmih.biWidth);
    std::cout << "substatCount: " << artifact::numOfSubstats(data, bmih.biWidth) << std::endl;
    std::cout << "mainStatKey: " << artifact::mainStatKey(api) << std::endl;
    std::cout << "rarity: " << artifact::rarity(data, bmih.biWidth) << std::endl;
    std::cout << "set: " << artifact::setKey(api, substatNum) << std::endl;
    std::cout << "lock: " << artifact::lock(data, bmih.biWidth) << std::endl;
    artifact::ISubstat* test = artifact::substats(api, substatNum);
    for (size_t i = 0; i < substatNum; i++)
    {
        std::cout << "Substat" << i << ": " << test[i] << '\n';
    }
//#endif
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
    
    pixDestroy(&screenPix);
    return 0;
}
