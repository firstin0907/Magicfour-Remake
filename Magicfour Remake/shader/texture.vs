cbuffer MatrixBuffer
{
    matrix mvpMatrix;
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

PixelInputType vsMain(VertexInputType input)
{
    PixelInputType output;
    
    input.position.w = 1.0f;
 
    output.position = mul(input.position, mvpMatrix);
    output.tex = input.tex;
	
    return output;
}