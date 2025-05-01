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
    IDirectInput8* directInput_;
    IDirectInputDevice8* keyboard_;
    IDirectInputDevice8* mouse_;

    unsigned char keyboardState_[2][256];
    unsigned char* m_keyboardState_curr;
    unsigned char* m_keyboardState_prev;
    DIMOUSESTATE mouseState_;

    int screen_width_, screen_height_, mouseX_, mouseY_;

};