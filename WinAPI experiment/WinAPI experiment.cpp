#include <Windows.h>
#include <iostream>
#include <cmath>
#include <memory> 
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

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

HWND genshinWnd = NULL;

BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
    const size_t maxLength = 128U;
    std::string title((long long)GetWindowTextLengthA(hWnd) + 1, 'a');

    if (!IsWindowVisible(hWnd)) return TRUE;

    UINT charsCopied = GetWindowTextA(hWnd, &title[0], maxLength);
    DWORD pId = 0;
    GetWindowThreadProcessId(hWnd, &pId);

    if (charsCopied && title.find("Genshin Impact") != std::string::npos) {
        std::cout << title << ": " << pId << "\n";
        genshinWnd = hWnd;
    }
    return TRUE;
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

DWORD getScreenBitmap(BITMAP& outBitmap, BITMAPINFOHEADER& bi, BITMAPFILEHEADER& bmfHeader) {
    HDC screen = GetDC(NULL);
    HDC cScreen = CreateCompatibleDC(screen);

    int cx = GetSystemMetrics(SM_CXSCREEN);
    int cy = GetSystemMetrics(SM_CYSCREEN);
    int x = GetSystemMetrics(SM_XVIRTUALSCREEN); 
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);

    HBITMAP hbmScreen = CreateCompatibleBitmap(
        screen,
        cx,
        cy
    );
    if (!hbmScreen) {
        return GetLastError();
    }

    SelectObject(cScreen, hbmScreen);

    if (!BitBlt(screen,
        0, 0,
        cx, cy,
        cScreen,
        x, y,
        CAPTUREBLT | SRCCOPY)) {
        return GetLastError();
    }

    if (!GetObject(hbmScreen, sizeof(BITMAP), &outBitmap)) {
        return GetLastError();
    }

    bi.biSize = sizeof(BITMAPINFOHEADER);
    bi.biWidth = outBitmap.bmWidth;
    bi.biHeight = outBitmap.bmHeight;
    bi.biPlanes = 1;
    bi.biBitCount = 24;
    bi.biCompression = BI_RGB;
    bi.biSizeImage = 0;
    bi.biXPelsPerMeter = 0;
    bi.biYPelsPerMeter = 0;
    bi.biClrUsed = 0;
    bi.biClrImportant = 0;

    SIZE_T dwBmpSize = ((outBitmap.bmWidth * bi.biBitCount + 31) / 32) * 4 * outBitmap.bmHeight;
    
    char* lpvbitmap = (char*)HeapAlloc(GetProcessHeap(), HEAP_GENERATE_EXCEPTIONS, dwBmpSize);

    if (!GetDIBits(screen, hbmScreen, 0,
        (UINT)outBitmap.bmHeight,
        lpvbitmap,
        (BITMAPINFO*)&bi, DIB_RGB_COLORS)) {
        return GetLastError();
    }
    outBitmap.bmBits = lpvbitmap;

    // A file is created, this is where we will save the screen capture.
    HANDLE hFile = CreateFile(L"captureqwsx.bmp",
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    // Add the size of the headers to the size of the bitmap to get the total file size.
    SIZE_T dwSizeofDIB = dwBmpSize + sizeof(BITMAPFILEHEADER) + sizeof(BITMAPINFOHEADER);

    // Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

    // Size of the file.
    bmfHeader.bfSize = dwSizeofDIB;

    // bfType must always be BM for Bitmaps.
    bmfHeader.bfType = 0x4D42; // BM.

    DWORD dwBytesWritten;
    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpvbitmap, dwBmpSize, &dwBytesWritten, NULL);

    HeapFree(GetProcessHeap(), 0, lpvbitmap);
    ReleaseDC(NULL, screen);
    DeleteDC(cScreen);
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

bool getBmpData(HWND windowOpt, BITMAPINFOHEADER& bmfhOut, void*& pixelsOut, SIZE_T& dwBmpSizeOut) {
    int x1, y1, cx, cy;

    // get screen dimensions
    x1 = GetSystemMetrics(SM_XVIRTUALSCREEN);
    y1 = GetSystemMetrics(SM_YVIRTUALSCREEN);
    cx = GetSystemMetrics(SM_CXSCREEN);
    cy = GetSystemMetrics(SM_CYSCREEN);

    // copy screen to bitmap
    HDC     hScreen = GetDC(windowOpt);
    HDC     hDC = CreateCompatibleDC(hScreen);
    HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, cx, cy);
    HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
    BOOL    bRet = BitBlt(hDC, 0, 0, cx, cy, hScreen, 0, 0, SRCCOPY | CAPTUREBLT);
    BITMAP bmpScreen;
    GetObject(hBitmap, sizeof(BITMAP), &bmpScreen);

    bmfhOut.biSize = sizeof(BITMAPINFOHEADER);
    bmfhOut.biWidth = bmpScreen.bmWidth;
    bmfhOut.biHeight = bmpScreen.bmHeight;
    bmfhOut.biPlanes = 1;
    bmfhOut.biBitCount = 32;
    bmfhOut.biCompression = BI_RGB;
    bmfhOut.biSizeImage = 0;
    bmfhOut.biXPelsPerMeter = 0;
    bmfhOut.biYPelsPerMeter = 0;
    bmfhOut.biClrUsed = 0;
    bmfhOut.biClrImportant = 0;

    dwBmpSizeOut = ((bmpScreen.bmWidth * bmfhOut.biBitCount + 31) / 32) * 4 * bmpScreen.bmHeight;

    pixelsOut = (char*)malloc(dwBmpSizeOut);

    GetDIBits(hScreen, hBitmap, 0,
        (UINT)bmpScreen.bmHeight,
        pixelsOut,
        (BITMAPINFO*)&bmfhOut, DIB_RGB_COLORS);

    //cleanup
    SelectObject(hDC, old_obj);
    DeleteDC(hDC);
    ReleaseDC(NULL, hScreen);
    DeleteObject(hBitmap);

    return true;
}

