#include <Windows.h>

#include <iostream>
#include <cmath>
#include <algorithm>
#include <memory> 
#include <fstream>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include "ArtifactAttributeEnums.h"
#include "bitmapcapture.h"
#include "TesseractFunctions.h"
#include "WinapiMacros.h"
#include "ArtifactData.h"
#include "Pix.h"
#include "Debug.h"

inline long ilerp(UINT a, UINT b, float t) {
    return std::lround(a + (b - a) * t);
}

inline void printErr(DWORD err, const char errSrc[]) {
    std::cout << errSrc <<  " failed, code: " << err << std::endl;
}

std::ostream& operator<<(std::ostream& out, artifact::ISubstat substat) {
    return out << substat.key << ": " << substat.value;
}

int checkGenshinWnd(HWND& genshinWnd) {
    EnumWindows(bmp::enumWindowCallback, reinterpret_cast<LPARAM>(&genshinWnd));

    if (!genshinWnd) {
        std::cout << "Genshin not found" << std::endl;
        return EXIT_FAILURE;
    }

    if (!SetForegroundWindow(genshinWnd)) {
        printErr(GetLastError(), "SetForegroundWindow");
        return EXIT_FAILURE;
    }

    RECT rect;
    if (!GetWindowRect(genshinWnd, &rect)) {
        printErr(GetLastError(), "GetWindowRect");
        return EXIT_FAILURE;
    }

    if (!IsWindowVisible(genshinWnd)) {
        std::cout << "Window not visible\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int main()
{
    HWND genshinWnd;

    if (checkGenshinWnd(genshinWnd) == EXIT_FAILURE)
        return EXIT_FAILURE;

    Pix* screenPix;
    pix::windowCapture(genshinWnd, screenPix);

    tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
    if (api->Init("tessdata", "eng_HYWenHei", tesseract::OcrEngineMode::OEM_LSTM_ONLY)) {
        std::cerr << "Could not initialize tesseract.\n";
        return EXIT_FAILURE;
    }

    api->SetPageSegMode(tesseract::PSM_SINGLE_LINE);

    api->SetImage(screenPix);

    using artifact::IArtifact;

    drawOcrBoxes(screenPix, "rect.bmp");
    IArtifact test = artifact::getArtifactData(api, screenPix);
    std::ofstream out;
    out.open("artifact test.txt");
    artifact::initializeGood(out);

    //artifact::generateTrainingImages(api, screenPix);

    testScanner(genshinWnd, api, [&out](IArtifact& arti) {
        artifact::writeArtifact(out,arti);
        out << ",";
        }, true);
    out << "]}";
    out.close();
    //Cleanup
    api->End();
    delete api;
    //delete[] outText;
    pixDestroy(&screenPix);

    return 0;
}
