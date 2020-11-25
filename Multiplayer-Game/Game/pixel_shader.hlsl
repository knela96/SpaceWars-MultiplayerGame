
cbuffer pixelBuffer : register(b0)
{
	float4x4 ProjectionMatrix;
	float4x4 ViewMatrix;
	float4x4 WorldMatrix;
	float4   TintColor;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 uv  : TEXCOORD0;
};

sampler sampler0;
Texture2D texture0;

float4 main(PS_INPUT input) : SV_Target
{
	float4 out_col = texture0.Sample(sampler0, input.uv) * TintColor;
	return out_col;
}
