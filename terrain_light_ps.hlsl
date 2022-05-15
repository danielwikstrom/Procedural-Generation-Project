// Light pixel shader
// Calculate diffuse lighting for a single directional light(also texturing)

Texture2D terrainTexture1 : register(t0);
Texture2D terrainTexture2 : register(t1);
Texture2D terrainTexture3 : register(t2);
Texture2D terrainTexture4 : register(t3);
Texture2D terrainTexture5 : register(t4);
SamplerState SampleType : register(s0);


cbuffer LightBuffer : register(b0)
{
	float4 ambientColor;
    float4 diffuseColor;
    float3 lightPosition;
	float time;
};

cbuffer VolcanoBuffer : register(b1)
{
	float2 volcanoCenter;
	float volcanoRadius;
	float volcanoMountainRadius;
}

struct InputType
{
    float4 position : SV_POSITION;
    float2 tex : TEXCOORD0;
    float3 normal : NORMAL;
	float3 position3D : TEXCOORD2;
};


int fastFloor(float x)
{
	return x > 0 ? (int)x : (int)x - 1;
}

float dot(int g[3], float x, float y, float z)
{
	return g[0] * x + g[1] * y + g[2] * z;
}

float mix(float a, float b, float t)
{
	return (1 - t) * a + t * b;
}

float fade(float t)
{
	return t * t * t * (t * (t * 6 - 15) + 10);
}

float perlinNoise(float x, float y, float z)
{
	int grad3[12][3] =
	{ {1,1,0},{-1,1,0},{1,-1,0},{-1,-1,0},
	{1,0,1},{-1,0,1},{1,0,-1},{-1,0,-1},
	{0,1,1},{0,-1,1},{0,1,-1},{0,-1,-1} };

	int p[256] = { 151, 160, 137, 91, 90, 15,
	131, 13, 201, 95, 96, 53, 194, 233, 7, 225, 140, 36, 103, 30, 69, 142, 8, 99, 37, 240, 21, 10, 23,
	190, 6, 148, 247, 120, 234, 75, 0, 26, 197, 62, 94, 252, 219, 203, 117, 35, 11, 32, 57, 177, 33,
	88, 237, 149, 56, 87, 174, 20, 125, 136, 171, 168, 68, 175, 74, 165, 71, 134, 139, 48, 27, 166,
	77, 146, 158, 231, 83, 111, 229, 122, 60, 211, 133, 230, 220, 105, 92, 41, 55, 46, 245, 40, 244,
	102, 143, 54, 65, 25, 63, 161, 1, 216, 80, 73, 209, 76, 132, 187, 208, 89, 18, 169, 200, 196,
	135, 130, 116, 188, 159, 86, 164, 100, 109, 198, 173, 186, 3, 64, 52, 217, 226, 250, 124, 123,
	5, 202, 38, 147, 118, 126, 255, 82, 85, 212, 207, 206, 59, 227, 47, 16, 58, 17, 182, 189, 28, 42,
	223, 183, 170, 213, 119, 248, 152, 2, 44, 154, 163, 70, 221, 153, 101, 155, 167, 43, 172, 9,
	129, 22, 39, 253, 19, 98, 108, 110, 79, 113, 224, 232, 178, 185, 112, 104, 218, 246, 97, 228,
	251, 34, 242, 193, 238, 210, 144, 12, 191, 179, 162, 241, 81, 51, 145, 235, 249, 14, 239, 107,
	49, 192, 214, 31, 181, 199, 106, 157, 184, 84, 204, 176, 115, 121, 50, 45, 127, 4, 150, 254,
	138, 236, 205, 93, 222, 114, 67, 29, 24, 72, 243, 141, 128, 195, 78, 66, 215, 61, 156, 180 };

	int perm[512];
	for (int i = 0; i < 512; i++)
	{
		perm[i] = p[i & 255];
	}

	int X = fastFloor(x);
	int Y = fastFloor(y);
	int Z = fastFloor(z);

	x = x - X;
	y = y - Y;
	z = z - Z;

	X = X & 255;
	Y = Y & 255;
	Z = Z & 255;

	int gi000 = perm[X + perm[Y + perm[Z]]] % 12;
	int gi001 = perm[X + perm[Y + perm[Z + 1]]] % 12;
	int gi010 = perm[X + perm[Y + 1 + perm[Z]]] % 12;
	int gi011 = perm[X + perm[Y + 1 + perm[Z + 1]]] % 12;
	int gi100 = perm[X + 1 + perm[Y + perm[Z]]] % 12;
	int gi101 = perm[X + 1 + perm[Y + perm[Z + 1]]] % 12;
	int gi110 = perm[X + 1 + perm[Y + 1 + perm[Z]]] % 12;
	int gi111 = perm[X + 1 + perm[Y + 1 + perm[Z + 1]]] % 12;

	float n000 = dot(grad3[gi000], x, y, z);
	float n100 = dot(grad3[gi100], x - 1, y, z);
	float n010 = dot(grad3[gi010], x, y - 1, z);
	float n110 = dot(grad3[gi110], x - 1, y - 1, z);
	float n001 = dot(grad3[gi001], x, y, z - 1);
	float n101 = dot(grad3[gi101], x - 1, y, z - 1);
	float n011 = dot(grad3[gi011], x, y - 1, z - 1);
	float n111 = dot(grad3[gi111], x - 1, y - 1, z - 1);

	// Compute the fade curve value for each of x, y, z
	float u = fade(x);
	float v = fade(y);
	float w = fade(z);

	float nx00 = mix(n000, n100, u);
	float nx01 = mix(n001, n101, u);
	float nx10 = mix(n010, n110, u);
	float nx11 = mix(n011, n111, u);

	//Interpolate the four results along y
	float nxy0 = mix(nx00, nx10, v);
	float nxy1 = mix(nx01, nx11, v);

	//Interpolate the two last results along z
	float nxyz = mix(nxy0, nxy1, w);

	return nxyz;
}

