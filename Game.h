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
#include "PostProcess.h"


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
    /// <summary>
    /// Do a render pass to a render texture of the scene. In this case the only rendered object is the terrain
    /// </summary>
    /// <param name="rt"></param>
    void RenderSceneToTexture(RenderTexture* rt);
    void SetBloomPostProcess(float intensity, float cutoff);
    void Clear();
    void CreateDeviceDependentResources();
    void CreateWindowSizeDependentResources();
	void SetupGUI();
    bool CheckSphereCollision();
    bool SphereWithTriangle(DirectX::SimpleMath::Vector3 A, DirectX::SimpleMath::Vector3 B, DirectX::SimpleMath::Vector3 C, DirectX::SimpleMath::Vector3 center, float radius);
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
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_textureSkybox;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>                        m_textureShadow;

	//Shaders
    TerrainShader															m_TerrainShader;
    BasicShader															    m_BallShader;
    BasicShader                                                             m_colorShader;

	//Scene. 
    Terrain::VolcanoType                                                    m_volcano;
	Terrain																	m_Terrain;
	ModelClass																m_ball;
    ModelClass																m_debugCube;

	//RenderTextures
    RenderTexture*                                                          m_NormalRenderPass;
	RenderTexture*															m_PostProcessRenderPass;
    RenderTexture*                                                          m_PostProcessRenderPass2;
	RECT																	m_fullscreenRect;
	RECT																	m_CameraViewRect;

    float                                                                   m_terrainDisplacementX;
    float                                                                   m_terrainDisplacementY;
    float                                                                   terrainSide = 128;
    float                                                                   terrainScale = 20;

    BallPhysics                                                             ballMovement;
    float                                                                   LaunchForce;
    float*                                                                  ForcePtr;
    float                                                                   MinLaunchForce = 2500;
    float                                                                   MaxLaunchForce = 25000;
    bool                                                                    isKinematic = false;
    float                                                                   ballTimer;
    float                                                                   ballMaxTime = 7;
    float                                                                   ballScale = 20;
    DirectX::SimpleMath::Vector3                                            collisionPoint;
    DirectX::SimpleMath::Vector3                                            collisionNormal;
    DirectX::SimpleMath::Vector3                                            ThrowPos;

    int                                                                     Score = 0;
    bool                                                                    IsChanging = false;
    float                                                                   TimeToChange = 3;
    float                                                                   Timer;
    int                                                                     Rounds = 1;
    int                                                                     MaxRounds = 5;
    bool                                                                    GameFinished = false;
    bool*                                                                   Bloom;
    float                                                                   debugFloat;


    float                                                                   cubeScale;
    DirectX::SimpleMath::Vector3                                            cubePos;
	

    std::unique_ptr<BasicPostProcess>                                       postProcess;
    std::unique_ptr<DualPostProcess>                                        dualPostProcess;

    std::unique_ptr<DirectX::AudioEngine>                                   m_audEngine;

    std::unique_ptr<DirectX::SoundEffect> m_bounce;
    std::unique_ptr<DirectX::SoundEffect> m_score;
    std::unique_ptr<DirectX::SoundEffect> m_throw;
    std::unique_ptr<DirectX::SoundEffect> m_background;
    std::unique_ptr<DirectX::SoundEffectInstance> m_bounceInstance;
    std::unique_ptr<DirectX::SoundEffectInstance> m_scoreInstance;
    std::unique_ptr<DirectX::SoundEffectInstance> m_throwInstance;
    std::unique_ptr<DirectX::SoundEffectInstance> m_backgroundInstance;


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