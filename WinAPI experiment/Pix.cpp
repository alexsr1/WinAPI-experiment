#include "Pix.h"
#include "bitmapcapture.h"

namespace pix {
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

    void windowCapture(HWND window, Pix*& screenPix) {
        BITMAPINFOHEADER bmih;
        void* data;
        LONG bmpSize;

        bmp::getBmpData(bmp::genshinWnd, bmih, data, bmpSize);
        bmih.biHeight *= -1;
        screenPix = pix::bmpToPix(bmih, (l_uint32*)data, bmpSize);
    }
}