float IsVolcano(float2 pos)
{
	float2 distanceVector = pos - (volcanoCenter * 20.0f);
	float distance = sqrt(pow(distanceVector.x, 2) + pow(distanceVector.y, 2));
	distance = saturate(distance / (volcanoMountainRadius * 20));


	return pow(distance,16);
}

float IsLava(float2 pos)
{
	float2 distanceVector = pos - (volcanoCenter * 20.0f);
	float distance = sqrt(pow(distanceVector.x, 2) + pow(distanceVector.y, 2));
	distance = saturate(floor(distance / (volcanoRadius * 18)));
	return distance;
}

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

	float4 SandColor = terrainTexture1.Sample(SampleType, input.tex);
	float4 GrassColor = terrainTexture2.Sample(SampleType, input.tex * 0.3f);
	float4 DirtColor = terrainTexture3.Sample(SampleType, input.tex * 3);
	float4 RockColor = terrainTexture4.Sample(SampleType, input.tex * 3);
	float4 SnowColor = terrainTexture5.Sample(SampleType, input.tex * 4);
	float4 LavaColour1 = float4(1.0, 0.1, 0.1, 1.0) ;
	float4 LavaColour2 = float4(1.0, 0.6, 0.1, 1.0) ;
	//float4 SnowColor = float4(1.0, 1.0, 1.0, 1.0);
	//float4 RockColor = float4(0.5, 0.5, 0.5, 1.0);
	//float4 DirtColor = float4(0.5, 0.25, 0.0, 1.0);
	//float4 SandColor = float4(0.9, 0.7, 0.2, 1.0);
	//float4 GrassColor = float4(0.2, 0.5, 0.1, 1.0);
	float4 heightColors[] = {GrassColor, GrassColor,DirtColor, RockColor, SnowColor, SnowColor};

	

	float minY = -300.5;
	float maxY = 300;

	float minX = 0;
	float maxX = 2560;
	float normalizedHeight = saturate(floor(input.position3D.y - minY) / (maxY - minY));


	float perlin = perlinNoise(input.position3D.x * 0.05f , 0, input.position3D.z * 0.05f );
	float perlinHeight = saturate(normalizedHeight + (perlin*0.04));
	float colToUse = floor(perlinHeight * 5);
	float colToLerp = colToUse + 1;
	colToLerp = clamp(colToLerp, 0, 4);
	float lerpingValue = (perlinHeight * 5) - colToUse;

	lerpingValue = ceil(lerpingValue * 1)/1;
	float4 finalColor = lerp(heightColors[colToUse], heightColors[colToLerp], lerpingValue);

	// Paint volcano with different texture
	float2 pos = float2(input.position3D.x, input.position3D.z);
	finalColor = lerp(RockColor * 0.5, finalColor, IsVolcano(pos)) * color;

	// Lava texture
	float frq = 0.001;
	float amp = 20;
	float speed = 0.1f;
	float lavaPerlin = perlinNoise(input.position3D.x * 0.03f, time * speed, input.position3D.z * 0.03f);
	float lavaFunction = sin(input.position3D.x * frq * (lavaPerlin)) * amp * lavaPerlin + sin(input.position3D.z * frq * (lavaPerlin)) * 2 * amp * lavaPerlin;
	lavaFunction *= 0.1;

	float4 LavaLerped = lerp(LavaColour2, LavaColour1, lavaFunction) * 2;
	float lavaLerp = saturate(ceil(1 - lavaFunction));
	float4 LavaColourFinal = lerp(RockColor * 0.1, LavaLerped, lavaLerp);
	finalColor = lerp(LavaColourFinal, finalColor, IsLava(pos));



	color = finalColor;

    return color;
}

