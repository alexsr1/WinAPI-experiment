#include <tesseract/baseapi.h>

#include "Debug.h"
#include "bitmapcapture.h"
#include "WinapiMacros.h"
#include "Pix.h"
#include "ArtifactData.h"

inline long ilerp(UINT a, UINT b, float t) {
    return std::lround(a + (b - a) * t);
}

inline void printErr(DWORD err, const char errSrc[]) {
    std::cout << errSrc << " failed, code: " << err << std::endl;
}


DWORD testScanner(HWND genshinWnd, tesseract::TessBaseAPI* api, std::function<void(artifact::IArtifact&)> proc) {
    RECT rect;
    if (!GetWindowRect(bmp::genshinWnd, &rect)) {
        printErr(GetLastError(), "GetWindowRect");
        return EXIT_FAILURE;
    }

    const int xOrigin = ilerp(rect.left, rect.right, 0.09464508094f);
    const int yOrigin = ilerp(rect.top, rect.bottom, 0.18837459634f);

    const int xOffset = std::lround(143.f / 1900.f * (float)(rect.right - rect.left));
    const int yOffset = std::lround(149.f / 929.f * (float)(rect.bottom - rect.top));

    std::cout << "TEST: \n";
    void* data;
    BITMAPINFOHEADER bmih;
    LONG dwBmpSize;
    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 8; j++) {
            const int tempX = xOrigin + xOffset * j;
            const int tempY = yOrigin + yOffset * i;

            if (!SetForegroundWindow(bmp::genshinWnd)) {
                printErr(GetLastError(), "SetForegroundWindow");
                return GetLastError();
            }

            if (!SetCursorPos(tempX, tempY)) {
                printErr(GetLastError(), "SetCursorPos");
                return GetLastError();
            }
            //cursorClick(tempX, tempY);

            Pix* screenPix;
            pix::windowCapture(genshinWnd, screenPix);
            api->SetImage(screenPix);

            artifact::IArtifact artifactData = artifact::getArtifactData(api, screenPix);
            proc(artifactData);

            pixDestroy(&screenPix);
            goto END;
        }
        //mouseWheel(-5);
    }
END:;
}

void drawOcrBoxes(Pix* screenPix, const char* path) {
    void* data = pixGetData(screenPix);
    long width = pixGetWidth(screenPix);

    using rects::TextBox;
    using artifact::substatBoxYOffset;

    TextBox runtimeSetKeyBox = artifact::setKeyBox;
    unsigned int substatsNum = artifact::numOfSubstats(data, width);
    runtimeSetKeyBox.posY += substatsNum * substatBoxYOffset;

    TextBox boxes[] = {
        artifact::mainStatKeyBox,
        artifact::mainStatValueBox,
        artifact::slotKeyBox,
        artifact::levelBox,
        artifact::slotKeyBox,
        runtimeSetKeyBox,
    };
    TextBox substats[4];
    constexpr size_t numOfTextboxes = sizeof(boxes) / sizeof(TextBox);
    std::string stats[numOfTextboxes] = {
        "Main stat type",
        "Main stat value",
        "Artifact type",
        "Level",
        "Slot",
        "Set",
    };

    for (size_t i = 0; i < sizeof(boxes) / sizeof(TextBox); i++)
    {
        using namespace rects;
        RGBQUAD red{ 0, 0, 255, 0 };
        bmp::drawRect(boxes[i], red, width, (RGBQUAD*)data);
    }

    TextBox runtimeSubstatBox = artifact::substatBox;
    for (size_t i = 0; i < substatsNum; i++)
    {
        using namespace rects;
        RGBQUAD red{ 0, 0, 255, 0 };
        bmp::drawRect(runtimeSubstatBox, red, width, (RGBQUAD*)data);

        runtimeSubstatBox.posY += artifact::substatBoxYOffset;
    }
    pixWrite(path, screenPix, IFF_BMP);
}