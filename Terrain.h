#pragma once

using namespace DirectX;
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <algorithm>
#include "PerlinNoise.h"

class Terrain
{
private:
	struct VertexType
	{
		DirectX::SimpleMath::Vector3 position;
		DirectX::SimpleMath::Vector2 texture;
		DirectX::SimpleMath::Vector3 normal;
	};
	struct HeightMapType
	{
		float x, y, z;
		float nx, ny, nz;
		float u, v;
	};
public:
	Terrain();
	~Terrain();

	bool Initialize(ID3D11Device*, int terrainWidth, int terrainHeight);
	void Render(ID3D11DeviceContext*);
	bool SmoothHeightMap(ID3D11Device*);
	bool GenerateHeightMap(ID3D11Device*);
	void Volcanize(int x, int y, float radius, float depth);
	float DistanceBetween2DPoints(float p1X, float p1Y, float p2X, float p2Y);
	bool Update();
	float* GetWavelength();
	float* GetAmplitude();
	float GetWavelengthValue();
	float GetAmplitudeValue();

private:
	bool CalculateNormals();
	void Shutdown();
	void ShutdownBuffers();
	bool InitializeBuffers(ID3D11Device*);
	void RenderBuffers(ID3D11DeviceContext*);
	

private:
	bool m_terrainGeneratedToggle;
	int m_terrainWidth, m_terrainHeight;
	ID3D11Buffer * m_vertexBuffer, *m_indexBuffer;
	int m_vertexCount, m_indexCount;
	float m_frequency, m_amplitude, m_wavelength;
	HeightMapType* m_heightMap;
	float* m_randomMap;

	//arrays for our generated objects Made by directX
	std::vector<VertexPositionNormalTexture> preFabVertices;
	std::vector<uint16_t> preFabIndices;
};