void captureScreen(const char path[])
{
    BITMAPFILEHEADER   bmfHeader;
    BITMAPINFOHEADER   bi;
    void* lpbitmap;
    SIZE_T dwBmpSize;

    getBmpData(NULL, bi, lpbitmap, dwBmpSize);
    
    // Starting with 32-bit Windows, GlobalAlloc and LocalAlloc are implemented as wrapper functions that 
    // call HeapAlloc using a handle to the process's default heap. Therefore, GlobalAlloc and LocalAlloc 
    // have greater overhead than HeapAlloc.

    // Offset to where the actual bitmap bits start.
    bmfHeader.bfOffBits = (DWORD)sizeof(BITMAPFILEHEADER) + (DWORD)sizeof(BITMAPINFOHEADER);

    // Size of the file.
    bmfHeader.bfSize = dwBmpSize;

    // bfType must always be BM for Bitmaps.
    bmfHeader.bfType = 0x4D42; // BM.

    DWORD dwBytesWritten = 0;
    HANDLE hFile = hFile = CreateFileA(path,
        GENERIC_WRITE,
        0,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL, NULL);

    WriteFile(hFile, (LPSTR)&bmfHeader, sizeof(BITMAPFILEHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)&bi, sizeof(BITMAPINFOHEADER), &dwBytesWritten, NULL);
    WriteFile(hFile, (LPSTR)lpbitmap, dwBmpSize, &dwBytesWritten, NULL);

    // clean up
    free(lpbitmap);
}

Pix* bmpToPix(const BITMAPINFOHEADER& info, l_uint32* pxlData) {
    Pix* bmpPix = pixCreate(info.biWidth, info.biHeight, info.biBitCount);

    if ((bmpPix = pixCreateHeader(info.biWidth, info.biHeight, info.biBitCount)) == NULL)
        return nullptr;
    pixSetInputFormat(bmpPix, IFF_BMP);
	//l_int32 wpl = pixGetWpl(bmpPix);

	pixSetData(bmpPix, pxlData);
	pixSetPadBits(bmpPix, 0);
    pixEndianByteSwap(bmpPix);

	return bmpPix;
}

int main()
{
    tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
    char* outText;

    BITMAPINFOHEADER bmih;
    SIZE_T dwBmpSize;
    void* data;
    captureScreen("C:\\test.bmp");
    getBmpData(NULL, bmih, data, dwBmpSize);
    PIX* screenPix = bmpToPix(bmih, (l_uint32*)data);
    PIX* driveImg = pixRead("C:\\test.bmp");

    if (api->Init(NULL, "eng")) {
        std::cerr << "Could not initialize tesseract.\n";
        exit(1);
    }
    api->SetImage(screenPix);
    outText = api->GetUTF8Text();
    std::cout << "Ocr output: " << outText <<  std::endl;
    api->SetImage(driveImg);
    outText = api->GetUTF8Text();
    std::cout << "Ocr output: " << outText << std::endl;

    //const char type[] = "HI this is a test im testing out this new shit";
    /*EnumWindows(enumWindowCallback, NULL);

    if (!genshinWnd) {
        std::cout << "Window not found" << std::endl;
        return EXIT_FAILURE;
    }

    SetForegroundWindow(genshinWnd);
    Sleep(10);
    for(size_t i = 0; i < 49; i++)
        mouseWheel(-1);

    if (!SetForegroundWindow(genshinWnd)) {
        printErr(GetLastError(), "SetForegroundWindow");
        return EXIT_FAILURE;
    }
        
    RECT rect;
    if (!GetWindowRect(genshinWnd, &rect)) {
        printErr(GetLastError(), "GetWindowRect");
        return EXIT_FAILURE;
    }
    std::cout << "Rect: \nx: " << rect.left << " y: " << rect.top << "\n";
    std::cout << "x: " << rect.right << " y: " << rect.bottom << "\n";*/

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
        std::cout << "x: " << mousePos.x << " y: " << mousePos.y << std::endl;
    }*/
    //Cleanup
    api->End();
    delete api;
    delete[] outText;
    pixDestroy(&screenPix);
    return 0;
}
