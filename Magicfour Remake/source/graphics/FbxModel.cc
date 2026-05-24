#include "graphics/FbxModel.hh"

#include <vector>
#include <string>
#include <stdexcept>
#include <filesystem>

#include <directxmath.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "graphics/TextureClass.hh"

using namespace std;

FbxModel::FbxModel(ID3D11Device* device, const std::string& filepath)
	: filepath_(filepath)
{
	LoadModel(device, filepath);
}

FbxModel::~FbxModel()
{
	for (NodeInfo* node_info : node_info_)
	{
		delete node_info;

	}
}

void FbxModel::LoadModel(ID3D11Device* device, const std::string& file_name)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(file_name, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_LimitBoneWeights);
	if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode)
	{
		throw std::runtime_error("Failed to load model: " + std::string(importer.GetErrorString()));
	}

	global_inverse_transform_ = DirectX::XMLoadFloat4x4(
		reinterpret_cast<const DirectX::XMFLOAT4X4*>(&scene->mRootNode->mTransformation));
	global_inverse_transform_ = DirectX::XMMatrixTranspose(global_inverse_transform_);
	global_inverse_transform_ = DirectX::XMMatrixTranspose(global_inverse_transform_);

	LoadNode(device, nullptr, scene->mRootNode, scene);
	LoadMeshes(device, scene);
	LoadMaterials(device, scene);
}


void FbxModel::LoadNodes(const aiScene* scene)
{
}

#include <fstream>
using namespace DirectX;

void FbxModel::LoadNode(ID3D11Device* device, const NodeInfo* parent, const aiNode* node, const aiScene* scene)
{
	std::fstream fs("debug.txt", std::ios::out | std::ios::app);

	auto link_matrix = node->mTransformation; // Assimp의 행렬은 행 우선이므로 DirectX의 열 우선과 맞추기 위해 전치
	auto bone_matrix = DirectX::XMLoadFloat4x4(reinterpret_cast<const DirectX::XMFLOAT4X4*>(&link_matrix)); // 행렬을 DirectX 형식으로 변환하여 저장
	bone_matrix = DirectX::XMMatrixTranspose(bone_matrix);

	NodeInfo* node_info = new NodeInfo();
	node_info->name = node->mName.C_Str();
	node_info->transformation = bone_matrix;
	node_info->parent = parent;
	node_info_.emplace_back(node_info);
	node_mapping_[node_info->name] = node_info_.size() - 1;

	fs << node_info->name << "\n";
	const int current_index = static_cast<int>(node_info_.size() - 1);
	for (unsigned int i = 0; i < node->mNumChildren; ++i)
	{
		LoadNode(device, node_info, node->mChildren[i], scene);
	}
}

void FbxModel::LoadMeshes(ID3D11Device* device, const aiScene* scene)
{
	for (unsigned int i = 0; i < scene->mNumMeshes; ++i)
	{
		LoadMesh(device, scene->mMeshes[i], scene);
	}
}

void FbxModel::LoadMaterials(ID3D11Device* device, const aiScene* scene)
{
	std::filesystem::path filepath(filepath_);
	std::filesystem::path directory = filepath.parent_path();

	for (unsigned int i = 0; i < scene->mNumMaterials; ++i)
	{
		aiMaterial* raw_material = scene->mMaterials[i];
		aiString str;
		Material material;

		if (raw_material->GetTextureCount(aiTextureType_DIFFUSE) > 0)
		{
			raw_material->GetTexture(aiTextureType_DIFFUSE, 0, &str);
			std::string texture_path = (directory / std::string(str.C_Str())).lexically_normal().string();
			material.diffuse_texture_ = std::make_shared<TextureClass>(device, texture_path);
		}

		if (raw_material->GetTextureCount(aiTextureType_NORMALS) > 0)
		{
			raw_material->GetTexture(aiTextureType_NORMALS, 0, &str);
			std::string texture_path = (directory / std::string(str.C_Str())).lexically_normal().string();
			material.normal_texture_ = std::make_shared<TextureClass>(device, texture_path);
		}

		if (raw_material->GetTextureCount(aiTextureType_EMISSIVE) > 0)
		{
			raw_material->GetTexture(aiTextureType_EMISSIVE, 0, &str);
			std::string texture_path = (directory / std::string(str.C_Str())).lexically_normal().string();
			material.emissive_texture_ = std::make_shared<TextureClass>(device, texture_path);
		}

		materials_.emplace_back(std::move(material));
	}
}


