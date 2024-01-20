#include "../include/InputClass.hh"
#include "../include/GameException.hh"

#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)
#define WFILE WIDE(__FILE__)

#include <algorithm>


InputClass::InputClass(HINSTANCE hinstance,
    HWND hwnd, int screenWidth, int screenHeight)
{
    m_directInput = nullptr;
    m_keyboard = m_mouse = nullptr;

    m_keyboardState_curr = m_keyboardState[0];
    m_keyboardState_prev = m_keyboardState[1];

    HRESULT result;

    m_screenWidth = screenWidth;
    m_screenHeight = screenHeight;

    m_mouseX = m_mouseY = 0;

    result = DirectInput8Create(hinstance, DIRECTINPUT_VERSION,
        IID_IDirectInput8, (void**)&m_directInput, NULL);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);

    // keyboard
    result = m_directInput->CreateDevice(GUID_SysKeyboard, &m_keyboard, NULL);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);
    result = m_keyboard->SetDataFormat(&c_dfDIKeyboard);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);

    // set cooperative level
    // --> exclusive: input is only used for this application.
    // --> non-exclusive: input can be widely used for any application.
    result = m_keyboard->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_EXCLUSIVE);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);

    result = m_keyboard->Acquire();
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);


    // mouse
    result = m_directInput->CreateDevice(GUID_SysMouse, &m_mouse, NULL);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);
    result = m_mouse->SetDataFormat(&c_dfDIMouse);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);

    // set cooperative level
    // --> exclusive: input is only used for this application.
    // --> non-exclusive: input can be widely used for any application. (now)
    result = m_mouse->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);

    result = m_mouse->Acquire();
    if (FAILED(result)) throw GameException(L"Failed to initialize InputClass", WFILE, __LINE__);
}

InputClass::~InputClass()
{
    // Release the mouse.
    if (m_mouse)
    {
        m_mouse->Unacquire();
        m_mouse->Release();
        m_mouse = 0;
    }

    // Release the keyboard.
    if (m_keyboard)
    {
        m_keyboard->Unacquire();
        m_keyboard->Release();
        m_keyboard = 0;
    }

    // Release the main interface to direct input.
    if (m_directInput)
    {
        m_directInput->Release();
        m_directInput = 0;
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

    result = m_keyboard->GetDeviceState(sizeof(m_keyboardState[0]), (LPVOID)m_keyboardState_curr);
    if (FAILED(result))
    {
        // If the keyboard lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
            m_keyboard->Acquire();
        else return false;
    }
    return true;
}

bool InputClass::ReadMouse()
{
    HRESULT result;

    result = m_mouse->GetDeviceState(sizeof(DIMOUSESTATE), (LPVOID)&m_mouseState);
    if (FAILED(result))
    {
        // If the keyboard lost focus or was not acquired then try to get control back.
        if ((result == DIERR_INPUTLOST) || (result == DIERR_NOTACQUIRED))
            m_mouse->Acquire();
        else return false;
    }
    return true;
}

void InputClass::ProcessInput()
{
    m_mouseX += m_mouseState.lX;
    m_mouseY += m_mouseState.lY;

    if (m_mouseX < 0) m_mouseX = 0;
    if (m_mouseY < 0) m_mouseY = 0;

    if (m_mouseX > m_screenWidth) m_mouseX = m_screenWidth;
    if (m_mouseY > m_screenHeight) m_mouseY = m_screenHeight;
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
    x = m_mouseX, y = m_mouseY;
}

bool InputClass::IsMousePressed()
{
    return m_mouseState.rgbButtons[0] & 0x80;
}
