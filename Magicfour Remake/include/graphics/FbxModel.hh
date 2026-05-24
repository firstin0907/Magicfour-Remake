#ifndef FBXMODEL_HH
#define FBXMODEL_HH

#include <d3d11.h>

#include <vector>
#include <string>
#include <unordered_map>

#include <DirectXMath.h>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "graphics/Mesh.h"

class FbxModel
{
    struct VertexType
    {
        DirectX::XMFLOAT3 position;
        DirectX::XMFLOAT2 tex;
        DirectX::XMFLOAT3 normal;

        int boneIDs[4] = { 0 };
        float weights[4] = { 0.f };
    };

    struct NodeInfo
    {
        std::string name;
        DirectX::XMMATRIX transformation;
        DirectX::XMMATRIX final_transform;
        const NodeInfo* parent;
	};

    struct BoneInfo
    {
        DirectX::XMMATRIX offset_matrix;   // aiBone->mOffsetMatrix
        DirectX::XMMATRIX final_transform; // 애니메이션 적용 후 최종 행렬
    };

    struct MeshData
    {
        MeshBuffer<VertexType> buffer_;
		unsigned int material_index_;
	};

    struct Material
    {
        std::shared_ptr<class TextureClass> diffuse_texture_;
        std::shared_ptr<class TextureClass> normal_texture_;
        std::shared_ptr<class TextureClass> emissive_texture_;
	};

public:
    FbxModel() = delete;
    FbxModel(ID3D11Device* device, const std::string& filepath);
	~FbxModel();

private:
	void LoadModel(ID3D11Device* device, const std::string& file_name);
    void LoadNodes(const aiScene* scene);
    void LoadMeshes(ID3D11Device* device, const aiScene* scene);
	void LoadMaterials(ID3D11Device* device, const aiScene* scene);
    
	void LoadNode(ID3D11Device* device, const NodeInfo* parent, const aiNode* node, const aiScene* scene);
    void LoadMesh(ID3D11Device* device, const aiMesh* mesh, const aiScene* scene);

public:
	void Update(const std::unordered_map<std::string, DirectX::XMMATRIX> frame_shape);
	void Render(ID3D11DeviceContext* device_context, size_t buffer_index);

    ID3D11ShaderResourceView* GetDiffuseTexture(size_t buffer_index);
    ID3D11ShaderResourceView* GetNormalTexture(size_t buffer_index);
    ID3D11ShaderResourceView* GetEmissiveTexture(size_t buffer_index);

    size_t GetMeshCount() const { return mesh_buffers_.size(); }
    size_t GetMeshVertexCount(size_t buffer_index) const { return mesh_buffers_[buffer_index].buffer_.GetIndexCount(); }

private:
    std::string filepath_;

	std::vector<MeshData> mesh_buffers_;
	std::vector<Material> materials_;

public:
    std::unordered_map<std::string, int> bone_mapping_;
    std::unordered_map<std::string, int> node_mapping_;
	DirectX::XMMATRIX global_inverse_transform_;

    std::vector<NodeInfo*> node_info_;
    std::vector<BoneInfo> bone_info_;

    int bone_count_ = 0;
};

#endif