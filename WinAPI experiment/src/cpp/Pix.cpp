#include "Pix.h"
#include "bitmapcapture.h"

namespace pix {
    struct Pixel {
        uint8_t byte0;
        uint8_t byte1;
        uint8_t byte2;
        uint8_t byte3;
    };

    void bmpBytesToPix(void* pixelData, unsigned int width, unsigned int height) {
        Pixel* pixels = (Pixel*)pixelData;

        for (size_t i = 0; i < height; i++) {
            for (size_t j = 0; j < width; j++)
            {
                Pixel& currentPixel = pixels[width * i + j];
                Pixel temp = currentPixel;
                
                //for some reason leptonica stores pixel bytes in the last three bytes (fixEndianByteSwap doesn't completely work)
                temp.byte0 = 255;
                temp.byte1 = currentPixel.byte0;
                temp.byte2 = currentPixel.byte1;
                temp.byte3 = currentPixel.byte2;

                currentPixel = temp;
            }
        }
    }

    Pix* bmpToPix(const BITMAPINFOHEADER& info, l_uint32* pxlData, const LONG dataSize) {
        //Pix* bmpPix = pixCreate(info.biWidth, info.biHeight, info.biBitCount);
        Pix* bmpPix;

        if ((bmpPix = pixCreateHeader(info.biWidth, info.biHeight, info.biBitCount)) == NULL)
            return nullptr;

        pixSetInputFormat(bmpPix, IFF_BMP);
        bmpBytesToPix(pxlData, info.biWidth, info.biHeight);
        pixSetData(bmpPix, pxlData);
        pixSetPadBits(bmpPix, 0);

        bool readerror = 0;
        l_int32 fileBpl = dataSize / info.biHeight;
        l_int32 pixWpl = pixGetWpl(bmpPix);
        l_int32 extrabytes = fileBpl - 3 * info.biWidth;
        l_uint32* line = pixGetData(bmpPix) + pixWpl * (info.biHeight - 1);

        //pixEndianByteSwap(bmpPix);

        return bmpPix;
    }

    void windowCapture(HWND window, Pix*& screenPix) {
        BITMAPINFOHEADER bmih;
        void* data;
        LONG bmpSize;

        bmp::getBmpData(window, bmih, data, bmpSize);
        bmih.biHeight *= -1;
        screenPix = pix::bmpToPix(bmih, (l_uint32*)data, bmpSize);
    }
}
