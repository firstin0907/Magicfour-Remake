#include "../include/ModelClass.hh"

#include "../include/TextureClass.hh"
#include "../include/GameException.hh"

#include <sstream>

#define WIDE2(x) L##x
#define WIDE(x) WIDE2(x)
#define WFILE WIDE(__FILE__)

using namespace std;

ModelClass::ModelClass(ID3D11Device* device, ID3D11DeviceContext* deviceContext,
	const char* modelFilename, const char* diffuse_filename, const char* normal_filename)
{
	bool result;

	// Load in the m_model data.
	LoadModel(modelFilename);

	// Calculate the tangent and binormal vectors for the model.
	CalculateModelVectors();

	// Initialize the vertex and index buffers.
	InitializeBuffers(device);

	// Load the texture for this m_model.
	LoadTextures(device, deviceContext, diffuse_filename, normal_filename);
}



ModelClass::~ModelClass()
{
}

void ModelClass::Shutdown()
{
	// Release the m_model data.
	ReleaseModel();
}


void ModelClass::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);

	return;
}


int ModelClass::GetIndexCount()
{
	return m_indexCount;
}

ID3D11ShaderResourceView* ModelClass::GetDiffuseTexture()
{
	if (m_DiffuseTexture) return m_DiffuseTexture->GetTexture();
	else return nullptr;
}


ID3D11ShaderResourceView* ModelClass::GetNormalTexture()
{
	if (m_NormalTexture) return m_NormalTexture->GetTexture();
	else return nullptr;
}



void ModelClass::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];

	// Create the index array.
	indices = new unsigned long[m_indexCount];

	// Load the vertex array and index array with data.
	for (int i = 0; i < m_vertexCount; i++)
	{
		vertices[i].position = XMFLOAT3(m_model[i].x, m_model[i].y, m_model[i].z);
		vertices[i].texture = XMFLOAT2(m_model[i].tu, m_model[i].tv);
		vertices[i].normal = XMFLOAT3(m_model[i].nx, m_model[i].ny, m_model[i].nz);
		vertices[i].tangent = XMFLOAT3(m_model[i].tx, m_model[i].ty, m_model[i].tz);
		vertices[i].binormal = XMFLOAT3(m_model[i].bx, m_model[i].by, m_model[i].bz);

		indices[i] = i;
	}

	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, m_vertexBuffer.GetAddressOf());
	if (FAILED(result)) throw GAME_EXCEPTION(L"Failed to create vertex buffer.");

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, m_indexBuffer.GetAddressOf());
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
	deviceContext->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}


void ModelClass::LoadTextures(ID3D11Device* device,
	ID3D11DeviceContext* deviceContext, const char* diffuse_filename, const char* normal_filename)
{
	// Create and initialize the diffuse texture object.
	m_DiffuseTexture = make_unique<TextureClass>(device, deviceContext, diffuse_filename);

	if (normal_filename)
	{
		// Create and initialize the diffuse texture object.
		m_NormalTexture = make_unique<TextureClass>(device, deviceContext, normal_filename);
	}
	else m_NormalTexture = nullptr;
}


#include <vector>

void ModelClass::LoadModel(const char* filename)
{
	struct Vertex { float x, y, z; };
	vector<Vertex> v_list, vt_list, vn_list;

	ifstream fin(filename);
	if (fin.fail()) throw filenotfound_error(filename, WFILE, __LINE__);

	string buffer;
	while (getline(fin, buffer))
	{
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
			vt_list.push_back({ x, y });
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
				m_model.push_back(face_v[0]);
				m_model.push_back(face_v[i + 1]);
				m_model.push_back(face_v[i + 2]);
			}
		}
	}


	m_indexCount = m_vertexCount = m_model.size();
}

void ModelClass::CalculateModelVectors()
{

	VectorType tangent, binormal;
	for (int i = 0; i < m_vertexCount; i += 3)
	{
		TempVertexType vertex[3] =
		{
			{m_model[i + 0].x, m_model[i + 0].y, m_model[i + 0].z, m_model[i + 0].tu, m_model[i + 0].tv},
			{m_model[i + 1].x, m_model[i + 1].y, m_model[i + 1].z, m_model[i + 1].tu, m_model[i + 1].tv},
			{m_model[i + 2].x, m_model[i + 2].y, m_model[i + 2].z, m_model[i + 2].tu, m_model[i + 2].tv}
		};

		CalculateTangentBinormal(vertex[0], vertex[1], vertex[2], tangent, binormal);
		for (int j = i; j < i + 3; j++)
		{
			m_model[j].tx = tangent.x;
			m_model[j].ty = tangent.y;
			m_model[j].tz = tangent.z;

			m_model[j].bx = binormal.x;
			m_model[j].by = binormal.y;
			m_model[j].bz = binormal.z;
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