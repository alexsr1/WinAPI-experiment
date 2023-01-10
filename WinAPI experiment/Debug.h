#pragma once
#include <iostream>
#include <Windows.h>

inline long ilerp(UINT a, UINT b, float t);
inline void printErr(DWORD err, const char errSrc[]);
DWORD testScanner(HWND genshinWnd);