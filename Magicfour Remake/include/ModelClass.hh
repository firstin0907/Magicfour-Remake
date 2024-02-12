#pragma
#include <d3d11.h>
#include <directxmath.h>
#include <fstream>
#include <wrl.h>
#include <vector>

using namespace DirectX;
using namespace std;

template<typename T>
using ComPtr = Microsoft::WRL::ComPtr<T>;

class ModelClass
{
private:
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
	ModelClass(ID3D11Device* device, const char* modelFilename,
		const wchar_t* diffuse_filename,
		const wchar_t* normal_filename = nullptr,
		const wchar_t* emissive_filename = nullptr
	);
	ModelClass(const ModelClass&) = delete;
	~ModelClass();

	void Shutdown();
	void Render(ID3D11DeviceContext*);

	inline const vector<pair<MaterialType, int> >& GetMaterial()
	{
		return m_MaterialList; 
	}

	int GetIndexCount();
	ID3D11ShaderResourceView* GetDiffuseTexture();
	ID3D11ShaderResourceView* GetNormalTexture();
	ID3D11ShaderResourceView* GetEmissiveTexture();


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
	ComPtr<ID3D11Buffer> m_vertexBuffer, m_indexBuffer;
	int m_vertexCount, m_indexCount;
	unique_ptr<class TextureClass> m_DiffuseTexture;
	unique_ptr<class TextureClass> m_NormalTexture;
	unique_ptr<class TextureClass> m_EmissiveTexture;
	vector<ModelType> m_model;

	vector<pair<MaterialType, int> > m_MaterialList;
};