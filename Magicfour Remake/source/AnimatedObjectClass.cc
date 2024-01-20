#include "../include/AnimatedObjectClass.hh"

#include <fstream>

#include "../include/GameException.hh"

using namespace std; 

#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)
#define WFILE WIDE(__FILE__)


AnimatedObjectClass::AnimationNode*
	AnimatedObjectClass::create_hierarchy(
		AnimationNode* curr_node, ifstream& fin)
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
			fin >> curr_node->channel_num;
			channels_num += curr_node->channel_num;
			curr_node->channels = new channel_t[curr_node->channel_num];

			for (int i = 0; i < curr_node->channel_num; i++)
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

			AnimationNode* child_node = new AnimationNode(buffer, curr_node);
			nodes.push_back(child_node);

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

AnimatedObjectClass::~AnimatedObjectClass()
{
	for (auto& node : nodes) delete node;
	nodes.clear();

	delete[] frame_info;
}

void AnimatedObjectClass::UpdateGlobalMatrices(const int frame, XMMATRIX transform_of_root, vector<XMMATRIX>& result)
{
	float* frame_info_it = frame_info + (frame % frames_num) * channels_num;

	root.global_transform = transform_of_root;

	for (auto& node : nodes)
	{
		XMMATRIX joint_transform = XMMatrixIdentity();
		for (int i = 0; i < node->channel_num; i++)
		{
			switch (node->channels[i])
			{
			case ANIMATION_CHANNEL_XPOS:
				joint_transform = XMMatrixTranslation(*frame_info_it++, 0, 0) * joint_transform; break;
			case ANIMATION_CHANNEL_YPOS:
				joint_transform = XMMatrixTranslation(0, *frame_info_it++, 0) * joint_transform; break;
			case ANIMATION_CHANNEL_ZPOS:
				joint_transform = XMMatrixTranslation(0, 0, *frame_info_it++ ) * joint_transform; break;
				break;
			case ANIMATION_CHANNEL_XROT:
				joint_transform = XMMatrixRotationX(*frame_info_it++ * 0.0174532925f) * joint_transform; break;
			case ANIMATION_CHANNEL_YROT:
				joint_transform = XMMatrixRotationY(*frame_info_it++ * 0.0174532925f) * joint_transform; break;
			case ANIMATION_CHANNEL_ZROT:
				joint_transform = XMMatrixRotationZ(*frame_info_it++ * 0.0174532925f) * joint_transform; break;
				break;
				
			}
		}
		node->global_transform = joint_transform * node->link_matrix * node->parent->global_transform;
		if (!node->children.empty())
		{
			result.push_back(node->shape_transform * node->global_transform);
		}
	}
}

AnimatedObjectClass::AnimatedObjectClass(const char* filename)
	: channels_num(0), frame_info(nullptr), root("", nullptr)
{
	ifstream fin(filename);
	if (fin.fail()) throw filenotfound_error(filename, WFILE, __LINE__);

	string buffer;
	fin >> buffer; // HIERARCHY
	fin >> buffer; // ROOT
	fin >> buffer; // name

	// Create Tree
	AnimationNode* root_node = new AnimationNode(buffer, &root);
	nodes.push_back(root_node);
	create_hierarchy(root_node, fin);

	fin >> buffer; // MOTION
	fin >> buffer; // Frames:
	fin >> frames_num;

	fin >> buffer; // Frame
	fin >> buffer; // Time:
	fin >> frame_time;

	const int info_sz = frames_num * channels_num;
	frame_info = new float[info_sz];
	for (int i = 0; i < info_sz; i++) fin >> frame_info[i];

	fin.close();

	ofstream fout("test.txt");
	for (auto& node : nodes) fout << node->name << std::endl;
	fout << frames_num << std::endl << frame_time;
	fout.close();
}