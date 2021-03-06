#include "pch.h"
#include "Terrain.h"


Terrain::Terrain()
{
	m_terrainGeneratedToggle = false;
}


Terrain::~Terrain()
{
}

bool Terrain::Initialize(ID3D11Device* device, int terrainWidth, int terrainHeight)
{
	int index;
	float height = 0.0;
	bool result;
	
	// Save the dimensions of the terrain.
	m_terrainWidth = terrainWidth;
	m_terrainHeight = terrainHeight;

	m_frequency = m_terrainWidth / 20;
	m_amplitude = 0;
	m_wavelength = 0;

	// Create the structure to hold the terrain data.
	m_heightMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if (!m_heightMap)
	{
		return false;
	}

	m_heightMapToLerp = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if (!m_heightMapToLerp)
	{
		return false;
	}

	m_initMap = new HeightMapType[m_terrainWidth * m_terrainHeight];
	if (!m_initMap)
	{
		return false;
	}

	m_randomMap = new float[m_terrainWidth * m_terrainHeight];

	VolcanoInfo = VolcanoType();
	VolcanoInfo.center = DirectX::SimpleMath::Vector2(0, 0);
	VolcanoInfo.radius = 10;

	//this is how we calculate the texture coordinates first calculate the step size there will be between vertices. 
	float textureCoordinatesStep = 5.0f / m_terrainWidth;  //tile 5 times across the terrain. 
	// Initialise the data in the height map (flat).
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			m_heightMap[index].y = (float)height;
			m_heightMap[index].z = (float)j;

			//and use this step to calculate the texture coordinates for this point on the terrain.
			m_heightMap[index].u = (float)i * textureCoordinatesStep;
			m_heightMap[index].v = (float)j * textureCoordinatesStep;

		}
	}

	//even though we are generating a flat terrain, we still need to normalise it. 
	// Calculate the normals for the terrain data.
	result = CalculateNormals();
	if (!result)
	{
		return false;
	}

	// Initialize the vertex and index buffer that hold the geometry for the terrain.
	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}


	return true;
}

void Terrain::Render(ID3D11DeviceContext* deviceContext)
{
	// Put the vertex and index buffers on the graphics pipeline to prepare them for drawing.
	RenderBuffers(deviceContext);
	deviceContext->DrawIndexed(m_indexCount, 0, 0);

	return;
}

