#include <iostream>

#include <tesseract/baseapi.h>
#include <leptonica/allheaders.h>

#include "TesseractFunctions.h"

void ocrFile(const char path[]) {
    tesseract::TessBaseAPI* Tess = new tesseract::TessBaseAPI();

    if (Tess->Init(NULL, "eng")) {
        std::cerr << "Could not initialize tesseract.\n";
        return;
    }

    // Open input image with leptonica library
    Pix* image = pixRead(path);
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