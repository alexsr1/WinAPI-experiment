#include <Windows.h>
#include <iostream>
#include "bitmapcapture.h"

namespace bmp {
    HWND genshinWnd = NULL;

    BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
        const size_t maxLength = 128U;
        std::string title((long long)GetWindowTextLengthA(hWnd) + 1, 'a');

        if (!IsWindowVisible(hWnd)) return TRUE;

        UINT charsCopied = GetWindowTextA(hWnd, &title[0], maxLength);
        DWORD pId = 0;
        GetWindowThreadProcessId(hWnd, &pId);

        if (charsCopied && title.find("Genshin Impact") != std::string::npos) {
#ifdef _DEBUG
            std::cout << title << ": " << pId << "\n";
#endif // DEBUG
            genshinWnd = hWnd;
        }
        return TRUE;
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

    bool getBmpData(HWND windowOpt, BITMAPINFOHEADER& bmfhOut, void*& pixelsOut, SIZE_T& dwBmpSizeOut) {
        int x1, y1, cx, cy;

        // get screen dimensions
        if (!windowOpt) {
            x1 = GetSystemMetrics(SM_XVIRTUALSCREEN);
            y1 = GetSystemMetrics(SM_YVIRTUALSCREEN);
            cx = GetSystemMetrics(SM_CXSCREEN);
            cy = GetSystemMetrics(SM_CYSCREEN);

        }
        else {
            RECT clientArea;
            GetClientRect(windowOpt, &clientArea);
            x1 = clientArea.left;
            y1 = clientArea.top;
            cx = clientArea.right - clientArea.left;
            cy = clientArea.bottom - clientArea.top;
        }

        // copy screen to bitmap
        HDC     hScreen = GetDC(windowOpt);
        HDC     hDC = CreateCompatibleDC(hScreen);
        HBITMAP hBitmap = CreateCompatibleBitmap(hScreen, cx, cy);
        HGDIOBJ old_obj = SelectObject(hDC, hBitmap);
        BOOL    bRet = BitBlt(hDC, 0, 0, cx, cy, hScreen, x1, y1, SRCCOPY | CAPTUREBLT);
        BITMAP  bmpScreen;
        GetObject(hBitmap, sizeof(BITMAP), &bmpScreen);

        bmfhOut.biSize = sizeof(BITMAPINFOHEADER);
        bmfhOut.biWidth = bmpScreen.bmWidth;
        bmfhOut.biHeight = -1 * bmpScreen.bmHeight;
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
            bmpScreen.bmHeight,
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
	void drawRect(const rects::TextBox& rect, RGBQUAD color, BITMAPINFOHEADER& dataInfo, RGBQUAD* data) {
		for (size_t i = rect.posX; i <= rect.posX + rect.width; i++)
		{
			data[rect.posY * dataInfo.biWidth + i] = color;
		}
		for (size_t i = rect.posY; i <= rect.posY + rect.height; i++)
		{
			data[i * dataInfo.biWidth + rect.posX] = color;
		}
	}
}