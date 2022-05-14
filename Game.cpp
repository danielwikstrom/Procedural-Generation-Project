//
// Game.cpp
//

#include "pch.h"
#include "Game.h"


//toreorganise
#include <fstream>

extern void ExitGame();

using namespace DirectX;
using namespace DirectX::SimpleMath;
using namespace ImGui;

using Microsoft::WRL::ComPtr;

Game::Game() noexcept(false)
{
    m_deviceResources = std::make_unique<DX::DeviceResources>();
    m_deviceResources->RegisterDeviceNotify(this);
}

Game::~Game()
{
#ifdef DXTK_AUDIO
    if (m_audEngine)
    {
        m_audEngine->Suspend();
    }
#endif
}

// Initialize the Direct3D resources required to run.
void Game::Initialize(HWND window, int width, int height)
{

	m_input.Initialise(window);

    m_deviceResources->SetWindow(window, width, height);

    m_deviceResources->CreateDeviceResources();
    CreateDeviceDependentResources();

    m_deviceResources->CreateWindowSizeDependentResources();
    CreateWindowSizeDependentResources();

	//setup imgui.  its up here cos we need the window handle too
	//pulled from imgui directx11 example
	//IMGUI_CHECKVERSION();
	//ImGui::CreateContext();
	//ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
	//ImGui::StyleColorsDark();
	//ImGui_ImplWin32_Init(window);		//tie to our window
	//ImGui_ImplDX11_Init(m_deviceResources->GetD3DDevice(), m_deviceResources->GetD3DDeviceContext());	//tie to directx

	m_fullscreenRect.left = 0;
	m_fullscreenRect.top = 0;
	m_fullscreenRect.right = 800;
	m_fullscreenRect.bottom = 600;

	m_CameraViewRect.left = 500;
	m_CameraViewRect.top = 0;
	m_CameraViewRect.right = 800;
	m_CameraViewRect.bottom = 240;

	//setup light
	m_Light.setAmbientColour(0.3f, 0.3f, 0.3f, 1.0f);
	m_Light.setDiffuseColour(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light.setPosition(2.0f, 1.0f, 1.0f);
	m_Light.setDirection(-1.0f, -1.0f, 0.0f);

	//setup camera
	m_Camera01.setPosition(Vector3(-100.0f, 1300.0f, 1280.0f));
    m_Camera01.setRotation(Vector3(0.0f, 90.0f, 120.0f));	

    ballMovement.mass = 100;
    ballMovement.CurrentPosiiton = m_Camera01.getPosition();
    ballMovement.CurrentVelocity = Vector3(0, 0, 0);
    ballTimer = ballMaxTime;
    LaunchForce = MinLaunchForce;
    collisionPoint = Vector3(0,0,0);
    collisionNormal = Vector3(0, 0, 0);

    //m_volcano = Terrain::VolcanoType();
    //m_volcano.center = DirectX::SimpleMath::Vector2(0, 0);
    //m_volcano.radius = 10;
	
#ifdef DXTK_AUDIO
    // Create DirectXTK for Audio objects
    AUDIO_ENGINE_FLAGS eflags = AudioEngine_Default;
#ifdef _DEBUG
    eflags = eflags | AudioEngine_Debug;
#endif

    m_audEngine = std::make_unique<AudioEngine>(eflags);

    m_audioEvent = 0;
    m_audioTimerAcc = 10.f;
    m_retryDefault = false;

    m_waveBank = std::make_unique<WaveBank>(m_audEngine.get(), L"adpcmdroid.xwb");

    m_soundEffect = std::make_unique<SoundEffect>(m_audEngine.get(), L"MusicMono_adpcm.wav");
    m_effect1 = m_soundEffect->CreateInstance();
    m_effect2 = m_waveBank->CreateInstance(10);

    m_effect1->Play(true);
    m_effect2->Play();
#endif
}

#pragma region Frame Update
// Executes the basic game loop.
void Game::Tick()
{
	//take in input
	m_input.Update();								//update the hardware
	m_gameInputCommands = m_input.getGameInput();	//retrieve the input for our game
	
	//Update all game objects
    m_timer.Tick([&]()
    {
        Update(m_timer);
    });

	//Render all game content. 
    Render();

#ifdef DXTK_AUDIO
    // Only update audio engine once per frame
    if (!m_audEngine->IsCriticalError() && m_audEngine->Update())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
#endif

	
}

// Updates the world.
void Game::Update(DX::StepTimer const& timer)
{	
	//this is hacky,  i dont like this here.  
	auto device = m_deviceResources->GetD3DDevice();
    
    float deltaTime = float(timer.GetElapsedSeconds());
    float rotationSpeed = m_Camera01.getRotationSpeed() * deltaTime;
    float movementSpeed = m_Camera01.getMoveSpeed() * deltaTime;

    if (m_gameInputCommands.forward)
    {
        Vector3 position = m_Camera01.getPosition(); //get the position
        position += (m_Camera01.getForward() * movementSpeed); //add the forward vector
        m_Camera01.setPosition(position);
    }
    if (m_gameInputCommands.back)
    {
        Vector3 position = m_Camera01.getPosition(); //get the position
        position -= (m_Camera01.getForward() * movementSpeed); //add the forward vector
        m_Camera01.setPosition(position);
    }
    if (m_gameInputCommands.right)
    {
        Vector3 position = m_Camera01.getPosition(); //get the position
        position += (m_Camera01.getRight() * movementSpeed); //add the forward vector
        m_Camera01.setPosition(position);
    }
    if (m_gameInputCommands.left)
    {
        Vector3 position = m_Camera01.getPosition(); //get the position
        position -= (m_Camera01.getRight() * movementSpeed); //add the forward vector
        m_Camera01.setPosition(position);
    }

    if (m_gameInputCommands.xAxis != 0)
    {
        Vector3 rotation = m_Camera01.getRotation();
        rotation.y += rotationSpeed * m_gameInputCommands.xAxis;
        m_Camera01.setRotation(rotation);
    }
    if (m_gameInputCommands.yAxis != 0)
    {
        Vector3 rotation = m_Camera01.getRotation();
        rotation.z -= rotationSpeed * m_gameInputCommands.yAxis;
        if (rotation.z >= 179.5f)
            rotation.z = 179.5f;
        else if (rotation.z <= 0.5f)
            rotation.z = 0.5f;

        m_Camera01.setRotation(rotation);
    }

	if (m_gameInputCommands.generate)
	{
		m_Terrain.GenerateHeightMap(device);
        m_volcano.center = m_Terrain.GetVolcanoInfo()->center;
        m_volcano.radius = m_Terrain.GetVolcanoInfo()->radius;
        m_volcano.mountainRadius = m_Terrain.GetVolcanoInfo()->mountainRadius;
	}
    if (!isKinematic)
    {
        if (m_gameInputCommands.isPressingLaunch)
        {
            if (LaunchForce <= MaxLaunchForce)
            {
                LaunchForce += m_timer.GetElapsedSeconds() * ((MaxLaunchForce - MinLaunchForce)/2);
            }
            else
            {
                LaunchForce = MaxLaunchForce;
            }
        }
        if (m_gameInputCommands.launchButtonUp)
        {
            isKinematic = true;
        }
    }

    else
    {
        LaunchForce = 0;
    }

    if (m_gameInputCommands.smooth)
    {
        m_Terrain.SmoothHeightMap(device);
    }

    if (m_terrainDisplacementX != *m_Terrain.GetAmplitude() || m_terrainDisplacementY != *m_Terrain.GetWavelength())
    {
        m_terrainDisplacementX = *m_Terrain.GetAmplitude();
        m_terrainDisplacementY = *m_Terrain.GetWavelength();
        m_Terrain.GenerateHeightMap(device);
        m_volcano.center = m_Terrain.GetVolcanoInfo()->center;
        m_volcano.radius = m_Terrain.GetVolcanoInfo()->radius;
        m_volcano.mountainRadius = m_Terrain.GetVolcanoInfo()->mountainRadius;
    }

	m_Camera01.Update();	//camera update.
	m_Terrain.Update();		//terrain update.  doesnt do anything at the moment. 

	m_view = m_Camera01.getCameraMatrix();
	m_world = Matrix::Identity;

	/*create our UI*/
	SetupGUI();

#ifdef DXTK_AUDIO
    m_audioTimerAcc -= (float)timer.GetElapsedSeconds();
    if (m_audioTimerAcc < 0)
    {
        if (m_retryDefault)
        {
            m_retryDefault = false;
            if (m_audEngine->Reset())
            {
                // Restart looping audio
                m_effect1->Play(true);
            }
        }
        else
        {
            m_audioTimerAcc = 4.f;

            m_waveBank->Play(m_audioEvent++);

            if (m_audioEvent >= 11)
                m_audioEvent = 0;
        }
    }
#endif

  
	if (m_input.Quit())
	{
		ExitGame();
	}
}
#pragma endregion

#pragma region Frame Render
// Draws the scene.
void Game::Render()
{	
    // Don't try to render anything before the first Update.
    if (m_timer.GetFrameCount() == 0)
    {
        return;
    }

    Clear();

    m_deviceResources->PIXBeginEvent(L"Render");
    auto context = m_deviceResources->GetD3DDeviceContext();
	auto renderTargetView = m_deviceResources->GetRenderTargetView();
	auto depthTargetView = m_deviceResources->GetDepthStencilView();

    // Draw Text to the screen


	//Set Rendering states. 
	context->OMSetBlendState(m_states->Opaque(), nullptr, 0xFFFFFFFF);
	context->OMSetDepthStencilState(m_states->DepthDefault(), 0);
	context->RSSetState(m_states->CullClockwise());
	//context->RSSetState(m_states->Wireframe());



	
    ///APPLY PHYSICS TO BALL
	if (isKinematic)
	{
		if (ballTimer > 0)
		{
			float deltaTime = float(m_timer.GetElapsedSeconds());
			DirectX::SimpleMath::Vector3 newPos;
			DirectX::SimpleMath::Vector3 newVelocity;
			DirectX::SimpleMath::Vector3 gravity = DirectX::SimpleMath::Vector3(0, -9.81, 0) * 20;
			DirectX::SimpleMath::Vector3 acceleration = (gravity)+(m_Camera01.getForward() * LaunchForce);

			newVelocity = ballMovement.CurrentVelocity + (acceleration * deltaTime);
			newPos = ballMovement.CurrentPosiiton + (newVelocity * deltaTime);
			ballMovement.CurrentPosiiton = newPos;
			ballMovement.CurrentVelocity = newVelocity;
            ballTimer -= m_timer.GetElapsedSeconds();
		}
        else
        {
            isKinematic = false;
            ballTimer = ballMaxTime;
            ballMovement.CurrentVelocity = Vector3(0, 0, 0);
            ballMovement.CurrentPosiiton = m_Camera01.getPosition() + m_Camera01.getForward() * 50;
            LaunchForce = MinLaunchForce;
        }
        if (this->CheckSphereCollision())
	    {
		    float pointX = this->collisionPoint.x;
		    float pointZ = this->collisionPoint.z;
		    float distance = m_Terrain.DistanceBetween2DPoints(pointX, pointZ, m_volcano.center.x * terrainScale, m_volcano.center.y * terrainScale);


            /// IF ball landed inside volcano
		    if (distance <= (m_volcano.radius*terrainScale))
		    {
			    isKinematic = false;
			    ballTimer = ballMaxTime;
			    ballMovement.CurrentVelocity = Vector3(0, 0, 0);
			    ballMovement.CurrentPosiiton = m_Camera01.getPosition() + m_Camera01.getForward() * 50;
			    LaunchForce = MinLaunchForce;

                Score++;
                ///TODO: SCORE ON SCREEN, CHANGE LANDSCAPE


                IsChanging = true;
                float random = static_cast <float> (rand()) / (static_cast <float> (RAND_MAX / 100));
                m_Terrain.GetFinalHeightMap(m_deviceResources->GetD3DDevice(), random);

                Timer = TimeToChange;
		    }
            // If ball lands elsewhere
            else
            {
                float deltaTime = float(m_timer.GetElapsedSeconds());
                DirectX::SimpleMath::Vector3 newPos;
                DirectX::SimpleMath::Vector3 newVelocity;
                DirectX::SimpleMath::Vector3 gravity = DirectX::SimpleMath::Vector3(0, -9.81, 0) * 20;
                DirectX::SimpleMath::Vector3 acceleration = (gravity)+(m_Camera01.getForward() * LaunchForce);

                newVelocity = ballMovement.CurrentVelocity.Length() * 0.3f * -this->collisionNormal + (acceleration * deltaTime);
                newPos = ballMovement.CurrentPosiiton + (newVelocity * deltaTime);
                ballMovement.CurrentPosiiton = newPos;
                ballMovement.CurrentVelocity = newVelocity;
                ballTimer -= m_timer.GetElapsedSeconds();
            }
        }

	}
    else
    {
        ballMovement.CurrentPosiiton = m_Camera01.getPosition() + m_Camera01.getForward() * 50 + m_Camera01.getUp() * -30;
    }

    if (IsChanging && Timer >=0)
    {
        Timer -= m_timer.GetElapsedSeconds();

        m_Terrain.ChangeHeightMap(m_deviceResources->GetD3DDevice(), (Timer - TimeToChange) / TimeToChange);
        m_volcano.center = m_Terrain.GetVolcanoInfo()->center;
        m_volcano.radius = m_Terrain.GetVolcanoInfo()->radius;
        m_volcano.mountainRadius = m_Terrain.GetVolcanoInfo()->mountainRadius;

        std::wstring print = std::to_wstring(m_Terrain.m_heightMap[9].y);
        const wchar_t* rochar = print.c_str();

        m_sprites->Begin();
        m_font->DrawString(m_sprites.get(), rochar, XMFLOAT2(10, 50), Colors::Yellow);
        m_sprites->End();
    }

    if (Timer < 0)
    {
        Timer = TimeToChange;
        IsChanging = false;
    }

    //prepare transform for floor object. 
    m_world = SimpleMath::Matrix::Identity; //set world back to identity
    SimpleMath::Matrix newPosition3 = SimpleMath::Matrix::CreateTranslation(0.0f, 0.0f, 0.0f);
    SimpleMath::Matrix newScale = SimpleMath::Matrix::CreateScale(terrainScale);		//scale the terrain down a little. 
    m_world = m_world * newScale * newPosition3;

    //setup and draw terrain
    m_TerrainShader.EnableShader(context);
    m_TerrainShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_timer.GetTotalSeconds(),
        m_volcano, m_textureSand.Get(), m_textureGrass.Get(), m_textureDirt.Get(), m_textureRock.Get(), m_textureSnow.Get());
    m_Terrain.Render(context);

    //BALL
    m_world = SimpleMath::Matrix::Identity; //set world back to identity
    SimpleMath::Matrix positionBall = SimpleMath::Matrix::CreateTranslation(ballMovement.CurrentPosiiton);
    SimpleMath::Matrix scaleBall = SimpleMath::Matrix::CreateScale(ballScale);
    m_world = m_world * scaleBall * positionBall;

    //setup and draw ball
    m_BallShader.EnableShader(context);
    m_BallShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_textureBall.Get());
    m_ball.Render(context);

    //CUBE
    m_world = SimpleMath::Matrix::Identity; //set world back to identity
    SimpleMath::Matrix positionCube = SimpleMath::Matrix::CreateTranslation(cubePos);
    SimpleMath::Matrix scaleCube = SimpleMath::Matrix::CreateScale(1 * ballScale, 0.1 * ballScale, 1 * ballScale);
    m_world = m_world * scaleCube * positionCube ;

    //setup and draw ball
    m_shadowShader.EnableShader(context);
    m_shadowShader.SetShaderParameters(context, &m_world, &m_view, &m_projection, &m_Light, m_textureBall.Get());
    m_debugCube.Render(context);



    // Draw Text to the screen
    std::wstring print = std::to_wstring(0);
    const wchar_t* rochar = print.c_str();

    m_sprites->Begin();
    m_font->DrawString(m_sprites.get(), rochar, XMFLOAT2(10, 0), Colors::Yellow);
    m_sprites->End();

    print = std::to_wstring(0);
    rochar = print.c_str();

    m_sprites->Begin();
    m_font->DrawString(m_sprites.get(), rochar, XMFLOAT2(350, 0), Colors::Yellow);
    m_sprites->End();
    // Draw text

	//render our GUI
	//ImGui::Render();
	//ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
	
    
    // Show the new frame.
    m_deviceResources->Present();
}

