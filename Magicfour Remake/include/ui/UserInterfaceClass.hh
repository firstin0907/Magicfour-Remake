#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <d3d11.h>
#include <wrl.h>
#include <d2d1.h>
#include <dwrite.h>

#include <DirectXMath.h>

#include "util/ResourceMap.hh"
#include "ui/common/UIContext.hh"
#include "core/global.hh"

class UserInterfaceClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

	using XMMATRIX = DirectX::XMMATRIX;

public:
	UserInterfaceClass(class D2DClass* direct2D,
		ID3D11Device* device, int screen_width, int screen_height);
	~UserInterfaceClass();

	void CalculateScreenPos(const XMMATRIX& mvp_matrix,
		const XMMATRIX& ortho_inv, float& x, float& y) const;

	void CalculateScreenPos(const XMMATRIX& world_matrix, float& x, float& y) const;

	void Begin2dDraw(class D2DClass* direct2D, const XMMATRIX& vp_matrix, const XMMATRIX& ortho_matrix);
	void End2dDraw(class D2DClass* direct2D);

	void DrawMonsterUI(class D2DClass* direct2D, class GameObjectList& monsters, time_t curr_time);
	void DrawCharacterUI(class D2DClass* direct2D, class CharacterClass* character, time_t curr_time);
	void DrawSystemUI(class D2DClass* direct2D, GameState game_state, time_t actual_curr_time);

	inline const UIContext& GetContext() { return context; }


private:
	UIContext context;

	XMMATRIX vp_matrix;
	XMMATRIX ortho_inverse;

	std::pair<unsigned int, time_t> bonus_effect_;
};