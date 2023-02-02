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

int main()
{
    EnumWindows(bmp::enumWindowCallback, NULL);

    if (!bmp::genshinWnd) {
        std::cout << "Genshin not found" << std::endl;
        return EXIT_FAILURE;
    }

    if (!SetForegroundWindow(bmp::genshinWnd)) {
        printErr(GetLastError(), "SetForegroundWindow");
        return EXIT_FAILURE;
    }

    RECT rect;
    if (!GetWindowRect(bmp::genshinWnd, &rect)) {
        printErr(GetLastError(), "GetWindowRect");
        return EXIT_FAILURE;
    }

    if (!IsWindowVisible(bmp::genshinWnd)) {
        std::cout << "Window not visible\n";
        return EXIT_FAILURE;
    }
#ifdef _DEBUG
    std::cout << "Rect: \nx: " << rect.left << " y: " << rect.top << "\n";
    std::cout << "x: " << rect.right << " y: " << rect.bottom << "\n";
#endif
    Pix* screenPix;
    pix::windowCapture(bmp::genshinWnd, screenPix);

    tesseract::TessBaseAPI* api = new tesseract::TessBaseAPI();
    if (api->Init("tessdata", "eng")) {
        std::cerr << "Could not initialize tesseract.\n";
        return EXIT_FAILURE;
    }
    api->SetVariable("tessedit_char_whitelist", "0123456789+%DMG");

    api->SetImage(screenPix);

    using artifact::IArtifact;

    drawOcrBoxes(screenPix, "rect.bmp");
    IArtifact test = artifact::getArtifactData(api, screenPix);
    std::ofstream out;
    out.open("artifact test.txt");
    artifact::initializeGood(out);

    testScanner(bmp::genshinWnd, api, [&out](IArtifact& fuck) {
        artifact::writeArtifact(out, fuck);
    out << ",";
        });
    out << "]}";
    out.close();
    //Cleanup
    api->End();
    delete api;
    //delete[] outText;
    
    pixDestroy(&screenPix);
    return 0;
}
