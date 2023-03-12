#pragma once
#include <Windows.h>
#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

namespace pix {
	Pix* bmpToPix(const BITMAPINFOHEADER& info, l_uint32* pxlData, const LONG dataSize);
	void windowCapture(HWND window, Pix*& screenPix);
}
