#include <Windows.h>

#include <stdexcept>
#include <wrl.h>
#include <dxgi.h>
#include <d3dcommon.h>
#include <d3d11.h>
#include <DirectXMath.h>

#include "include/SystemClass.hh"
#include "include/GameException.hh"

#include <cstdio>

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR pScmdline, int iCmdshow)
{
	try
	{
		// Create the system object.
		unique_ptr<SystemClass> System = make_unique<SystemClass>();

		System->Run();
	}
	catch (...)
	{
		return -1;
	}
	return 0;
}