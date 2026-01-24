#include "core/AnimatedObjectClass.hh"

#include <fstream>
#include <cstring>

#include "core/GameException.hh"

#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)
#define WFILE WIDE(__FILE__)

using namespace std; 
using namespace DirectX;

AnimatedObjectClass::AnimatedObjectClass(const char* filename, bool ignore_first_frame)
	: channels_num_(0), filename_(std::wstring(filename, filename + strlen(filename)))
{
	ifstream fin(filename);
	if (fin.fail()) throw filenotfound_error(filename, WFILE, __LINE__);

	string buffer;
	fin >> buffer; // HIERARCHY
	fin >> buffer; // ROOT
	fin >> buffer; // name

	// Create Tree
	nodes_.push_back(std::make_shared<AnimationNode>(buffer));
	create_hierarchy(*nodes_.begin(), fin);

	fin >> buffer; // MOTION
	fin >> buffer; // Frames:
	fin >> frames_num_;

	fin >> buffer; // Frame
	fin >> buffer; // Time:
	fin >> frame_time_;

	if (ignore_first_frame)
	{
		--frames_num_;
		for (int i = 0; i < channels_num_ && fin >> buffer; i++); // skip first frame
	}

	const int info_sz = frames_num_ * channels_num_;
	if (info_sz <= 0)
	{
		throw GameException(
			L"AnimatedObjectClass: No frame data in file(\"" + this->filename_ + L"\").",
			WFILE, __LINE__);
	}

	frame_info_.assign(info_sz, 0.0f);
	for (int i = 0; i < info_sz; i++)
	{
		if (!(fin >> frame_info_[i]))
		{
			throw GameException(
				L"AnimatedObjectClass: Unexpected end of file(\"" + this->filename_ + L"\") when reading frame data.",
				WFILE, __LINE__);
		}
	}
}

AnimatedObjectClass::~AnimatedObjectClass() = default;

AnimatedObjectClass::FrameShape AnimatedObjectClass::MergeFrameShapes(
	const AnimatedObjectClass::FrameShape& first,
	const AnimatedObjectClass::FrameShape& second,
	float alpha)
{
	AnimatedObjectClass::FrameShape result;

	for (const auto& [name, matrix] : first)
	{
		auto it = second.find(name);
		if (it != second.end())
		{
			XMVECTOR S, R, T;
			XMMatrixDecompose(&S, &R, &T, matrix);
			XMVECTOR S2, R2, T2;
			XMMatrixDecompose(&S2, &R2, &T2, it->second);
			result[name] = XMMatrixScalingFromVector(
				XMVectorLerp(S, S2, alpha))
				* XMMatrixRotationQuaternion(
					XMQuaternionSlerp(R, R2, alpha))
				* XMMatrixTranslationFromVector(
					XMVectorLerp(T, T2, alpha));
		}
	}

	return result;

}

AnimatedObjectClass::FrameShape AnimatedObjectClass::UpdateAndGetShapeMatrix(const int frame, XMMATRIX transform_of_root)
{
	if (nodes_.empty()) return {};

	FrameShape result;

	std::vector<float>::iterator frame_info_it = frame_info_.begin() + (frame % frames_num_) * channels_num_;

	nodes_[0]->global_transform = transform_of_root;

	for (auto& node : nodes_)
	{
		XMMATRIX joint_transform = XMMatrixIdentity();
		for (channel_t channel : node->channels)
		{
			switch (channel)
			{
			case ANIMATION_CHANNEL_XPOS:
				joint_transform = XMMatrixTranslation(*frame_info_it++, 0, 0) * joint_transform; break;
			case ANIMATION_CHANNEL_YPOS:
				joint_transform = XMMatrixTranslation(0, *frame_info_it++, 0) * joint_transform; break;
			case ANIMATION_CHANNEL_ZPOS:
				joint_transform = XMMatrixTranslation(0, 0, *frame_info_it++) * joint_transform; break;
			case ANIMATION_CHANNEL_XROT:
				joint_transform = XMMatrixRotationX(*frame_info_it++ * 0.0174532925f) * joint_transform; break;
			case ANIMATION_CHANNEL_YROT:
				joint_transform = XMMatrixRotationY(*frame_info_it++ * 0.0174532925f) * joint_transform; break;
			case ANIMATION_CHANNEL_ZROT:
				joint_transform = XMMatrixRotationZ(*frame_info_it++ * 0.0174532925f) * joint_transform; break;
				break;

			}
		}

		auto parent = node->parent.lock();
		if (!parent) parent = node;

		node->global_transform = joint_transform * node->link_matrix * parent->global_transform;
		if (!node->children.empty())
		{
			result[node->name] = node->shape_transform * node->global_transform;
		}
	}

	return result;
}

std::shared_ptr<AnimatedObjectClass::AnimationNode>
AnimatedObjectClass::create_hierarchy(
	std::shared_ptr<AnimationNode> curr_node, ifstream& fin)
{
	string buffer;
	fin >> buffer; // {

	while (1)
	{
		fin >> buffer;
		if (buffer == "OFFSET")
		{
			fin >> curr_node->offset_x >> curr_node->offset_y >> curr_node->offset_z;
			curr_node->link_matrix = XMMatrixTranslation(
				curr_node->offset_x, curr_node->offset_y, curr_node->offset_z);
		}

		else if (buffer == "CHANNELS")
		{
			int node_channel_num;
			fin >> node_channel_num;
			channels_num_ += node_channel_num;
			curr_node->channels.resize(node_channel_num);

			for (int i = 0; i < node_channel_num; i++)
			{
				fin >> buffer;
				if (buffer.size() > 2)
				{
					switch (buffer[0])
					{
					case 'X':
						if (buffer[1] == 'p') curr_node->channels[i] = ANIMATION_CHANNEL_XPOS;
						else if (buffer[1] == 'r') curr_node->channels[i] = ANIMATION_CHANNEL_XROT;
						break;
					case 'Y':
						if (buffer[1] == 'p') curr_node->channels[i] = ANIMATION_CHANNEL_YPOS;
						else if (buffer[1] == 'r') curr_node->channels[i] = ANIMATION_CHANNEL_YROT;
						break;
					case 'Z':
						if (buffer[1] == 'p') curr_node->channels[i] = ANIMATION_CHANNEL_ZPOS;
						else if (buffer[1] == 'r') curr_node->channels[i] = ANIMATION_CHANNEL_ZROT;
						break;
					}
				}
			}
		}
		else if (buffer == "JOINT" || buffer == "End")
		{
			fin >> buffer;

			auto child_node = std::make_shared<AnimationNode>(buffer, curr_node);
			nodes_.push_back(child_node);

			curr_node->children.push_back(create_hierarchy(child_node, fin));
		}
		else if (buffer == "}") break;
	}



	if (!curr_node->children.empty())
	{
		auto& child = curr_node->children[0];

		XMVECTOR child_offset = { child->offset_x, child->offset_y, child->offset_z };
		float length = sqrt(child->offset_x * child->offset_x + child->offset_y * child->offset_y
			+ child->offset_z * child->offset_z);

		child_offset = child_offset / length;

		auto axis = XMVector3Cross({ 0, 1, 0 }, child_offset);
		float angle = acos(child->offset_y / length);

		if (XMVector3Equal(axis, XMVectorZero())) axis = { 0, 1, 0 }, angle = 0;

		curr_node->shape_transform = XMMatrixTranslation(0, 1, 0)
			* XMMatrixScaling(0.125f, length / 2, 0.125f) * XMMatrixRotationAxis(axis, angle);
	}

	return curr_node;
}
