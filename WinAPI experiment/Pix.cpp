#include "Pix.h"

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
}
