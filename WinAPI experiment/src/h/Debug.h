#pragma once
#include <iostream>
#include <functional>
#include <Windows.h>

#include "ArtifactData.h"

inline long ilerp(UINT a, UINT b, float t);
inline void printErr(DWORD err, const char errSrc[]);
DWORD testScanner(HWND genshinWnd, tesseract::TessBaseAPI* api, std::function<void(artifact::IArtifact&)> proc = nullptr, bool getArtifacts = false);
void drawOcrBoxes(Pix* screenPix, const char* path);