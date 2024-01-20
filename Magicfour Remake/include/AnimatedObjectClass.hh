#pragma once
#include <vector>
#include <memory>
#include <string>

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

using namespace std;
using namespace DirectX;

class AnimatedObjectClass
{
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
			global_transform = link_matrix = XMMatrixIdentity();
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
		AnimationNode* curr_node, ifstream& fin);

public:
	void UpdateGlobalMatrices(const int frame, XMMATRIX, vector<XMMATRIX>& result);

	AnimatedObjectClass(const char* filename);
	~AnimatedObjectClass();
};

