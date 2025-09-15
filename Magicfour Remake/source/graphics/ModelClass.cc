#include "graphics/ModelClass.hh"

#include "graphics/TextureClass.hh"
#include "core/GameException.hh"

#include <sstream>
#include <algorithm>

#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)
#define WFILE WIDE(__FILE__)

using namespace std;

ModelClass* ModelClass::lastRenderedModel_ = nullptr;

ModelClass::ModelClass(ID3D11Device* device,
	const std::string& modelFilename,
	const std::string& diffuse_filename,
	const std::string& normal_filename,
	const std::string& emissive_filename)
	: ModelClass(device,
		modelFilename.c_str(),
		diffuse_filename.empty() ? nullptr : std::wstring(diffuse_filename.begin(), diffuse_filename.end()).c_str(),
		normal_filename.empty() ? nullptr : std::wstring(normal_filename.begin(), normal_filename.end()).c_str(),
		emissive_filename.empty() ? nullptr : std::wstring(emissive_filename.begin(), emissive_filename.end()).c_str()) {};

ModelClass::ModelClass(ID3D11Device* device,
	const char* modelFilename,
	const wchar_t* diffuse_filename,
	const wchar_t* normal_filename,
	const wchar_t* emissive_filename)
{
	// Load in the model_ data.
	LoadModel(modelFilename);

	// Calculate the tangent and binormal vectors for the model.
	CalculateModelVectors();

	// Initialize the vertex and index buffers.
	InitializeBuffers(device);

	// Load the texture for this model_.
	LoadTextures(device, diffuse_filename, normal_filename, emissive_filename);
}



ModelClass::~ModelClass()
{
}

void ModelClass::Shutdown()
{
	// Release the model_ data.
	ReleaseModel();
}


void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	if (lastRenderedModel_ == this) return;

	lastRenderedModel_ = this;

	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}


int ModelClass::GetIndexCount()
{
	return indexCount_;
}

ID3D11ShaderResourceView* ModelClass::GetDiffuseTexture()
{
	if (diffuseTexture_) return diffuseTexture_->GetTexture();
	else return nullptr;
}


ID3D11ShaderResourceView* ModelClass::GetNormalTexture()
{
	if (normalTexture_) return normalTexture_->GetTexture();
	else return nullptr;
}

ID3D11ShaderResourceView* ModelClass::GetEmissiveTexture()
{
	if (emissiveTexture_) return emissiveTexture_->GetTexture();
	return nullptr;
}

void ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// Create the vertex array.
	vertices = new VertexType[vertexCount_];

	// Create the index array.
	indices = new unsigned long[indexCount_];

	// Load the vertex array and index array with data.
	for (int i = 0; i < vertexCount_; i++)
	{
		vertices[i].position = XMFLOAT3(model_[i].x, model_[i].y, model_[i].z);
		vertices[i].texture = XMFLOAT2(model_[i].tu, model_[i].tv);
		vertices[i].normal = XMFLOAT3(model_[i].nx, model_[i].ny, model_[i].nz);
		vertices[i].tangent = XMFLOAT3(model_[i].tx, model_[i].ty, model_[i].tz);
		vertices[i].binormal = XMFLOAT3(model_[i].bx, model_[i].by, model_[i].bz);

		indices[i] = i;
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * vertexCount_;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, vertexBuffer_.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create vertex buffer.");

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * indexCount_;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, indexBuffer_.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create index buffer.");

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;

}

