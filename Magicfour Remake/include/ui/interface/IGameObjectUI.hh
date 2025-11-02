#pragma once

#include "util/ResourceMap.hh"

class IGameObjectUI
{
public:
	static void DrawUI(class D2DClass* direct2D,
		class ID3D11Device* device, int screen_width, int screen_height) {};
};