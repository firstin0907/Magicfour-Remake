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

public:
	ModelClass(ID3D11Device* device, ID3D11DeviceContext* deviceContext,
		const char* modelFilename, const char* diffuse_filename, const char* normal_filename = nullptr);
	ModelClass(const ModelClass&) = delete;
	~ModelClass();

	void Shutdown();
	void Render(ID3D11DeviceContext*);

	int GetIndexCount();
	ID3D11ShaderResourceView* GetDiffuseTexture();
	ID3D11ShaderResourceView* GetNormalTexture();


private:
	bool InitializeBuffers(ID3D11Device*);
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTextures(ID3D11Device*, ID3D11DeviceContext*, const char*, const char*);

	bool LoadModel(const char*);
	void ReleaseModel();


	void CalculateModelVectors();
	void CalculateTangentBinormal(TempVertexType, TempVertexType, TempVertexType, VectorType&, VectorType&);


private:
	ComPtr<ID3D11Buffer> m_vertexBuffer, m_indexBuffer;
	int m_vertexCount, m_indexCount;
	unique_ptr<class TextureClass> m_DiffuseTexture;
	unique_ptr<class TextureClass> m_NormalTexture;
	vector<ModelType> m_model;
};