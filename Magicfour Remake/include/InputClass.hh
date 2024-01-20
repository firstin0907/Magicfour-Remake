#pragma once

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#include <dinput.h>

class InputClass
{
public:
    InputClass(HINSTANCE hinstance,
        HWND hwnd, int screenWidth, int screenHeight);
    InputClass(const InputClass&) = delete;
    ~InputClass();

    bool Frame();

    bool IsEscapePressed();
    bool IsKeyPressed(int keysym);
    bool IsKeyDown(int keysym);
    void GetMouseLocation(int& x, int& y);
    bool IsMousePressed();

private:
    bool ReadKeyboard();
    bool ReadMouse();
    void ProcessInput();


private:
    IDirectInput8* m_directInput;
    IDirectInputDevice8* m_keyboard;
    IDirectInputDevice8* m_mouse;

    unsigned char m_keyboardState[2][256];
    unsigned char* m_keyboardState_curr;
    unsigned char* m_keyboardState_prev;
    DIMOUSESTATE m_mouseState;

    int m_screenWidth, m_screenHeight, m_mouseX, m_mouseY;

};