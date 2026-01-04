#pragma once
#include <vector>
#include <memory>
#include <string>
#include <unordered_map>

#include <DirectXMath.h>

enum channel_t
{
	ANIMATION_CHANNEL_XPOS,
	ANIMATION_CHANNEL_YPOS,
	ANIMATION_CHANNEL_ZPOS,
	ANIMATION_CHANNEL_XROT,
	ANIMATION_CHANNEL_YROT,
	ANIMATION_CHANNEL_ZROT
};

class AnimatedObjectClass
{
public:
	using FrameShape = std::unordered_map<std::string, DirectX::XMMATRIX>;

private:
	using XMMATRIX = DirectX::XMMATRIX;

	template<typename T>
	using vector = std::vector<T>;

	struct AnimationNode
	{
		float offset_x, offset_y, offset_z;

		std::string name;
		int channel_num;
		channel_t* channels;

		AnimationNode* parent;
		vector<AnimationNode*> children;

		XMMATRIX link_matrix;
		XMMATRIX shape_transform;
		XMMATRIX global_transform;

		AnimationNode(std::string name, AnimationNode* parent)
			: name(name), parent(parent), channels(nullptr)
		{
			global_transform = link_matrix = DirectX::XMMatrixIdentity();
		};
		~AnimationNode() { delete[] channels; }
	};

	int channels_num, frames_num;
	float frame_time;

	AnimationNode root;
	vector<AnimationNode* > nodes;

	float* frame_info;


private:
	AnimationNode* create_hierarchy(
		AnimationNode* curr_node, std::ifstream& fin);

public:
	/// @brief Merges two frame shapes with linear interpolation.
	/// @param first The first frame shape.
	/// @param second The second frame shape.
	/// @param alpha The interpolation factor (0.0 to 1.0). Alpha = 0.0 returns the first shape, alpha = 1.0 returns the second shape.
	/// @return The merged frame shape.
	static AnimatedObjectClass::FrameShape MergeFrameShapes(
		const AnimatedObjectClass::FrameShape& first,
		const AnimatedObjectClass::FrameShape& second,
		float alpha
	);

	FrameShape UpdateAndGetShapeMatrix(const int frame, XMMATRIX transform_of_root);

	AnimatedObjectClass(const char* filename, bool ignore_first_frame = false);
	~AnimatedObjectClass();
};