bool Terrain::CalculateNormals()
{
	int i, j, index1, index2, index3, index, count;
	float vertex1[3], vertex2[3], vertex3[3], vector1[3], vector2[3], sum[3], length;
	DirectX::SimpleMath::Vector3* normals;


	// Create a temporary array to hold the un-normalized normal vectors.
	normals = new DirectX::SimpleMath::Vector3[(m_terrainHeight - 1) * (m_terrainWidth - 1)];
	if (!normals)
	{
		return false;
	}

	// Go through all the faces in the mesh and calculate their normals.
	for (j = 0; j < (m_terrainHeight - 1); j++)
	{
		for (i = 0; i < (m_terrainWidth - 1); i++)
		{
			index1 = (j * m_terrainHeight) + i;
			index2 = (j * m_terrainHeight) + (i + 1);
			index3 = ((j + 1) * m_terrainHeight) + i;

			// Get three vertices from the face.
			vertex1[0] = m_heightMap[index1].x;
			vertex1[1] = m_heightMap[index1].y;
			vertex1[2] = m_heightMap[index1].z;

			vertex2[0] = m_heightMap[index2].x;
			vertex2[1] = m_heightMap[index2].y;
			vertex2[2] = m_heightMap[index2].z;

			vertex3[0] = m_heightMap[index3].x;
			vertex3[1] = m_heightMap[index3].y;
			vertex3[2] = m_heightMap[index3].z;

			// Calculate the two vectors for this face.
			vector1[0] = vertex1[0] - vertex3[0];
			vector1[1] = vertex1[1] - vertex3[1];
			vector1[2] = vertex1[2] - vertex3[2];
			vector2[0] = vertex3[0] - vertex2[0];
			vector2[1] = vertex3[1] - vertex2[1];
			vector2[2] = vertex3[2] - vertex2[2];

			index = (j * (m_terrainHeight - 1)) + i;

			// Calculate the cross product of those two vectors to get the un-normalized value for this face normal.
			normals[index].x = (vector1[1] * vector2[2]) - (vector1[2] * vector2[1]);
			normals[index].y = (vector1[2] * vector2[0]) - (vector1[0] * vector2[2]);
			normals[index].z = (vector1[0] * vector2[1]) - (vector1[1] * vector2[0]);
		}
	}

	// Now go through all the vertices and take an average of each face normal 	
	// that the vertex touches to get the averaged normal for that vertex.
	for (j = 0; j < m_terrainHeight; j++)
	{
		for (i = 0; i < m_terrainWidth; i++)
		{
			// Initialize the sum.
			sum[0] = 0.0f;
			sum[1] = 0.0f;
			sum[2] = 0.0f;

			// Initialize the count.
			count = 0;

			// Bottom left face.
			if (((i - 1) >= 0) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (m_terrainHeight - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Bottom right face.
			if ((i < (m_terrainWidth - 1)) && ((j - 1) >= 0))
			{
				index = ((j - 1) * (m_terrainHeight - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper left face.
			if (((i - 1) >= 0) && (j < (m_terrainHeight - 1)))
			{
				index = (j * (m_terrainHeight - 1)) + (i - 1);

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Upper right face.
			if ((i < (m_terrainWidth - 1)) && (j < (m_terrainHeight - 1)))
			{
				index = (j * (m_terrainHeight - 1)) + i;

				sum[0] += normals[index].x;
				sum[1] += normals[index].y;
				sum[2] += normals[index].z;
				count++;
			}

			// Take the average of the faces touching this vertex.
			sum[0] = (sum[0] / (float)count);
			sum[1] = (sum[1] / (float)count);
			sum[2] = (sum[2] / (float)count);

			// Calculate the length of this normal.
			length = sqrt((sum[0] * sum[0]) + (sum[1] * sum[1]) + (sum[2] * sum[2]));

			// Get an index to the vertex location in the height map array.
			index = (j * m_terrainHeight) + i;

			// Normalize the final shared normal for this vertex and store it in the height map array.
			m_heightMap[index].nx = (sum[0] / length);
			m_heightMap[index].ny = (sum[1] / length);
			m_heightMap[index].nz = (sum[2] / length);
		}
	}

	// Release the temporary normals.
	delete[] normals;
	normals = 0;

	return true;
}

void Terrain::Shutdown()
{
	// Release the index buffer.
	if (m_indexBuffer)
	{
		m_indexBuffer->Release();
		m_indexBuffer = 0;
	}

	// Release the vertex buffer.
	if (m_vertexBuffer)
	{
		m_vertexBuffer->Release();
		m_vertexBuffer = 0;
	}

	return;
}

bool Terrain::InitializeBuffers(ID3D11Device* device)
{
	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int index, i, j;
	int index1, index2, index3, index4, index5, index6; //geometric indices. 

	// Calculate the number of vertices in the terrain mesh.
	m_vertexCount = (m_terrainWidth - 1) * (m_terrainHeight - 1) * 6;

	// Set the index count to the same as the vertex count.
	m_indexCount = m_vertexCount;

	// Create the vertex array.
	vertices = new VertexType[m_vertexCount];
	if (!vertices)
	{
		return false;
	}

	// Create the index array.

	indices = new unsigned long[m_indexCount];
	if (!indices)
	{
		return false;
	}

	// Initialize the index to the vertex buffer.
	index = 0;

	for (j = 0; j < (m_terrainHeight - 1); j++)
	{
		for (i = 0; i < (m_terrainWidth - 1); i++)
		{
			if ((j % 2 == 0 && i % 2 == 0) || (j%2 == 1 && i%2 == 1) )
			{
				index1 = (m_terrainHeight * (j + 1)) + i;      // Upper left.
				index2 = (m_terrainHeight * (j + 1)) + (i + 1);  // Upper right.
				index3 = (m_terrainHeight * j) + i;          // Bottom left.
				index4 = (m_terrainHeight * j) + i;          // Bottom left.
				index5 = (m_terrainHeight * (j + 1)) + (i + 1);  // Upper right.
				index6 = (m_terrainHeight * j) + (i + 1);      // Bottom right.
			}
			else 
			{
				index1 = (m_terrainHeight * (j + 1)) + i;      // Upper left.
				index2 = (m_terrainHeight * j) + (i + 1);      // Bottom right.
				index3 = (m_terrainHeight * j) + i;          // Bottom left.
				index4 = (m_terrainHeight * (j + 1)) + i;      // Upper left.
				index5 = (m_terrainHeight * (j + 1)) + (i + 1);  // Upper right.
				index6 = (m_terrainHeight * j) + (i + 1);      // Bottom right.
			}

			// Upper left.
			vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index1].x, m_heightMap[index1].y, m_heightMap[index1].z);
			vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index1].nx, m_heightMap[index1].ny, m_heightMap[index1].nz);
			vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index1].u, m_heightMap[index1].v);
			indices[index] = index;
			index++;

			// Upper right.
			vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index2].x, m_heightMap[index2].y, m_heightMap[index2].z);
			vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index2].nx, m_heightMap[index2].ny, m_heightMap[index2].nz);
			vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index2].u, m_heightMap[index2].v);
			indices[index] = index;
			index++;

			// Bottom left.
			vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index3].x, m_heightMap[index3].y, m_heightMap[index3].z);
			vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index3].nx, m_heightMap[index3].ny, m_heightMap[index3].nz);
			vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index3].u, m_heightMap[index3].v);
			indices[index] = index;
			index++;

			// Bottom left.
			vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index4].x, m_heightMap[index4].y, m_heightMap[index4].z);
			vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index4].nx, m_heightMap[index4].ny, m_heightMap[index4].nz);
			vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index4].u, m_heightMap[index4].v);
			indices[index] = index;
			index++;

			// Upper right.
			vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index5].x, m_heightMap[index5].y, m_heightMap[index5].z);
			vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index5].nx, m_heightMap[index5].ny, m_heightMap[index5].nz);
			vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index5].u, m_heightMap[index5].v);
			indices[index] = index;
			index++;

			// Bottom right.
			vertices[index].position = DirectX::SimpleMath::Vector3(m_heightMap[index6].x, m_heightMap[index6].y, m_heightMap[index6].z);
			vertices[index].normal = DirectX::SimpleMath::Vector3(m_heightMap[index6].nx, m_heightMap[index6].ny, m_heightMap[index6].nz);
			vertices[index].texture = DirectX::SimpleMath::Vector2(m_heightMap[index6].u, m_heightMap[index6].v);
			indices[index] = index;
			index++;
		}
	}


	// Set up the description of the static vertex buffer.
	vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	vertexBufferDesc.ByteWidth = sizeof(VertexType) * m_vertexCount;
	vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vertexBufferDesc.CPUAccessFlags = 0;
	vertexBufferDesc.MiscFlags = 0;
	vertexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the vertex data.
	vertexData.pSysMem = vertices;
	vertexData.SysMemPitch = 0;
	vertexData.SysMemSlicePitch = 0;

	// Now create the vertex buffer.
	result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_vertexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Set up the description of the static index buffer.
	indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	indexBufferDesc.ByteWidth = sizeof(unsigned long) * m_indexCount;
	indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	indexBufferDesc.CPUAccessFlags = 0;
	indexBufferDesc.MiscFlags = 0;
	indexBufferDesc.StructureByteStride = 0;

	// Give the subresource structure a pointer to the index data.
	indexData.pSysMem = indices;
	indexData.SysMemPitch = 0;
	indexData.SysMemSlicePitch = 0;

	// Create the index buffer.
	result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_indexBuffer);
	if (FAILED(result))
	{
		return false;
	}

	// Release the arrays now that the vertex and index buffers have been created and loaded.
	delete[] vertices;
	vertices = 0;

	delete[] indices;
	indices = 0;


	return true;
}