bool Game::CheckSphereCollision()
{
    bool collision = false;
    Vector3 ballpos = ballMovement.CurrentPosiiton;
    if ((ballpos.x / terrainScale >= 0) && (ballpos.x / terrainScale < terrainSide)
        && (ballpos.z / terrainScale >= 0) && (ballpos.z / terrainScale < terrainSide))
    {
        //Get area to check
        int lowerWidth = (int)((ballpos.x - ballScale * 10) / terrainScale);
        int lowerHeight = (int)((ballpos.z - ballScale * 10) / terrainScale);
        int upperWidth = (int)((ballpos.x + ballScale * 10) / terrainScale);
        int upperHeight = (int)((ballpos.z + ballScale * 10) / terrainScale);
        int k = int(ballpos.x / terrainScale);
        int l = int(ballpos.z / terrainScale);
        int indexpos = (terrainSide * l) + k;
        cubePos.x = ballpos.x;
        cubePos.y = m_Terrain.m_heightMap[indexpos].y * terrainScale;
        cubePos.z = ballpos.z;



        if (lowerWidth < 0)
        {
            lowerWidth = 0;
        }
        if (lowerHeight < 0)
        {
            lowerHeight = 0;
        }
        if (upperWidth > terrainSide - 1)
        {
            upperWidth = terrainSide - 1;
        }
        if (upperHeight > terrainSide - 1)
        {
            upperHeight = terrainSide - 1;
        }







        for (int j = lowerHeight; j < upperHeight; j++)
        {
            for (int i = lowerWidth; i <= upperWidth; i++)
            {
                if (j + 1 < terrainSide && i + 1 < terrainSide && !collision)
                {
                    Vector3 CollisionPoint;
                    int index = (terrainSide * j) + i;
                    Vector3 A;
                    A.x = m_Terrain.m_heightMap[index].x * terrainScale;
                    A.y = m_Terrain.m_heightMap[index].y * terrainScale;
                    A.z = m_Terrain.m_heightMap[index].z * terrainScale;

                    index = (terrainSide * j) + i + 1;
                    Vector3 B;
                    B.x = m_Terrain.m_heightMap[index].x * terrainScale;
                    B.y = m_Terrain.m_heightMap[index].y * terrainScale;
                    B.z = m_Terrain.m_heightMap[index].z * terrainScale;

                    index = (terrainSide * (j + 1)) + i;
                    Vector3 C;
                    C.x = m_Terrain.m_heightMap[index].x * terrainScale;
                    C.y = m_Terrain.m_heightMap[index].y * terrainScale;
                    C.z = m_Terrain.m_heightMap[index].z * terrainScale;
                    collision = this->SphereWithTriangle(A, B, C, ballMovement.CurrentPosiiton, ballScale / 2);
                    if (collision)
                    {
                        break;
                    }
                }
            }
            if (collision)
            {
                break;
            }
        }
    }
    else
    {
        cubeScale = 200;
        cubePos.x = 0;
        cubePos.y = 0;
        cubePos.z = 0;
    }


    return collision;
}


