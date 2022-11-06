#include <Windows.h>
#include <iostream>
#include <cmath>

inline UINT lerp(UINT a, UINT b, float t) {
    return (UINT)std::round(a + (b - a) * t);
}

inline void printErr(DWORD err, const char errSrc[]) {
    std::cout << errSrc <<  " failed, code: " << err << std::endl;
}

inline DWORD pressStringKeys(const char keys[]) {
    INPUT inputs[2] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_KEYBOARD;

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.dwFlags = KEYEVENTF_KEYUP;

    while (*keys != '\0') {
        WORD key = (*keys >= 'a' ? *keys - 32 : *keys);
        inputs[0].ki.wVk = key;
        inputs[1].ki.wVk = key;

        UINT uSent = SendInput(ARRAYSIZE(inputs), inputs, sizeof(INPUT));
        if (uSent != ARRAYSIZE(inputs))
        {  
            return GetLastError();
        }
        keys++;
    }
    return 0;
}

HWND genshinWnd = NULL;

BOOL CALLBACK enumWindowCallback(HWND hWnd, LPARAM lparam) {
    const size_t maxLength = 128U;
    std::string title((long long)GetWindowTextLengthA(hWnd) + 1, 'a');

    if (!IsWindowVisible(hWnd)) return TRUE;

    UINT charsCopied = GetWindowTextA(hWnd, &title[0], maxLength);
    DWORD pId = 0;
    GetWindowThreadProcessId(hWnd, &pId);

    if (charsCopied && title.find("Genshin Impact") != std::string::npos) {
        std::cout << title << ": " << pId << "\n";
        genshinWnd = hWnd;
    }
    return TRUE;
}

inline DWORD cursorClick(int x, int y) {
    const UINT cInputs = 2;
    INPUT inputs[cInputs] = {};
    ZeroMemory(inputs, sizeof(inputs));

    inputs[0].type = INPUT_MOUSE;
    inputs[0].mi.dx = x;
    inputs[0].mi.dy = y;
    inputs[0].mi.dwFlags = MOUSEEVENTF_LEFTDOWN;

    inputs[1].type = INPUT_MOUSE;
    inputs[1].mi.dx = x;
    inputs[1].mi.dy = y;
    inputs[1].mi.dwFlags = MOUSEEVENTF_LEFTUP;

    if (!SendInput(cInputs, inputs, sizeof(INPUT)))
        return GetLastError();
    return 0;
}

inline DWORD mouseWheel(int amount) {
    UINT numInputs = 1;
    INPUT input;
    POINT pos;
    GetCursorPos(&pos);

    ZeroMemory(&input, sizeof(input));
    int size = sizeof(INPUT);

    input.type = INPUT_MOUSE;
    input.mi.dwFlags = MOUSEEVENTF_WHEEL;
    input.mi.mouseData = amount;
    input.mi.dx = pos.x;
    input.mi.dy = pos.y;
    input.mi.dwExtraInfo = GetMessageExtraInfo();

    if (!SendInput(numInputs, &input, size))
        return GetLastError();
    return 0;
}

int main()
{
    //const char type[] = "HI this is a test im testing out this new shit";
    EnumWindows(enumWindowCallback, NULL);

    if (!genshinWnd) {
        std::cout << "Window not found" << std::endl;
        return EXIT_FAILURE;
    }

    SetForegroundWindow(genshinWnd);
    Sleep(10);
    for(size_t i = 0; i < 49; i++)
        mouseWheel(-1);

    if (!SetForegroundWindow(genshinWnd)) {
        printErr(GetLastError(), "SetForegroundWindow");
        return EXIT_FAILURE;
    }
        
    RECT rect;
    if (!GetWindowRect(genshinWnd, &rect)) {
        printErr(GetLastError(), "GetWindowRect");
        return EXIT_FAILURE;
    }
    std::cout << "Rect: \nx: " << rect.left << " y: " << rect.top << "\n";
    std::cout << "x: " << rect.right << " y: " << rect.bottom << "\n";

    //Origin
    const int xOrigin = lerp(rect.left, rect.right, 0.09464508094f);
    const int yOrigin = lerp(rect.top, rect.bottom, 0.18837459634f);

    const int xOffset = std::lround(143.f / 1900.f * (float)(rect.right - rect.left));
    const int yOffset = std::lround(149.f / 929.f * (float)(rect.bottom - rect.top));

    for (size_t i = 0; i < 5; i++) {
        for (size_t j = 0; j < 8; j++) {
            const int tempX = xOrigin + xOffset * j;
            const int tempY = yOrigin + yOffset * i;
            Sleep(100);

            if (!SetCursorPos(tempX, tempY)) {
                printErr(GetLastError(), "SetCursorPos");
                return EXIT_FAILURE;
            }
            cursorClick(tempX, tempY);
        }
        mouseWheel(-5);
    }

    /*while (1) {
        POINT mousePos;
        if (!GetCursorPos(&mousePos)) {
            printErr(GetLastError(), "GetCursorPos");
            return EXIT_FAILURE;
        }
        std::cout << "x: " << mousePos.x << " y: " << mousePos.y << std::endl;
    }*/

    return 0;
}