void Terrain::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;

	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_vertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_indexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}

float* GetRandomArray(float min, float max, int size)
{
	float* matrix = new float[size];
	for (int i = 0; i < size; i++)
	{
		float random = min + static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / (max - min)));
		matrix[i] = random;
	}
	return matrix;
}



bool Terrain::SmoothHeightMap(ID3D11Device* device)
{
	srand(static_cast <unsigned> (time(0)));
	bool result;

	int index;
	float height = 0.0;

	//loop through the terrain and set the hieghts how we want. This is where we generate the terrain
	//in this case I will run a sin-wave through the terrain in one axis.
	float* smoothMap = new float[m_terrainHeight * m_terrainWidth];
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			float value = 0.0f;
			int numNeighbours = 0;
			if (i > 0)
			{
				value += m_randomMap[j * m_terrainHeight + i - 1];
				numNeighbours++;
			}
			if (i < m_terrainWidth - 1)
			{
				value += m_randomMap[j * m_terrainHeight + i + 1];
				numNeighbours++;
			}
			if (j > 0)
			{
				value += m_randomMap[(j - 1) * m_terrainHeight + i];
				numNeighbours++;
			}
			if (j < m_terrainHeight - 1)
			{
				value += m_randomMap[(j + 1) * m_terrainHeight + i];
				numNeighbours++;
			}
			value /= numNeighbours;

			smoothMap[j * m_terrainHeight + i] = (m_randomMap[j * m_terrainHeight + i] + value) / 2;

			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			m_heightMap[index].y = smoothMap[j * m_terrainHeight + i];
			m_heightMap[index].z = (float)j;
		}
	}
	m_randomMap = smoothMap;

	result = CalculateNormals();
	if (!result)
	{
		return false;
	}

	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
}