void FbxModel::LoadMesh(ID3D11Device* device, const aiMesh* mesh, const aiScene* scene)
{

	vector<VertexType> vertices(mesh->mNumVertices);

	// load vertex positions, normals, and texture coordinates
	for (unsigned int i = 0; i < mesh->mNumVertices; ++i)
	{
		vertices[i].position = DirectX::XMFLOAT3(
			mesh->mVertices[i].x,
			mesh->mVertices[i].y,
			mesh->mVertices[i].z);

		if (mesh->HasNormals())
		{
			vertices[i].normal = DirectX::XMFLOAT3(
				mesh->mNormals[i].x,
				mesh->mNormals[i].y,
				mesh->mNormals[i].z);
		}

		if (mesh->HasTextureCoords(0))
		{
			vertices[i].tex = DirectX::XMFLOAT2(
				mesh->mTextureCoords[0][i].x,
				mesh->mTextureCoords[0][i].y);
		}
	}

	// load bone weights
	for(size_t i = 0; i < mesh->mNumBones; ++i)
	{
		const aiBone* bone = mesh->mBones[i];
		std::string bone_name(bone->mName.data);

		int bone_index = 0;
		if (bone_mapping_.find(bone_name) == bone_mapping_.end())
		{
			bone_index = bone_count_++;
			bone_mapping_[bone_name] = bone_index;

			BoneInfo bone_info;
			bone_info.offset_matrix = DirectX::XMLoadFloat4x4(
				reinterpret_cast<const DirectX::XMFLOAT4X4*>(&bone->mOffsetMatrix));
			bone_info.offset_matrix = DirectX::XMMatrixTranspose(bone_info.offset_matrix);
			bone_info_.push_back(bone_info);
		}
		else
		{
			bone_index = bone_mapping_[bone_name];
		}


		// Storage bone weights for each vertex influenced by this bone
		for (unsigned int j = 0; j < bone->mNumWeights; ++j)
		{
			unsigned int vertex_id = bone->mWeights[j].mVertexId;
			float weight = bone->mWeights[j].mWeight;

			auto& vertex = vertices[vertex_id];
			for(int k = 0; k < 4; ++k)
			{
				if(vertex.boneIDs[k] == 0) // 빈 슬롯 찾기
				{
					vertex.boneIDs[k] = bone_index;
					vertex.weights[k] = weight;
					break;
				}
			}
		}
	}

	// load indices
	vector<unsigned long> indices;
	for (unsigned int i = 0; i < mesh->mNumFaces; ++i)
	{
		const aiFace& face = mesh->mFaces[i];
		for (unsigned int j = 0; j < face.mNumIndices; ++j)
		{
			indices.push_back(face.mIndices[j]);
		}
	}


	MeshData mesh_data;
	mesh_data.buffer_.Initialize(device, vertices, indices);
	mesh_data.material_index_ = mesh->mMaterialIndex;

	mesh_buffers_.emplace_back(std::move(mesh_data));
}

void FbxModel::Render(ID3D11DeviceContext* device_context, size_t mesh_index)
{
	if (mesh_index >= mesh_buffers_.size())
		throw std::out_of_range("Buffer index out of range.");

	mesh_buffers_[mesh_index].buffer_.Render(device_context);
}

void FbxModel::Update(const std::unordered_map<std::string, DirectX::XMMATRIX> frame_shape)
{
	for(auto& bone_info : bone_info_)
	{
		bone_info.final_transform = DirectX::XMMatrixIdentity();
	}

	for (auto& node_info : node_info_)
	{
		node_info->final_transform = node_info->transformation;

		if (node_info->parent)
		{
			auto it = frame_shape.find(node_info->name);
			if(it != frame_shape.end()) // 테스트용
			{
				node_info->final_transform = DirectX::XMMatrixIdentity()
					* it->second
					* node_info->transformation
					* node_info->parent->final_transform;
			}
			else
			{
				node_info->final_transform = DirectX::XMMatrixIdentity()
					* node_info->transformation
					* node_info->parent->final_transform;
			}
		}
	}

	for (const auto& [bone_name, bone_index] : bone_mapping_)
	{
		if (node_mapping_.find(bone_name) != node_mapping_.end())
		{
			int node_index = node_mapping_[bone_name];
			bone_info_[bone_index].final_transform =
				bone_info_[bone_index].offset_matrix * // 1. 본 로컬 공간으로 이동
				node_info_[node_index]->final_transform * // 2. 노드의 월드 변환 적용
				global_inverse_transform_;
		}
	}
}

ID3D11ShaderResourceView* FbxModel::GetDiffuseTexture(size_t buffer_index)
{
	size_t material_index = mesh_buffers_[buffer_index].material_index_;
	if (!materials_[material_index].diffuse_texture_) return nullptr;
	return materials_[material_index].diffuse_texture_->GetTexture();
}

ID3D11ShaderResourceView* FbxModel::GetNormalTexture(size_t buffer_index)
{
	size_t material_index = mesh_buffers_[buffer_index].material_index_;
	if (!materials_[material_index].normal_texture_) return nullptr;
	return materials_[material_index].normal_texture_->GetTexture();
}

ID3D11ShaderResourceView* FbxModel::GetEmissiveTexture(size_t buffer_index)
{
	size_t material_index = mesh_buffers_[buffer_index].material_index_;
	if (!materials_[material_index].emissive_texture_) return nullptr;
	return materials_[material_index].emissive_texture_->GetTexture();
}
