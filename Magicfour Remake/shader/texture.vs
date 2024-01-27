cbuffer MatrixBuffer
{
    matrix M;
    matrix V;
    matrix P;
};

struct VertexInputType
{
    float4 position : POSITION;
    float2 tex : TEXCOORD0;
};

struct PixelInputType
{
    float4 position : SV_Position;
    float2 tex : TEXCOORD0;
};

PixelInputType TextureVertexShader(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;
 
    output.position = mul(input.position, M);
    output.position = mul(output.position, V);
    output.position = mul(output.position, P);
    output.tex = input.tex;
	
    return output;
}