bool Terrain::GenerateHeightMap(ID3D11Device* device)
{
	PerlinNoise noise = PerlinNoise();
	bool result;

	int index;
	float height = 0.0;


	float perlinMultiplier = 0.05;
	float heightMultiplier = 30;
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			m_heightMap[index].x = (float)i;
			float perlini = (i * perlinMultiplier + m_wavelength * 10);
			float perlinj = (j * perlinMultiplier + m_amplitude * 10);
			float perlinVal = noise.noise(perlinj, perlini, 0);
			//perlinVal = abs(1-perlinVal) * 2 - 1;
			perlinVal *= heightMultiplier;
			m_heightMap[index].y = perlinVal;
			m_heightMap[index].z = (float)j;

		}
	}



	// Volcano spawns only in interior 60% of the map
	this->Volcanize(this->GetHighestPeak(m_terrainHeight * 0.2, m_terrainHeight * 0.8, m_terrainWidth * 0.2, m_terrainWidth * 0.8, m_heightMap), volcanoRadius, volcanoDepth, volcanoMountainRadius, volcanHeightMultiplier, m_heightMap);

	result = CalculateNormals();
	if (!result)
	{
		return false;
	}

	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
}

bool Terrain::ChangeHeightMap(ID3D11Device* device, float timeStep)
{
	PerlinNoise noise = PerlinNoise();
	srand(static_cast <unsigned> (time(0)));
	bool result;

	

	int index;
	float height = 0.0;

	if (timeStep >= 0.99f)
		timeStep = 1;


	float perlinMultiplier = 0.05;
	float heightMultiplier = 30;
	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;
			m_heightMap[index].y = flerp(m_initMap[index].y, m_heightMapToLerp[index].y, timeStep);
		}
	}

	VolcanoInfo.center.x = flerp(startVolcanoCenter.x, goalVolcanoCenter.x, timeStep);
	VolcanoInfo.center.y = flerp(startVolcanoCenter.y, goalVolcanoCenter.y, timeStep);

	



	result = CalculateNormals();
	if (!result)
	{
		return false;
	}

	result = InitializeBuffers(device);
	if (!result)
	{
		return false;
	}
}

float Terrain::flerp(float a, float b, float t)
{
	return a + (b - a) * t; 
}

