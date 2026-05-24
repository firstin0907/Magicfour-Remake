

cbuffer MatrixBuffer : register(b0)
{
	matrix mvpMatrix;
	matrix worldTrInvMatrix;
};


cbuffer BoneBuffer : register(b2)
{
    matrix boneTransforms[100];
};


struct VertexInputType
{
    float4 position  : POSITION;
    float2 tex   : TEXCOORD;
    float3 normal : NORMAL;
    int4 boneIDs : BONEIDS;
    float4 weights : WEIGHTS;
};

struct PixelInputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
	float3 normal : NORMAL;
};

PixelInputType vsMain(VertexInputType input)
{
    PixelInputType output;

	// Change the position vector to be 4 units for proper matrix calculations.
    input.position.w = 1.0f;

    matrix skinMatrix =
        input.weights.x * boneTransforms[input.boneIDs.x] +
        input.weights.y * boneTransforms[input.boneIDs.y] +
        input.weights.z * boneTransforms[input.boneIDs.z] +
        input.weights.w * boneTransforms[input.boneIDs.w];
    output.position = mul(input.position, skinMatrix);

	// Calculate the position of the vertex against the world, view, and projection matrices.
    output.position = mul(output.position, mvpMatrix);    

	// Store the texture coordinates for the pixel shader.
	output.tex = input.tex;
    
	// Calculate the normal vector against the world matrix only.
    output.normal = mul(input.normal, worldTrInvMatrix);
	
    // Normalize the normal vector.
    output.normal = normalize(output.normal);

    return output;
}