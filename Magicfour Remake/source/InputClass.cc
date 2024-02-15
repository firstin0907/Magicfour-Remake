#include "../include/InputClass.hh"
#include "../include/GameException.hh"

#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)
#define WFILE WIDE(__FILE__)

#include <algorithm>


InputClass::InputClass(HINSTANCE hinstance,
    HWND hwnd, int screenWidth, int screenHeight)
{
    directInput_ = nullptr;
    keyboard_ = mouse_ = nullptr;

    m_keyboardState_curr = keyboardState_[0];
    m_keyboardState_prev = keyboardState_[1];

    HRESULT result;

    screen_width_ = screenWidth;
    screen_height_ = screenHeight;

    mouseX_ = mouseY_ = 0;

    result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION,
        IID_IDirectInput8, (void**)&directInput_, NULL);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);

    // keyboard
    result = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);
    result = keyboard_->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);

    // set cooperative level
    // --> exclusive: input is only used for this application.
    // --> non-exclusive: input can be widely used for any application.
    result = keyboard_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);

    result = keyboard_->Acquire();
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);


    // mouse
    result = directInput_->CreateDevice(GUID_SysMouse, &mouse_, NULL);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);
    result = mouse_->SetDataFormat(&c_dfDIMouse);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);

    // set cooperative level
    // --> exclusive: input is only used for this application.
    // --> non-exclusive: input can be widely used for any application. (now)
    result = mouse_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);

    result = mouse_->Acquire();
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);
}

InputClass::~InputClass()
{
    // Release the mouse.
    if (mouse_)
    {
        mouse_->Unacquire();
        mouse_->Release();
        mouse_ = 0;
    }

    // Release the keyboard.
    if (keyboard_)
    {
        keyboard_->Unacquire();
        keyboard_->Release();
        keyboard_ = 0;
    }

    // Release the main interface to direct input.
    if (directInput_)
    {
        directInput_->Release();
        directInput_ = 0;
    }

    return;
}

bool InputClass::Frame()
{
    bool result;

    result = ReadKeyboard();
    if (!result) return false;

    result = ReadMouse();
    if (!result) return false;

    ProcessInput();

    return true;
}

bool InputClass::ReadKeyboard()
{
    HRESULT result;

    std::swap(m_keyboardState_curr, m_keyboardState_prev);

    result = keyboard_->GetDeviceState(sizeof(keyboardState_[0]), (LPVOID)m_keyboardState_curr);
    if (FAILED(result))
    {
        // If the keyboard lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
            keyboard_->Acquire();
        else return false;
    }
    return true;
}

bool InputClass::ReadMouse()
{
    HRESULT result;

    result = mouse_->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&mouseState_);
    if (FAILED(result))
    {
        // If the keyboard lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
            mouse_->Acquire();
        else return false;
    }
    return true;
}

void InputClass::ProcessInput()
{
    mouseX_ += mouseState_.lX;
    mouseY_ += mouseState_.lY;

    if (mouseX_ < 0) mouseX_ = 0;
    if (mouseY_ < 0) mouseY_ = 0;

    if (mouseX_ > screen_width_) mouseX_ = screen_width_;
    if (mouseY_ > screen_height_) mouseY_ = screen_height_;
}

bool InputClass::IsEscapePressed()
{
    return (m_keyboardState_curr[DIK_ESCAPE] & 0x80);
}

bool InputClass::IsKeyPressed(int keysym)
{
    return (m_keyboardState_curr[keysym] & 0x80);
}

bool InputClass::IsKeyDown(int keysym)
{
    return (m_keyboardState_curr[keysym] & 0x80) && !(m_keyboardState_prev[keysym] & 0x80);
}

void InputClass::GetMouseLocation(int& x, int& y)
{
    x = mouseX_, y = mouseY_;
}

bool InputClass::IsMousePressed()
{
    return mouseState_.rgbButtons[0] & 0x80;
}
