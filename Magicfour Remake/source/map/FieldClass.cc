#include "map/FieldClass.hh"

#include <DirectXMath.h>
#include <fstream>
#include <corecrt_math_defines.h>

#include "core/GameException.hh"
#include "shader/ShaderManager.hh"
#include "shader/NormalMapShaderClass.hh"
#include "shader/LightShaderClass.hh"
#include "shader/FireShaderClass.hh"

using namespace std;
using namespace DirectX;

FieldClass::FieldClass(const char* filename)
{
	ifstream fin(filename);

	if(fin.fail()) throw filenotfound_error(filename, WFILE, __LINE__);

	while (!fin.eof())
	{
		int left, bottom, right, top;
		fin >> left >> bottom >> right >> top;
		grounds_.emplace_back(rect_t{ left, bottom, right, top });
	}
}

void FieldClass::Draw(time_t curr_time, time_t time_delta, class ShaderManager* shader_manager,
	ResourceMap<class ModelClass>& models, ResourceMap<class TextureClass>& textures) const
{
	// Draw Background
	const static XMMATRIX kBackgroundMarix = XMMatrixScaling(192.0f, 153.6f, 1) * XMMatrixTranslation(0, 0, 100.0f);
	shader_manager->light_shader_->PushRenderQueue(models.get("background"), kBackgroundMarix);

	// Draw grounds
	for (const auto& ground : GetGrounds())
	{
		// for grass
		rect_t grass_range = ground.GetRange();
		const int grass_drawing_steps = grass_range.get_w() / 100000 + 1;
		for (long long i = 0; i < grass_drawing_steps; i++)
		{
			rect_t grass_section_range = grass_range;

			const long long curr_x = grass_range.x1 + grass_range.get_w() * i / grass_drawing_steps;
			const long long next_x = grass_range.x1 + grass_range.get_w() * (i + 1) / grass_drawing_steps;
			grass_section_range.x1 = curr_x, grass_section_range.x2 = next_x;

			const XMMATRIX&& grass_matrix = XMMatrixRotationY(3 * M_PI_2) * XMMatrixRotationZ(3 * M_PI_2) *
				XMMatrixTranslation(0.0f, 0.0f, -0.0001f) *
				grass_section_range.toMatrix();

			shader_manager->fire_shader_->PushRenderQueue(
				models.get("grass"),
				grass_matrix,
				{ -0.3f, -0.1f, -0.3f },
				{ 1.0f, 2.0f, 3.0f },
				{ 0.1f, 0.2f },
				{ 0.1f, 0.3f },
				{ 0.1f, 0.1f },
				0.4f, 0.0f);
		}

		rect_t ground_range = ground.GetRange();
		const int ground_drawing_steps = grass_range.get_w() / 420000 + 1;
		for (long long i = 0; i < ground_drawing_steps; i++)
		{
			// for ground
			rect_t ground_display_range = ground_range;

			const long long curr_x = ground_range.x1 + ground_range.get_w() * i / ground_drawing_steps;
			const long long next_x = ground_range.x1 + ground_range.get_w() * (i + 1) / ground_drawing_steps;
			ground_display_range.x1 = curr_x, ground_display_range.x2 = next_x;

			shader_manager->light_shader_->PushRenderQueue(models.get("cube"),
				ground_display_range.toMatrix());
		}
	}

	shader_manager->normalMap_shader_->PushRenderQueue(models.get("gem"),
		XMMatrixScaling(3, 3, 3) * XMMatrixTranslation(1750000 * kScope, (kGroundY - 50000) * kScope, +0.5f));

	shader_manager->normalMap_shader_->PushRenderQueue(models.get("gem"),
		XMMatrixScaling(4, 4, 4) * XMMatrixTranslation(1950000 * kScope, (kGroundY - 50000) * kScope, 0.0f));

}