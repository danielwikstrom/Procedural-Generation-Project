// Colour pixel/fragment shader
// Basic fragment shader outputting a colour

struct InputType
{
	float4 position : SV_POSITION;
	//float2 tex : TEXCOORD0;
	//float3 normal : NORMAL;
	float4 colour : COLOR;
};


float4 main(InputType input) : SV_TARGET
{
	//float4 colour = float4(1.0, 0.0, 0.0, 1.0);
//	return float4(1.0, 0.0, 1.0, 1.0);	
	float4 SnowColor = float4(1.0, 1.0, 1.0, 1.0);
	float4 RockColor = float4(0.5, 0.5, 0.5, 1.0);
	float4 SandColor = float4(0.9, 0.7, 0.2, 1.0);
	float4 GrassColor = float4(0.2, 0.5, 0.1, 1.0);

	float minY = -10.0;
	float maxY = 10.0;
	float normalizedHeight = (input.position.y - minY) / (maxY - minY);
	float4 finalColor = lerp(RockColor, SnowColor, normalizedHeight);

	return finalColor;
}