#pragma
#include <d3d11.h>
#include <directxmath.h>
#include <fstream>
#include <wrl.h>
#include <vector>
#include <directxcollision.h>
#include <variant>

class ModelClass
{
private:
	template<typename T>
	using ComPtr = Microsoft::WRL::ComPtr<T>;

	template<typename T>
	using unique_ptr = std::unique_ptr<T>;

	template<typename T>
	using vector = std::vector<T>;

	using XMFLOAT2 = DirectX::XMFLOAT2;
	using XMFLOAT3 = DirectX::XMFLOAT3;
	using XMFLOAT4 = DirectX::XMFLOAT4;

	struct VertexType
	{
		XMFLOAT3 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;

		// For normal mapping.
		XMFLOAT3 tangent;
		XMFLOAT3 binormal;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;

		// For normal mapping.
		float tx, ty, tz;
		float bx, by, bz;
	};

	struct TempVertexType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	struct VectorType
	{
		float x, y, z;
	};

	struct MaterialType
	{
		XMFLOAT3 ambient;
		XMFLOAT3 diffuse;
		XMFLOAT3 specular;
	};

public:
	ModelClass(ID3D11Device* device, const char* model_filename,
		const wchar_t* diffuse_filename,
		const wchar_t* normal_filename = nullptr,
		const wchar_t* emissive_filename = nullptr
	);

	ModelClass(ID3D11Device* device,
		const std::string& model_filename,
		const std::string& diffuse_filename,
		const std::string& normal_filename = "",
		const std::string& emissive_filename = ""
	);

	ModelClass(ID3D11Device* device,
		const std::string& model_filename,
		const std::shared_ptr<class TextureClass>& diffuse_texture,
		const std::shared_ptr<class TextureClass>& normal_texture = nullptr,
		const std::shared_ptr<class TextureClass>& emissive_texture = nullptr
	);


	ModelClass(const ModelClass&) = delete;
	~ModelClass();

	void Shutdown();
	void Render(ID3D11DeviceContext*);

	inline const vector<std::pair<MaterialType, int> >& GetMaterial()
	{
		return material_list_; 
	}

	int GetIndexCount();
	ID3D11ShaderResourceView* GetDiffuseTexture();
	ID3D11ShaderResourceView* GetNormalTexture();
	ID3D11ShaderResourceView* GetEmissiveTexture();

	const std::variant<DirectX::BoundingBox, DirectX::BoundingSphere>& GetBoundingVolume() const
	{
		return bounding_volume_;
	}


private:
	void InitializeBuffers(ID3D11Device*);
	void RenderBuffers(ID3D11DeviceContext*);

	void LoadTextures(ID3D11Device* device,
		const wchar_t* diffuse_filename,
		const wchar_t* normal_filename,
		const wchar_t* emissive_filename);

	void LoadModel(const char*);
	void ReleaseModel();


	void CalculateModelVectors();
	void CalculateTangentBinormal(TempVertexType, TempVertexType, TempVertexType, VectorType&, VectorType&);


private:
	ComPtr<ID3D11Buffer> vertexBuffer_, indexBuffer_;
	int vertexCount_, indexCount_;

	std::shared_ptr<class TextureClass> diffuse_texture_;
	std::shared_ptr<class TextureClass> normal_texture_;
	std::shared_ptr<class TextureClass> emissive_texture_;

	vector<ModelType> model_;

	vector<std::pair<MaterialType, int> > material_list_;
	
	std::variant<DirectX::BoundingBox, DirectX::BoundingSphere> bounding_volume_;
};