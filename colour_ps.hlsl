// Colour pixel/fragment shader
// Basic fragment shader outputting a colour


Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);

struct InputType
{
	float4 position : SV_POSITION;
	float2 tex : TEXCOORD0;
	//float3 normal : NORMAL;
	float4 colour : COLOR;
};


float4 main(InputType input) : SV_TARGET
{
	float2 uv = input.tex;
	uv *= 1;
	float4 tex = shaderTexture.Sample(SampleType, uv) * 1;
	float4 color = float4(1.0, 1.0, 1.0, 1.0) * tex;


	return color;
}