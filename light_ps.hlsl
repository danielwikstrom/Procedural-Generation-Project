// Light pixel shader
// Calculate diffuse lighting for a single directional light(also texturing)

Texture2D shaderTexture : register(t0);
SamplerState SampleType : register(s0);


cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
    float4 diffuseColor;
    float3 lightPosition;
    float padding;
};

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
	float3 position3D : TEXCOORD2;
};



float4 main(InputType input) : SV_TARGET
{
	float4	textureColor;
    float3	lightDir;
    float	lightIntensity;
    float4	color;

	// Invert the light direction for calculations.
	lightDir = normalize(input.position3D - lightPosition);

	// Calculate the amount of light on this pixel.
	lightIntensity = saturate(dot(input.normal, -lightDir));

	// Determine the final amount of diffuse color based on the diffuse color combined with the light intensity.
	color = ambientColor + (diffuseColor * lightIntensity); //adding ambient
	color = saturate(color);

	// Sample the pixel color from the texture using the sampler at this texture coordinate location.
	// textureColor = shaderTexture.Sample(SampleType, input.tex);

	float4 SnowColor = float4(1.0, 1.0, 1.0, 1.0);
	float4 RockColor = float4(0.5, 0.5, 0.5, 1.0);
	float4 GrassColor = float4(0.2, 0.5, 0.1, 1.0);
	float4 SandColor = float4(0.9, 0.7, 0.2, 1.0);
	float4 heightColors[] = { SandColor, GrassColor, RockColor, SnowColor, SnowColor };

	float minY = -1.5;
	float maxY = 1.5;
	float normalizedHeight = saturate((input.position3D.y - minY) / (maxY - minY));
	//
	//float init = normalizedHeight <= 0.25f ? normalizedHeight : 0.25f;
	//init = init / 0.25f;
	//float4 first = lerp(heightColors[0], heightColors[1], init);

	float colorCount = floor(normalizedHeight * 4);

	float lerpingValue = (normalizedHeight * 4) - floor(normalizedHeight * 4);

	float4 finalColor = lerp(heightColors[colorCount], heightColors[colorCount + 1], lerpingValue);



	color = color * finalColor;

    return color;
}

