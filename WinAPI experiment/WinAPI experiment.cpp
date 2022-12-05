#include <Windows.h>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <memory> 
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>
#include "ArtifactAttributeEnums.h"
#include "bitmapcapture.h"

inline long ilerp(UINT a, UINT b, float t) {
    return std::lround(a + (b - a) * t);
}

inline void printErr(DWORD err, const char errSrc[]) {
    std::cout << errSrc <<  " failed, code: " << err << std::endl;
}

inline DWORD pressStringKeys(const char keys[]) {
    INPUT inputs[2] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_KEYBOARD;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    while (*keys != '\0') {
        WORD key = (*keys >= 'a' ? *keys - 32 : *keys);
        inputs[0].ki.wVk = key;
        inputs[1].ki.wVk = key;

        UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
        if (uSent != ARRAYSIZE(inputs))
        {  
            return GetLastError();
        }
        keys++;
    }
    return 0;
}

inline DWORD cursorClick(int x, int y) {
    const UINT cInputs = 2;
    INPUT inputs[cInputs] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dx = x;
    inputs[0].mi.dy = y;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dx = x;
    inputs[1].mi.dy = y;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    if (!SendInput(cInputs, inputs, sizeof(INPUT)))
        return GetLastError();
    return 0;
}

inline DWORD mouseWheel(int amount) {
    UINT numInputs = 1;
    INPUT input;
    POINT pos;
    GetCursorPos(&pos);

    ZeroMemory(&input, sizeof(input));
    int size = sizeof(INPUT);

    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    input.mi.mouseData = amount;
    input.mi.dx = pos.x;
    input.mi.dy = pos.y;
    input.mi.dwExtraInfo = GetMessageExtraInfo();

    if (!SendInput(numInputs, &input, size))
        return GetLastError();
    return 0;
}


void ocrFile(const char path[]) {
    tesseract::TessBaseAPI* Tess = new tesseract::TessBaseAPI();

    if (Tess->Init(NULL, "eng")) {
        std::cerr << "Could not initialize tesseract.\n";
        return;
    }

    // Open input image with leptonica library
    Pix* image = pixRead("C:\\Users\\user\\test.bmp");
    Tess->SetImage(image);

    // Get OCR result
    char* outText = Tess->GetUTF8Text();
    std::cout << "OCR output: " << outText << std::endl;

    //Cleanup
    Tess->End();
    delete Tess;
    delete[] outText;
    pixDestroy(&image);
}

Pix* bmpToPix(const BITMAPINFOHEADER& info, l_uint32* pxlData, const size_t dataSize) {
    Pix* bmpPix = pixCreate(info.biWidth, info.biHeight, info.biBitCount);

    if ((bmpPix = pixCreateHeader(info.biWidth, info.biHeight, info.biBitCount)) == NULL)
        return nullptr;
    /*
    * NOTE: This only aligns the bitmap pixel data to the Leptonica Pix storage standard.
    * It does not fix the colors and is only meant for text recognition.
    */
    //Flip vertically
    for (size_t iTop = 0, iBottom = info.biHeight - 1; iTop < info.biHeight / 2; iTop++, iBottom--)
        for (size_t j = 0; j < info.biWidth; j++)
            std::swap(
                pxlData[iTop * info.biWidth + j],
                pxlData[iBottom * info.biWidth + j]
            );
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

    if (!SetForegroundWindow(genshinWnd)) {
        printErr(GetLastError(), "SetForegroundWindow");
        return EXIT_FAILURE;
    }*/

    RECT rect;
    /*if (!GetWindowRect(genshinWnd, &rect)) {
        printErr(GetLastError(), "GetWindowRect");
        return EXIT_FAILURE;
    }
    std::cout << "Rect: \nx: " << rect.left << " y: " << rect.top << "\n";
    std::cout << "x: " << rect.right << " y: " << rect.bottom << "\n";*/

    tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
    char* outText;

    BITMAPINFOHEADER bmih;
    SIZE_T dwBmpSize;
    void* data;

    bmp::getBmpData(bmp::genshinWnd, bmih, data, dwBmpSize);
    PIX* screenPix = bmpToPix(bmih, (l_uint32*)data, dwBmpSize);
    pixWrite("heighttestout.bmp", screenPix, IFF_BMP);
    /*PIX* screenPix = bmpToPix(bmih, (l_uint32*)data, dwBmpSize);
    PIX* drivePix = pixRead("out.bmp");

    if (api->Init("tessdata", "eng")) {
        std::cerr << "Could not initialize tesseract.\n";
        return EXIT_FAILURE;
    }

    api->SetImage(drivePix);

    std::string stats[] = {
        "Main stat type",
        "Main stat value",
        "Artifact type",
        "Artifact set"
    };
    for (size_t i = 0; i < sizeof(stats) / sizeof(stats[0]); i++)
    {
        using namespace rects;
        api->SetRectangle(boxes[i]->posX, boxes[i]->posY, 
            boxes[i]->width, boxes[i]->height);
        std::cout << stats[i] << ": " << api->GetUTF8Text() << '\n';
    }*/
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
    /*api->End();
    delete api;
    delete[] outText;
    
    pixDestroy(&screenPix);*/
    return 0;
}