bool Game::SphereWithTriangle(Vector3 A, Vector3 B, Vector3 C, Vector3 center, float radius)
{
    Vector3 closestPoint = this->ClosestPoint(A, B, C, center);



    if (this->PointInTriangle(closestPoint, A, B, C))
    {
        float distance = (center - closestPoint).Length();
        if (distance < radius)
        {
            this->collisionPoint = closestPoint;

            Vector3 AB = B - A;
            Vector3 AC = C - A;
            AB.Cross(AC, this->collisionNormal);
            this->collisionNormal = this->collisionNormal / this->collisionNormal.Length();
            //get unit normal

            return true;
        }

        

    }
    return false;
}

Vector3 Game::ClosestPoint(Vector3 A, Vector3 B, Vector3 C, Vector3 point)
{
    Vector3 AB = B - A;
    Vector3 AC = C - A;
    Vector3 normal;
    AB.Cross(AC, normal);
    //get unit normal
    normal = normal / normal.Length();
    float planeD = A.x * normal.x + A.y * normal.y + A.z * normal.z;
    float distance = point.Dot(normal) - planeD;

    return point - distance * normal;
}

bool Game::PointInTriangle(Vector3 point, Vector3 A, Vector3 B, Vector3 C)
{
    if ((point.x >= A.x && point.x <= B.x) && (point.z >= A.z && point.z <= C.z))
    {

        return true;
    }
    return false;
}

