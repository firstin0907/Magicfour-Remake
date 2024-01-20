#include "../include/TextureClass.hh"

TextureClass::TextureClass(ID3D11Device* device,
	ID3D11DeviceContext* deviceContext, const char* filename)
{
	m_targaData = nullptr;
	m_texture = nullptr;
	m_textureView = nullptr;

	Initialize(device, deviceContext, filename);
}

TextureClass::TextureClass(const TextureClass& other)
{
}

TextureClass::~TextureClass()
{
	Shutdown();
}

bool TextureClass::Initialize(ID3D11Device* device,
	ID3D11DeviceContext* deviceContext, const char* filename)
{
	bool result;
	HRESULT hResult;

	result = LoadTarga32Bit(filename);
	if (!result)
	{
		result = LoadTarga24Bit(filename);
		if (!result) return false;
	}

	D3D11_TEXTURE2D_DESC textureDesc;
	textureDesc.Height = m_height;
	textureDesc.Width = m_width;
	textureDesc.MipLevels = 0;
	textureDesc.ArraySize = 1;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT; // FOR STATIC DATA??
	textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	// Create the empty texture.
	hResult = device->CreateTexture2D(&textureDesc, nullptr, m_texture.GetAddressOf());
	if (FAILED(hResult)) return false;

	unsigned int rowPitch = (m_width * 4) * sizeof(unsigned char);

	// Copy the targa image data into the texture.
	deviceContext->UpdateSubresource(m_texture.Get(), 0, NULL, m_targaData, rowPitch, 0);

	// for mipmap
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;

	// Setup the shader resource view description.
	srvDesc.Format = textureDesc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	// Create the shader resource view for the texture.
	hResult = device->CreateShaderResourceView(m_texture.Get(), &srvDesc, m_textureView.GetAddressOf());
	if (FAILED(hResult)) return false;

	// Generate mipmaps for this texture.
	deviceContext->GenerateMips(m_textureView.Get());

	// Release the targa image data now that the image data has been loaded into the texture.
	delete[] m_targaData;
	m_targaData = nullptr;

	return true;
}

void TextureClass::Shutdown()
{
	// Release the targa data.
	if (m_targaData)
	{
		delete[] m_targaData;
		m_targaData = 0;
	}
}

ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return m_textureView.Get();
}

int TextureClass::GetWidth()
{
	return m_width;
}

int TextureClass::GetHeight()
{
	return m_height;
}

bool TextureClass::LoadTarga32Bit(const char* filename)
{
	int error, count, bpp;
	FILE* file;
	TargaHeader header;

	error = fopen_s(&file, filename, "rb");
	if (error != 0) return false;

	count = (int)fread(&header, sizeof(header), 1, file);
	if (count != 1) return false;

	// Get the important information from the header.
	m_height = (int)header.height;
	m_width = (int)header.width;
	bpp = (int)header.bpp;

	// It must be 32bit, not 24bit.
	if (bpp != 32)
	{
		fclose(file);
		return false;
	}

	// 4Byte(32bit) * pixels
	int imageSize = m_width * m_height * 4;

	unsigned char* targaImage = new unsigned char[imageSize];
	count = (unsigned int)fread(targaImage, 1, imageSize, file);
	if (count != imageSize) return false;

	m_targaData = new unsigned char[imageSize];

	int index = 0;
	int k = (m_width * m_height * 4) - (m_width * 4);
	// Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down and also is not in RGBA order.
	for (int j = 0; j < m_height; j++)
	{
		for (int i = 0; i < m_width; i++)
		{
			m_targaData[index + 0] = targaImage[k + 2];  // Red.
			m_targaData[index + 1] = targaImage[k + 1];  // Green.
			m_targaData[index + 2] = targaImage[k + 0];  // Blue
			m_targaData[index + 3] = targaImage[k + 3];  // Alpha
			// Increment the indexes into the targa data.
			k += 4;
			index += 4;
		}

		// Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
		k -= (m_width * 8);
	}

	// Release the targa image data now that it was copied into the destination array.
	delete[] targaImage;
	targaImage = nullptr;

	return true;
}


bool TextureClass::LoadTarga24Bit(const char* filename)
{
	int error, count, bpp;
	FILE* file;
	TargaHeader header;

	error = fopen_s(&file, filename, "rb");
	if (error != 0) return false;

	count = (int)fread(&header, sizeof(header), 1, file);
	if (count != 1) return false;

	// Get the important information from the header.
	m_height = (int)header.height;
	m_width = (int)header.width;
	bpp = (int)header.bpp;

	// It must be 32bit, not 24bit.
	if (bpp != 24) return false;

	// 4Byte(32bit) * pixels
	int imageSize = m_width * m_height * 3;
	int finalSize = m_width * m_height * 4;

	unsigned char* targaImage = new unsigned char[imageSize];
	count = (unsigned int)fread(targaImage, 1, imageSize, file);
	if (count != imageSize) return false;

	m_targaData = new unsigned char[finalSize];

	int index = 0;
	int k = (m_width * m_height * 3) - (m_width * 3);
	// Now copy the targa image data into the targa destination array in the correct order since the targa format is stored upside down and also is not in RGBA order.
	for (int j = 0; j < m_height; j++)
	{
		for (int i = 0; i < m_width; i++)
		{
			m_targaData[index + 0] = targaImage[k + 2];  // Red.
			m_targaData[index + 1] = targaImage[k + 1];  // Green.
			m_targaData[index + 2] = targaImage[k + 0];  // Blue
			m_targaData[index + 3] = 255;  // Alpha
			// Increment the indexes into the targa data.
			k += 3;
			index += 4;
		}

		// Set the targa image data index back to the preceding row at the beginning of the column since its reading it in upside down.
		k -= (m_width * 6);
	}

	// Release the targa image data now that it was copied into the destination array.
	delete[] targaImage;
	targaImage = nullptr;

	return true;
}