#include "ui/UserInterfaceClass.hh"

#include "graphics/TextureClass.hh"
#include "graphics/ModelClass.hh"
#include "graphics/FontClass.hh"
#include "game-object/MonsterClass.hh"
#include "core/GameException.hh"
#include "shader/TextureShaderClass.hh"
#include "game-object/CharacterClass.hh"
#include "core/D2DClass.hh"
#include "util/ResourceMap.hh"
#include "graphics/BitmapClass.hh"
#include "core/GameObjectList.hh"

#include "ui/MonsterUI.hh"
#include "ui/CharacterUI.hh"
#include "ui/SystemUI.hh"

using namespace std;
using namespace DirectX;

UserInterfaceClass::UserInterfaceClass(class D2DClass* direct2D,
	ID3D11Device* device, int screen_width, int screen_height)
	: context(screen_width, screen_height)
{
	const std::unordered_map<std::string, DWRITE_TEXT_ALIGNMENT> kTextAlignmentMap = {
		{"leading", DWRITE_TEXT_ALIGNMENT_LEADING},
		{"trailing", DWRITE_TEXT_ALIGNMENT_TRAILING},
		{"center", DWRITE_TEXT_ALIGNMENT_CENTER},
		{"justified", DWRITE_TEXT_ALIGNMENT_JUSTIFIED}
	};

	const std::unordered_map<std::string, DWRITE_PARAGRAPH_ALIGNMENT> kParagraphAlignmentMap = {
		{"near", DWRITE_PARAGRAPH_ALIGNMENT_NEAR},
		{"far", DWRITE_PARAGRAPH_ALIGNMENT_FAR},
		{"center", DWRITE_PARAGRAPH_ALIGNMENT_CENTER}
	};

	auto font_loader = [&kTextAlignmentMap, &kParagraphAlignmentMap, direct2D](xml_node_wrapper node)
		-> std::shared_ptr<FontClass>
		{
			// find text alignment key
			auto text_align = kTextAlignmentMap.find(node.get_attr("textAlignment", "leading"));
			auto paragraph_align = kParagraphAlignmentMap.find(node.get_attr("paragraphAlignment", "near"));

			return make_shared<FontClass>(
				direct2D,
				node.get_required_attr("family"),
				stof(node.get_required_attr("size")),
				(text_align != kTextAlignmentMap.end()) ? text_align->second : DWRITE_TEXT_ALIGNMENT_LEADING,
				(paragraph_align != kParagraphAlignmentMap.end()) ? paragraph_align->second : DWRITE_PARAGRAPH_ALIGNMENT_NEAR
			);
			
		};
	context.fonts_.loadFromXML("data/resources.xml", "Font", font_loader);

	auto bitmap_loader = [direct2D](xml_node_wrapper node) -> std::shared_ptr<BitmapClass>
	{
		return make_shared<BitmapClass>(direct2D, node.get_required_attr("src"));
	};
	context.bitmaps_.loadFromXML("data/resources.xml", "Bitmap", bitmap_loader);

}

UserInterfaceClass::~UserInterfaceClass()
{
}

void UserInterfaceClass::CalculateScreenPos(const XMMATRIX& mvp_matrix,
	const XMMATRIX& ortho_inv, float& x, float& y) const 
{
	XMVECTOR point = { 0, 0, 0, 1 };
	point =	XMVector4Transform(point, mvp_matrix);
	point /= point.m128_f32[3];
	point.m128_f32[2] = 0;
	point = XMVector4Transform(point, ortho_inv);
	
	x = point.m128_f32[0] + context.screen_width_ / 2.0f;
	y = context.screen_height_ / 2.0f - point.m128_f32[1];
}

void UserInterfaceClass::CalculateScreenPos(
	const XMMATRIX& world_matrix, float& x, float& y) const
{
	CalculateScreenPos(world_matrix * vp_matrix, ortho_inverse, x, y);
}


void UserInterfaceClass::Begin2dDraw(D2DClass* direct2D, const XMMATRIX& vp_matrix, const XMMATRIX& ortho_matrix)
{
	direct2D->BeginDraw();
	ortho_inverse = XMMatrixInverse(nullptr, ortho_matrix);
	this->vp_matrix = vp_matrix;
}

void UserInterfaceClass::End2dDraw(D2DClass* direct2D)
{
	direct2D->EndDraw();
}

void UserInterfaceClass::DrawMonsterUI(D2DClass* direct2D, GameObjectList& monsters, time_t curr_time)
{
	for (auto& object : monsters.elements)
	{
		MonsterClass* monster = static_cast<MonsterClass*>(object.get());
		MonsterUI::DrawUI(direct2D, this, monster, curr_time);
	}
}

void UserInterfaceClass::DrawCharacterUI(
	D2DClass* direct2D, CharacterClass* character, time_t curr_time)
{
	CharacterUI::DrawUI(direct2D, this, character, curr_time);
}

void UserInterfaceClass::DrawSystemUI(D2DClass* direct2D, GameState game_state,
	time_t actual_curr_time)
{
	constexpr time_t kSeconds = 1'000;

	if (game_state != context.system_context_.game_state)
	{
		context.system_context_.game_state = game_state;
		context.system_context_.state_start_time = actual_curr_time;
	}

	if (context.system_context_.prev_frame_time / kSeconds != actual_curr_time / kSeconds)
	{
		context.system_context_.prev_frame_cnt = context.system_context_.frame_cnt;
		context.system_context_.frame_cnt = 0;
	}

	SystemUI::DrawUI(direct2D, this, actual_curr_time);

	context.system_context_.frame_cnt++;
	context.system_context_.prev_frame_time = actual_curr_time;
}

