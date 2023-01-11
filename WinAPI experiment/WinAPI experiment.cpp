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
#include "Pix.h"

inline long ilerp(UINT a, UINT b, float t) {
    return std::lround(a + (b - a) * t);
}

inline void printErr(DWORD err, const char errSrc[]) {
    std::cout << errSrc <<  " failed, code: " << err << std::endl;
}

std::ostream& operator<<(std::ostream& out, artifact::ISubstat substat) {
    return out << substat.key << ": " << substat.value;
}

int main()
{
//    EnumWindows(bmp::enumWindowCallback, NULL);
//
//    if (!bmp::genshinWnd) {
//        std::cout << "Genshin not found" << std::endl;
//        return EXIT_FAILURE;
//    }
//
//    if (!SetForegroundWindow(bmp::genshinWnd)) {
//        printErr(GetLastError(), "SetForegroundWindow");
//        return EXIT_FAILURE;
//    }
//
//    RECT rect;
//    if (!GetWindowRect(bmp::genshinWnd, &rect)) {
//        printErr(GetLastError(), "GetWindowRect");
//        return EXIT_FAILURE;
//    }
//
//    if (!IsWindowVisible(bmp::genshinWnd)) {
//        std::cout << "Window not visible\n";
//        return EXIT_FAILURE;
//    }
//#ifdef _DEBUG
//    std::cout << "Rect: \nx: " << rect.left << " y: " << rect.top << "\n";
//    std::cout << "x: " << rect.right << " y: " << rect.bottom << "\n";
//#endif
//    //char* outText;
//
//    BITMAPINFOHEADER bmih;
//    LONG dwBmpSize;
//
//    bmp::getBmpData(bmp::genshinWnd, bmih, data, dwBmpSize);
//    bmih.biHeight *= -1;
    //PIX* screenPix = bmpToPix(bmih, (l_uint32*)data, dwBmpSize);
    PIX* screenPix = pixRead("out.bmp");
    void* data = pixGetData(screenPix);
    l_int32 width = pixGetWidth(screenPix);

    if (!SetForegroundWindow(bmp::genshinWnd)) {
        printErr(GetLastError(), "SetForegroundWindow");
        return EXIT_FAILURE;
    }

    RECT rect;
    if (!GetWindowRect(bmp::genshinWnd, &rect)) {
        printErr(GetLastError(), "GetWindowRect");
        return EXIT_FAILURE;
    }

    if (!IsWindowVisible(bmp::genshinWnd)) {
        std::cout << "Window not visible\n";
        return EXIT_FAILURE;
    }
#ifdef _DEBUG
    std::cout << "Rect: \nx: " << rect.left << " y: " << rect.top << "\n";
    std::cout << "x: " << rect.right << " y: " << rect.bottom << "\n";
#endif
  
    //PIX* drivePix = pixRead("rect.bmp");
    tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
    if (api->Init("tessdata", "eng")) {
        std::cerr << "Could not initialize tesseract.\n";
        return EXIT_FAILURE;
    }
    api->SetImage(screenPix);
    //void* data = pixGetData(drivePix);
    //artifact::printArtifactData(api, data, bmih.biWidth);

    pixWrite("rect.bmp", screenPix, IFF_BMP);
    //const char type[] = "HI this is a test im testing out this new shit";
    cursorClick(3, 4);

    //Origin
    const int xOrigin = ilerp(rect.left, rect.right, 0.09464508094f);
    const int yOrigin = ilerp(rect.top, rect.bottom, 0.18837459634f);

    const int xOffset = std::lround(143.f / 1900.f * (float)(rect.right - rect.left));
    const int yOffset = std::lround(149.f / 929.f * (float)(rect.bottom - rect.top));


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
