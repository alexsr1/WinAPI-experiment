#pragma once
#include <Windows.h>
namespace bmp {
	extern HWND genshinWnd;
	BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam);
	DWORD getScreenBitmap(BITMAP& outBitmap, BITMAPINFOHEADER& bi, BITMAPFILEHEADER& bmfHeader);
	bool getBmpData(HWND windowOpt, BITMAPINFOHEADER& bmfhOut, void*& pixelsOut, SIZE_T& dwBmpSizeOut);
	void captureScreen(const char path[]);
}

