#include "WinapiMacros.h"

DWORD pressStringKeys(const char keys[]) {
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

DWORD cursorClick(int x, int y) {
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


DWORD mouseWheel(int amount) {
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