void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, vertexBuffer_.GetAddressOf(), &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(indexBuffer_.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


void ModelClass::LoadTextures(ID3D11Device* device,
	const wchar_t* diffuse_filename, const wchar_t* normal_filename, const wchar_t* emissive_filename)
{
	// Create and initialize the diffuse texture object.
	diffuseTexture_ = make_unique<TextureClass>(device, diffuse_filename);

	if (normal_filename)
	{
		// Create and initialize the diffuse texture object.
		normalTexture_ = make_unique<TextureClass>(device, normal_filename);
	}
	else normalTexture_ = nullptr;


	if (emissive_filename)
	{
		emissiveTexture_ = make_unique<TextureClass>(device, emissive_filename);
	}
	else emissiveTexture_ = make_unique<TextureClass>(device, L"data/texture/black.png");
}



#include <vector>
#include <map>

using namespace std;

void ModelClass::LoadModel(const char* filename)
{
	struct Vertex { float x, y, z; };
	vector<Vertex> v_list, vt_list, vn_list;
	map<string, MaterialType> meterial;

	auto read_mtl_file = [&](const char* filename) {
		string mtl_name = "";
		MaterialType curr_mtl = {
			XMFLOAT3(1.0f, 1.0f, 1.0f),
			XMFLOAT3(1.0f, 1.0f, 1.0f),
			XMFLOAT3(1.0f, 1.0f, 1.0f)
		};

		ifstream fin(filename);
		if (fin.fail()) return; //throw filenotfound_error(filename, WFILE, __LINE__);

		string buffer;
		while (getline(fin, buffer))
		{
			istringstream iss(buffer);
			getline(iss, buffer, ' ');

			if (buffer == "#") continue; // case: comments
			else if (buffer == "newmtl")
			{
				if (mtl_name.size() > 0)
				{
					meterial[mtl_name] = curr_mtl;
					curr_mtl.ambient = curr_mtl.diffuse = curr_mtl.specular = XMFLOAT3(1.0f, 1.0f, 1.0f);
				}
				iss >> mtl_name;
			}
			else if (buffer == "Ka")
			{
				float r, g, b; iss >> r >> g >> b;
				curr_mtl.ambient = XMFLOAT3(r, g, b);
			}
			else if (buffer == "Kd")
			{
				float r, g, b; iss >> r >> g >> b;
				curr_mtl.diffuse = XMFLOAT3(r, g, b);
			}
			else if (buffer == "Ks")
			{
				float r, g, b; iss >> r >> g >> b;
				curr_mtl.specular = XMFLOAT3(r, g, b);
			}
		}

		if (mtl_name.size() > 0)
		{
			meterial[mtl_name] = curr_mtl;
		}
	};

	

	ifstream fin(filename);
	if (fin.fail()) throw filenotfound_error(filename, WFILE, __LINE__);

	string buffer;
	while (getline(fin, buffer))
	{
		replace(buffer.begin(), buffer.end(), '\r', ' ');
		istringstream iss(buffer);
		getline(iss, buffer, ' ');

		if (buffer == "#") continue; // case: comments
		else if (buffer == "v")
		{
			float x, y, z;
			iss >> x >> y >> z;
			v_list.push_back({ x, y, z });
		}
		else if (buffer == "vt")
		{
			float x, y;
			iss >> x >> y;
			vt_list.push_back({ x, 1 - y });
		}
		else if (buffer == "vn")
		{
			float x, y, z;
			iss >> x >> y >> z;
			vn_list.push_back({ x, y, z });
		}
		else if (buffer == "f")
		{
			vector<ModelType> face_v;
			while (getline(iss, buffer, ' '))
			{
				if (buffer.size() == 0) continue;
				face_v.push_back({ 0 });
				istringstream point_iss(buffer);

				getline(point_iss, buffer, '/');
				int num = stoi(buffer) - 1;

				face_v.back().x = v_list[num].x;
				face_v.back().y = v_list[num].y;
				face_v.back().z = v_list[num].z;

				if (getline(point_iss, buffer, '/'))
				{
					if (!buffer.empty())
					{
						int num = stoi(buffer) - 1;
						face_v.back().tu = vt_list[num].x;
						face_v.back().tv = vt_list[num].y;
					}
				}

				if (getline(point_iss, buffer, '/'))
				{
					if (!buffer.empty())
					{
						int num = stoi(buffer) - 1;
						face_v.back().nx = vn_list[num].x;
						face_v.back().ny = vn_list[num].y;
						face_v.back().nz = vn_list[num].z;
					}
				}
			}

			for (size_t i = 0; i < face_v.size() - 2; i++)
			{
				model_.push_back(face_v[0]);
				model_.push_back(face_v[i + 1]);
				model_.push_back(face_v[i + 2]);
			}
		}
		else if (buffer == "mtllib")
		{
			string path = string(filename);
			const int last = path.find_last_of('/');
			path = path.substr(0, path.find_last_of('/') + 1);
			iss >> buffer;

			read_mtl_file((path + buffer).c_str());
		}
		else if (buffer == "usemtl")
		{
			iss >> buffer;
			if (meterial.find(buffer) == meterial.end()) continue;
			materialList_.emplace_back(meterial[buffer], (int)model_.size());
		}
	}

	if (materialList_.empty()) materialList_.emplace_back(MaterialType{
			XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT3(1.0f, 1.0f, 1.0f) }, 0);

	indexCount_ = vertexCount_ = model_.size();
}

void ModelClass::CalculateModelVectors()
{

	VectorType tangent, binormal;
	for (int i = 0; i < vertexCount_; i += 3)
	{
		TempVertexType vertex[3] =
		{
			{model_[i + 0].x, model_[i + 0].y, model_[i + 0].z, model_[i + 0].tu, model_[i + 0].tv},
			{model_[i + 1].x, model_[i + 1].y, model_[i + 1].z, model_[i + 1].tu, model_[i + 1].tv},
			{model_[i + 2].x, model_[i + 2].y, model_[i + 2].z, model_[i + 2].tu, model_[i + 2].tv}
		};

		CalculateTangentBinormal(vertex[0], vertex[1], vertex[2], tangent, binormal);
		for (int j = i; j < i + 3; j++)
		{
			model_[j].tx = tangent.x;
			model_[j].ty = tangent.y;
			model_[j].tz = tangent.z;

			model_[j].bx = binormal.x;
			model_[j].by = binormal.y;
			model_[j].bz = binormal.z;
		}
	}
}

void ModelClass::CalculateTangentBinormal(TempVertexType vertex1, TempVertexType vertex2, TempVertexType vertex3, VectorType& tangent, VectorType& binormal)
{
	float vector1[3], vector2[3];
	float tuVector[2], tvVector[2];
	float den;
	float length;


	// Calculate the two vectors for this face.
	vector1[0] = vertex2.x - vertex1.x;
	vector1[1] = vertex2.y - vertex1.y;
	vector1[2] = vertex2.z - vertex1.z;

	vector2[0] = vertex3.x - vertex1.x;
	vector2[1] = vertex3.y - vertex1.y;
	vector2[2] = vertex3.z - vertex1.z;

	// Calculate the tu and tv texture space vectors.
	tuVector[0] = vertex2.tu - vertex1.tu;
	tvVector[0] = vertex2.tv - vertex1.tv;

	tuVector[1] = vertex3.tu - vertex1.tu;
	tvVector[1] = vertex3.tv - vertex1.tv;

	// Calculate the denominator of the tangent/binormal equation.
	den = 1.0f / (tuVector[0] * tvVector[1] - tuVector[1] * tvVector[0]);

	// Calculate the cross products and multiply by the coefficient to get the tangent and binormal.
	tangent.x = (tvVector[1] * vector1[0] - tvVector[0] * vector2[0]) * den;
	tangent.y = (tvVector[1] * vector1[1] - tvVector[0] * vector2[1]) * den;
	tangent.z = (tvVector[1] * vector1[2] - tvVector[0] * vector2[2]) * den;

	binormal.x = (tuVector[0] * vector2[0] - tuVector[1] * vector1[0]) * den;
	binormal.y = (tuVector[0] * vector2[1] - tuVector[1] * vector1[1]) * den;
	binormal.z = (tuVector[0] * vector2[2] - tuVector[1] * vector1[2]) * den;

	// Calculate the length of this normal.
	length = sqrt((tangent.x * tangent.x) + (tangent.y * tangent.y) + (tangent.z * tangent.z));

	// Normalize the normal and then store it
	tangent.x = tangent.x / length;
	tangent.y = tangent.y / length;
	tangent.z = tangent.z / length;

	// Calculate the length of this normal.
	length = sqrt((binormal.x * binormal.x) + (binormal.y * binormal.y) + (binormal.z * binormal.z));

	// Normalize the normal and then store it
	binormal.x = binormal.x / length;
	binormal.y = binormal.y / length;
	binormal.z = binormal.z / length;

	return;
}



void ModelClass::ReleaseModel()
{
	return;
}