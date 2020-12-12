
cbuffer vertexBuffer : register(b0)
{
	float4x4 ProjectionMatrix;
	float4x4 ViewMatrix;
	float4x4 WorldMatrix;
	float4   TintColor;
};

struct VS_INPUT
{
	float2 pos : POSITION;
	float2 uv  : TEXCOORD0;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	output.pos = mul(WorldMatrix, float4(input.pos.xy, 0.f, 1.f));
	output.pos = mul(ViewMatrix, output.pos);
	output.pos = mul(ProjectionMatrix, output.pos);
	output.uv  = input.uv;
	return output;
}
