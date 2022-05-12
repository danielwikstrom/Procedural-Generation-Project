//
// Game.h
//
#pragma once

#include "DeviceResources.h"
#include "StepTimer.h"
#include "TerrainShader.h"
#include "BasicShader.h"
#include "modelclass.h"
#include "Light.h"
#include "Input.h"
#include "Camera.h"
#include "RenderTexture.h"
#include "Terrain.h"


// A basic game implementation that creates a D3D11 device and
// provides a game loop.
class Game final : public DX::IDeviceNotify
{
public:

    Game() noexcept(false);
    ~Game();

    // Initialization and management
    void Initialize(HWND window, int width, int height);

    // Basic game loop
    void Tick();

    // IDeviceNotify
    virtual void OnDeviceLost() override;
    virtual void OnDeviceRestored() override;

    // Messages
    void OnActivated();
    void OnDeactivated();
    void OnSuspending();
    void OnResuming();
    void OnWindowMoved();
    void OnWindowSizeChanged(int width, int height);
#ifdef DXTK_AUDIO
    void NewAudioDevice();
#endif

    // Properties
    void GetDefaultSize( int& width, int& height ) const;
	
private:

	struct MatrixBufferType
	{
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
	}; 

    struct BallPhysics
    {
        DirectX::SimpleMath::Vector3 CurrentPosiiton;
        DirectX::SimpleMath::Vector3 CurrentVelocity;
        float mass;
    };

    void Update(DX::StepTimer const& timer);
    void Render();
    void Clear();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
	void SetupGUI();
    bool CheckSphereCollision(DirectX::SimpleMath::Vector3 outCollisionPoint);
    bool SphereWithTriangle(DirectX::SimpleMath::Vector3 A, DirectX::SimpleMath::Vector3 B, DirectX::SimpleMath::Vector3 C, DirectX::SimpleMath::Vector3 center, float radius, DirectX::SimpleMath::Vector3 collisionPoint);
    DirectX::SimpleMath::Vector3 ClosestPoint(DirectX::SimpleMath::Vector3 A, DirectX::SimpleMath::Vector3 B, DirectX::SimpleMath::Vector3 C, DirectX::SimpleMath::Vector3 point);
    bool PointInTriangle(DirectX::SimpleMath::Vector3 point, DirectX::SimpleMath::Vector3 A, DirectX::SimpleMath::Vector3 B, DirectX::SimpleMath::Vector3 C);

    // Device resources.
    std::unique_ptr<DX::DeviceResources>    m_deviceResources;

    // Rendering loop timer.
    DX::StepTimer                           m_timer;

	//input manager. 
	Input									m_input;
	InputCommands							m_gameInputCommands;

    // DirectXTK objects.
    std::unique_ptr<DirectX::CommonStates>                                  m_states;
    std::unique_ptr<DirectX::BasicEffect>                                   m_batchEffect;	
    std::unique_ptr<DirectX::EffectFactory>                                 m_fxFactory;
    std::unique_ptr<DirectX::SpriteBatch>                                   m_sprites;
    std::unique_ptr<DirectX::SpriteFont>                                    m_font;

	// Scene Objects
	std::unique_ptr<DirectX::PrimitiveBatch<DirectX::VertexPositionColor>>  m_batch;
	Microsoft::WRL::ComPtr<ID3D11InputLayout>                               m_batchInputLayout;
	std::unique_ptr<DirectX::GeometricPrimitive>                            m_testmodel;

	//lights
	Light																	m_Light;

	//Cameras
	Camera																	m_Camera01;

	//textures 
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture1;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_texture2;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_textureSnow;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_textureRock;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_textureDirt;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_textureGrass;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_textureSand;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_textureBall;

	//Shaders
    TerrainShader															m_TerrainShader;
    BasicShader															    m_BallShader;

	//Scene. 
    Terrain::VolcanoType                                                    m_volcano;
	Terrain																	m_Terrain;
	ModelClass																m_ball;
    ModelClass																m_debugCube;

	//RenderTextures
	RenderTexture*															m_FirstRenderPass;
	RECT																	m_fullscreenRect;
	RECT																	m_CameraViewRect;

    float                                                                   m_terrainDisplacementX;
    float                                                                   m_terrainDisplacementY;
    float                                                                   terrainSide = 128;
    float                                                                   terrainScale = 20;

    BallPhysics                                                             ballMovement;
    float                                                                   LaunchForce;
    float                                                                   MinLaunchForce = 10000;
    float                                                                   MaxLaunchForce = 30000;
    bool                                                                    isKinematic = false;
    float                                                                   ballTimer;
    float                                                                   ballMaxTime = 20;
    float                                                                   ballScale = 20;


    float                                                                   cubeScale;
    DirectX::SimpleMath::Vector3                                            cubePos;
	


#ifdef DXTK_AUDIO
    std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;
    std::unique_ptr<DirectX::WaveBank>                                      m_waveBank;
    std::unique_ptr<DirectX::SoundEffect>                                   m_soundEffect;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect1;
    std::unique_ptr<DirectX::SoundEffectInstance>                           m_effect2;
#endif
    

#ifdef DXTK_AUDIO
    uint32_t                                                                m_audioEvent;
    float                                                                   m_audioTimerAcc;

    bool                                                                    m_retryDefault;
#endif

    DirectX::SimpleMath::Matrix                                             m_world;
    DirectX::SimpleMath::Matrix                                             m_view;
    DirectX::SimpleMath::Matrix                                             m_projection;
};