bool Terrain::GetFinalHeightMap(ID3D11Device*, float displacement)
{
	PerlinNoise noise = PerlinNoise();
	bool result;


	int index;
	float height = 0.0;
	float perlinMultiplier = 0.05;
	float heightMultiplier = 30;

	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			index = (m_terrainHeight * j) + i;

			m_heightMapToLerp[index].x = (float)i;
			float perlini = (i * perlinMultiplier + displacement * 10);
			float perlinj = (j * perlinMultiplier + displacement * 10);
			float perlinVal = noise.noise(perlinj, perlini, 0);
			perlinVal *= heightMultiplier;
			m_heightMapToLerp[index].y = perlinVal;
			m_heightMapToLerp[index].z = (float)j;


			m_initMap[index].x = m_heightMap[index].x;
			m_initMap[index].y = m_heightMap[index].y;
			m_initMap[index].z = m_heightMap[index].z;
		}
	}

	startVolcanoCenter = VolcanoInfo.center;
	this->Volcanize(this->GetHighestPeak(m_terrainHeight * 0.2, m_terrainHeight * 0.8, m_terrainWidth * 0.2, m_terrainWidth * 0.8, m_heightMapToLerp), volcanoRadius, volcanoDepth, volcanoMountainRadius, volcanHeightMultiplier, m_heightMapToLerp);

	goalVolcanoCenter = VolcanoInfo.center;

	return false;
}

DirectX::SimpleMath::Vector2 Terrain::GetHighestPeak(int startPosX, int endPosX, int startPosZ, int endPosZ, HeightMapType* map)
{
	DirectX::SimpleMath::Vector2 highestPeakPos;

	int index;
	float heighestPointX = 0;
	float heighestPointY = 0;
	float heighestPointHeight = -MAXINT;

	for (int j = startPosX; j < endPosX; j++)
	{
		for (int i = startPosZ; i < endPosZ; i++)
		{
			index = (m_terrainHeight * j) + i;
			float height = map[index].y;
			//Check surrounding peaks
			if ((height > map[(m_terrainHeight * j) + i + 1].y)
				&& (height > map[(m_terrainHeight * j) + i - 1].y)
				&& (height > map[(m_terrainHeight * (j + 1)) + i].y)
				&& (height > map[(m_terrainHeight * (j - 1)) + i].y)
				&& height > heighestPointHeight)
			{
				heighestPointHeight = height;
				heighestPointX = i;
				heighestPointY = j;
			}
		}
	}

	highestPeakPos.x = heighestPointX;
	highestPeakPos.y = heighestPointY;

	return highestPeakPos;
}

void Terrain::Volcanize(DirectX::SimpleMath::Vector2 center, float radius, float depth, float mountainRadius, float mountainHeightMultiplier, HeightMapType* map)
{
	int centerIndex = (m_terrainHeight * center.y) + center.x;
	float centerX = map[centerIndex].x;
	float centerZ = map[centerIndex].z;
	float maxDepth = (map[centerIndex].y * mountainHeightMultiplier) - depth;
	float maxHeight = map[centerIndex].y * mountainHeightMultiplier;



	for (int j = 0; j < m_terrainHeight; j++)
	{
		for (int i = 0; i < m_terrainWidth; i++)
		{
			
			int index = (m_terrainHeight * j) + i;
			float pointX = map[index].x;
			float pointZ = map[index].z;
			float distance = this->DistanceBetween2DPoints(pointX, pointZ, centerX, centerZ);
			if (distance <= mountainRadius)
			{
				map[index].y = maxHeight - (sqrt(distance / mountainRadius)) * (maxHeight - map[index].y);
				//m_heightMap[index].y = maxHeight - (pow(distance / mountainRadius, 2) * (maxHeight - m_heightMap[index].y));
			}
			if (distance <= radius)
			{
				//m_heightMap[index].y = maxDepth;
				map[index].y = maxDepth + (pow(distance/radius, 16)) * (map[index].y - maxDepth);
			}
			
		}
	}

	VolcanoInfo.center = DirectX::SimpleMath::Vector2(centerX, centerZ);
	VolcanoInfo.radius = radius;
	VolcanoInfo.mountainRadius = mountainRadius;
}

float Terrain::DistanceBetween2DPoints(float p1X, float p1Y, float p2X, float p2Y)
{
	float distanceX = p2X - p1X;
	float distanceY = p2Y - p1Y;

	float distance = sqrt(pow(distanceX, 2) + pow(distanceY, 2));
	return distance;
}

bool Terrain::Update()
{
	return true;
}

float* Terrain::GetWavelength()
{
	return &m_wavelength;
}

float* Terrain::GetAmplitude()
{
	return &m_amplitude;
}

Terrain::VolcanoType* Terrain::GetVolcanoInfo()
{
	return &VolcanoInfo;
}
