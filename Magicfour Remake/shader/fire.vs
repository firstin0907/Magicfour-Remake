////////////////////////////////////////////////////////////////////////////////
// Filename: fire.vs
////////////////////////////////////////////////////////////////////////////////


/////////////
// GLOBALS //
/////////////
cbuffer MatrixBuffer
{
    matrix mvp;
};

cbuffer NoiseBuffer
{
    float frameTime;
    float3 scrollSpeeds;
    float3 scales; // 3���� scale
    float padding;
};


//////////////
// TYPEDEFS //
//////////////
struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float2 texCoords1 : TEXCOORD1; // By doing so, can use multi-scale texture.
    float2 texCoords2 : TEXCOORD2;
    float2 texCoords3 : TEXCOORD3;
};

////////////////////////////////////////////////////////////////////////////////
// Vertex Shader
////////////////////////////////////////////////////////////////////////////////
PixelInputType vsMain(VertexInputType input)
{
    PixelInputType output;
    

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    output.position = mul(input.position, mvp);
    output.tex = input.tex;

    // ù��° ������ �ؽ����� ��ǥ�� ù��° ũ�� �� ������ ��ũ�� �ӵ� ���� �̿��Ͽ� ����մϴ�.
    output.texCoords1 = (input.tex * scales.x);
    output.texCoords1.y = output.texCoords1.y + (frameTime * scrollSpeeds.x);

    // �ι�° ������ �ؽ����� ��ǥ�� �ι�° ũ�� �� ������ ��ũ�� �ӵ� ���� �̿��Ͽ� ����մϴ�.
    output.texCoords2 = (input.tex * scales.y);
    output.texCoords2.y = output.texCoords2.y + (frameTime * scrollSpeeds.y);

    // ����° ������ �ؽ����� ��ǥ�� ����° ũ�� �� ������ ��ũ�� �ӵ� ���� �̿��Ͽ� ����մϴ�.
    output.texCoords3 = (input.tex * scales.z);
    output.texCoords3.y = output.texCoords3.y + (frameTime * scrollSpeeds.z);

    return output;
}