// Helper method to clear the back buffers.
void Game::Clear()
{
    m_deviceResources->PIXBeginEvent(L"Clear");

    // Clear the views.
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto renderTarget = m_deviceResources->GetRenderTargetView();
    auto depthStencil = m_deviceResources->GetDepthStencilView();

    context->ClearRenderTargetView(renderTarget, Colors::CornflowerBlue);
    context->ClearDepthStencilView(depthStencil, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
    context->OMSetRenderTargets(1, &renderTarget, depthStencil);

    // Set the viewport.
    auto viewport = m_deviceResources->GetScreenViewport();
    context->RSSetViewports(1, &viewport);

    m_deviceResources->PIXEndEvent();
}

#pragma endregion

#pragma region Message Handlers
// Message handlers
void Game::OnActivated()
{
}

void Game::OnDeactivated()
{
}

void Game::OnSuspending()
{
#ifdef DXTK_AUDIO
    m_audEngine->Suspend();
#endif
}

void Game::OnResuming()
{
    m_timer.ResetElapsedTime();

#ifdef DXTK_AUDIO
    m_audEngine->Resume();
#endif
}

void Game::OnWindowMoved()
{
    auto r = m_deviceResources->GetOutputSize();
    m_deviceResources->WindowSizeChanged(r.right, r.bottom);
}

void Game::OnWindowSizeChanged(int width, int height)
{
    if (!m_deviceResources->WindowSizeChanged(width, height))
        return;

    CreateWindowSizeDependentResources();
}

#ifdef DXTK_AUDIO
void Game::NewAudioDevice()
{
    if (m_audEngine && !m_audEngine->IsAudioDevicePresent())
    {
        // Setup a retry in 1 second
        m_audioTimerAcc = 1.f;
        m_retryDefault = true;
    }
}
#endif

// Properties
void Game::GetDefaultSize(int& width, int& height) const
{
    width = 800;
    height = 600;
}
#pragma endregion

#pragma region Direct3D Resources
// These are the resources that depend on the device.
void Game::CreateDeviceDependentResources()
{
    auto context = m_deviceResources->GetD3DDeviceContext();
    auto device = m_deviceResources->GetD3DDevice();

    m_states = std::make_unique<CommonStates>(device);
    m_fxFactory = std::make_unique<EffectFactory>(device);
    m_sprites = std::make_unique<SpriteBatch>(context);
    m_font = std::make_unique<SpriteFont>(device, L"SegoeUI_18.spritefont");
	m_batch = std::make_unique<PrimitiveBatch<VertexPositionColor>>(context);

	//setup our terrain
	m_Terrain.Initialize(device, terrainSide, terrainSide);
    m_terrainDisplacementX = *m_Terrain.GetAmplitude();
    m_terrainDisplacementY = *m_Terrain.GetWavelength();

    m_Terrain.GenerateHeightMap(device);
    m_volcano.center = m_Terrain.GetVolcanoInfo()->center;
    m_volcano.radius = m_Terrain.GetVolcanoInfo()->radius;
    m_volcano.mountainRadius = m_Terrain.GetVolcanoInfo()->mountainRadius;

	//setup our test model
	m_ball.InitializeSphere(device);
    m_debugCube.InitializeSphere(device);

	//load and set up our Vertex and Pixel Shaders
    m_TerrainShader.InitStandard(device, L"terrain_light_vs.cso", L"terrain_light_ps.cso");
    m_BallShader.InitStandard(device, L"light_vs.cso", L"light_ps.cso");
    m_shadowShader.InitStandard(device, L"colour_vs.cso", L"colour_ps.cso");

	//load Textures
	CreateDDSTextureFromFile(device, L"seafloor.dds",		nullptr,	m_texture1.ReleaseAndGetAddressOf());
	CreateDDSTextureFromFile(device, L"EvilDrone_Diff.dds", nullptr,	m_texture2.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/snow.dds", nullptr, m_textureSnow.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/rock.dds", nullptr, m_textureRock.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/dirt.dds", nullptr, m_textureDirt.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/grass.dds", nullptr, m_textureGrass.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/sand.dds", nullptr, m_textureSand.ReleaseAndGetAddressOf());
    CreateDDSTextureFromFile(device, L"textures/bball.dds", nullptr,m_textureBall.ReleaseAndGetAddressOf());

	//Initialise Render to texture
	m_FirstRenderPass = new RenderTexture(device, 800, 600, 1, 2);	//for our rendering, We dont use the last two properties. but.  they cant be zero and they cant be the same. 

}

// Allocate all memory resources that change on a window SizeChanged event.
void Game::CreateWindowSizeDependentResources()
{
    auto size = m_deviceResources->GetOutputSize();
    float aspectRatio = float(size.right) / float(size.bottom);
    float fovAngleY = 70.0f * XM_PI / 180.0f;

    // This is a simple example of change that can be made when the app is in
    // portrait or snapped view.
    if (aspectRatio < 1.0f)
    {
        fovAngleY *= 2.0f;
    }

    // This sample makes use of a right-handed coordinate system using row-major matrices.
    m_projection = Matrix::CreatePerspectiveFieldOfView(
        fovAngleY,
        aspectRatio,
        0.01f,
        10000.0f
    );
}

void Game::SetupGUI()
{

	//ImGui_ImplDX11_NewFrame();
	//ImGui_ImplWin32_NewFrame();
	//ImGui::NewFrame();

	//ImGui::Begin("Sin Wave Parameters");
	//	ImGui::SliderFloat("Wave Amplitude",	m_Terrain.GetAmplitude(), 0.0f, 10.0f);
	//	ImGui::SliderFloat("Wavelength",		m_Terrain.GetWavelength(), 0.0f, 10.0f);
	//ImGui::End();
}


void Game::OnDeviceLost()
{
    m_states.reset();
    m_fxFactory.reset();
    m_sprites.reset();
    m_font.reset();
	m_batch.reset();
	m_testmodel.reset();
    m_batchInputLayout.Reset();
}

void Game::OnDeviceRestored()
{
    CreateDeviceDependentResources();
    CreateWindowSizeDependentResources();
}
#pragma endregion
