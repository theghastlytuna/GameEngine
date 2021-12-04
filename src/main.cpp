#include <Logging.h>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <filesystem>
#include <json.hpp>
#include <fstream>
#include <sstream>
#include <typeindex>
#include <optional>
#include <string>

// GLM math library
#include <GLM/glm.hpp>
#include <GLM/gtc/matrix_transform.hpp>
#include <GLM/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <GLM/gtx/common.hpp> // for fmod (floating modulus)

// Graphics
#include "Graphics/IndexBuffer.h"
#include "Graphics/VertexBuffer.h"
#include "Graphics/VertexArrayObject.h"
#include "Graphics/Shader.h"
#include "Graphics/Texture2D.h"
#include "Graphics/TextureCube.h"
#include "Graphics/VertexTypes.h"
#include "Graphics/Font.h"
#include "Graphics/GuiBatcher.h"

// Utilities
#include "Utils/MeshBuilder.h"
#include "Utils/MeshFactory.h"
#include "Utils/ObjLoader.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/ResourceManager/ResourceManager.h"
#include "Utils/FileHelpers.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/StringUtils.h"
#include "Utils/GlmDefines.h"

// Gameplay
#include "Gameplay/Material.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"

// Components
#include "Gameplay/Components/IComponent.h"
#include "Gameplay/Components/Camera.h"
#include "Gameplay/Components/RotatingBehaviour.h"
#include "Gameplay/Components/JumpBehaviour.h"
#include "Gameplay/Components/RenderComponent.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"
#include "Gameplay/Components/FirstPersonCamera.h"
#include "Gameplay/Components/MovingPlatform.h"
#include "Gameplay/Components/PlayerControl.h"
#include "Gameplay/Components/MorphAnimator.h"

// Physics
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/Colliders/BoxCollider.h"
#include "Gameplay/Physics/Colliders/PlaneCollider.h"
#include "Gameplay/Physics/Colliders/SphereCollider.h"
#include "Gameplay/Physics/Colliders/ConvexMeshCollider.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Graphics/DebugDraw.h"
#include "Gameplay/Components/TriggerVolumeEnterBehaviour.h"
#include "Gameplay/Components/SimpleCameraControl.h"
#include "Gameplay/Physics/Colliders/CylinderCollider.h"

// GUI
#include "Gameplay/Components/GUI/RectTransform.h"
#include "Gameplay/Components/GUI/GuiPanel.h"
#include "Gameplay/Components/GUI/GuiText.h"
#include "Gameplay/InputEngine.h"

//#define LOG_GL_NOTIFICATIONS 

/*
	Handles debug messages from OpenGL
	https://www.khronos.org/opengl/wiki/Debug_Output#Message_Components
	@param source    Which part of OpenGL dispatched the message
	@param type      The type of message (ex: error, performance issues, deprecated behavior)
	@param id        The ID of the error or message (to distinguish between different types of errors, like nullref or index out of range)
	@param severity  The severity of the message (from High to Notification)
	@param length    The length of the message
	@param message   The human readable message from OpenGL
	@param userParam The pointer we set with glDebugMessageCallback (should be the game pointer)
*/
void GlDebugMessage(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam) {
	std::string sourceTxt;
	switch (source) {
		case GL_DEBUG_SOURCE_API: sourceTxt = "DEBUG"; break;
		case GL_DEBUG_SOURCE_WINDOW_SYSTEM: sourceTxt = "WINDOW"; break;
		case GL_DEBUG_SOURCE_SHADER_COMPILER: sourceTxt = "SHADER"; break;
		case GL_DEBUG_SOURCE_THIRD_PARTY: sourceTxt = "THIRD PARTY"; break;
		case GL_DEBUG_SOURCE_APPLICATION: sourceTxt = "APP"; break;
		case GL_DEBUG_SOURCE_OTHER: default: sourceTxt = "OTHER"; break;
	}
	switch (severity) {
		case GL_DEBUG_SEVERITY_LOW:          LOG_INFO("[{}] {}", sourceTxt, message); break;
		case GL_DEBUG_SEVERITY_MEDIUM:       LOG_WARN("[{}] {}", sourceTxt, message); break;
		case GL_DEBUG_SEVERITY_HIGH:         LOG_ERROR("[{}] {}", sourceTxt, message); break;
			#ifdef LOG_GL_NOTIFICATIONS
		case GL_DEBUG_SEVERITY_NOTIFICATION: LOG_INFO("[{}] {}", sourceTxt, message); break;
			#endif
		default: break;
	}
}  

// Stores our GLFW window in a global variable for now
GLFWwindow* window;
// The current size of our window in pixels
glm::ivec2 windowSize = glm::ivec2(1600, 900);
// The title of our GLFW window
std::string windowTitle = "Boomerangers";


// using namespace should generally be avoided, and if used, make sure it's ONLY in cpp files
using namespace Gameplay;
using namespace Gameplay::Physics;

// The scene that we will be rendering
Scene::Sptr scene = nullptr;

void GlfwWindowResizedCallback(GLFWwindow* window, int width, int height) {
	glEnable(GL_SCISSOR_TEST);
	glViewport(0, 0, width, height);
	glScissor(0, 0, width, height / 2);
	windowSize = glm::ivec2(width, height);
	if (windowSize.x * windowSize.y > 0) {
		scene->MainCamera->ResizeWindow(width, height);
	}
	GuiBatcher::SetWindowSize({ width, height });
}

/// <summary>
/// Handles intializing GLFW, should be called before initGLAD, but after Logger::Init()
/// Also handles creating the GLFW window
/// </summary>
/// <returns>True if GLFW was initialized, false if otherwise</returns>
bool initGLFW() {
	// Initialize GLFW
	if (glfwInit() == GLFW_FALSE) {
		LOG_ERROR("Failed to initialize GLFW");
		return false;
	}

	//Create a new GLFW window and make it current
	window = glfwCreateWindow(windowSize.x, windowSize.y, windowTitle.c_str(), nullptr, nullptr);
	glfwMakeContextCurrent(window);
	
	// Set our window resized callback
	glfwSetWindowSizeCallback(window, GlfwWindowResizedCallback);

	// Pass the window to the input engine and let it initialize itself
	InputEngine::Init(window);

	GuiBatcher::SetWindowSize(windowSize);

	return true;
}

/// <summary>
/// Handles initializing GLAD and preparing our GLFW window for OpenGL calls
/// </summary>
/// <returns>True if GLAD is loaded, false if there was an error</returns>
bool initGLAD() {
	if (gladLoadGLLoader((GLADloadproc)glfwGetProcAddress) == 0) {
		LOG_ERROR("Failed to initialize Glad");
		return false;
	}
	return true;
}

/// <summary>
/// Draws a widget for saving or loading our scene
/// </summary>
/// <param name="scene">Reference to scene pointer</param>
/// <param name="path">Reference to path string storage</param>
/// <returns>True if a new scene has been loaded</returns>
bool DrawSaveLoadImGui(Scene::Sptr& scene, std::string& path) {
	// Since we can change the internal capacity of an std::string,
	// we can do cool things like this!
	ImGui::InputText("Path", path.data(), path.capacity());

	// Draw a save button, and save when pressed
	if (ImGui::Button("Save")) {
		scene->Save(path);

		std::string newFilename = std::filesystem::path(path).stem().string() + "-manifest.json";
		ResourceManager::SaveManifest(newFilename);
	}
	ImGui::SameLine();
	// Load scene from file button
	if (ImGui::Button("Load")) {
		// Since it's a reference to a ptr, this will
		// overwrite the existing scene!
		scene = nullptr;

		std::string newFilename = std::filesystem::path(path).stem().string() + "-manifest.json";
		ResourceManager::LoadManifest(newFilename);
		scene = Scene::Load(path);

		return true;
	}
	return false;
}

/// <summary>
/// Draws some ImGui controls for the given light
/// </summary>
/// <param name="title">The title for the light's header</param>
/// <param name="light">The light to modify</param>
/// <returns>True if the parameters have changed, false if otherwise</returns>
bool DrawLightImGui(const Scene::Sptr& scene, const char* title, int ix) {
	bool isEdited = false;
	bool result = false;
	Light& light = scene->Lights[ix];
	ImGui::PushID(&light); // We can also use pointers as numbers for unique IDs
	if (ImGui::CollapsingHeader(title)) {
		isEdited |= ImGui::DragFloat3("Pos", &light.Position.x, 0.01f);
		isEdited |= ImGui::ColorEdit3("Col", &light.Color.r);
		isEdited |= ImGui::DragFloat("Range", &light.Range, 0.1f);

		result = ImGui::Button("Delete");
	}
	if (isEdited) {
		scene->SetShaderLight(ix);
	}

	ImGui::PopID();
	return result;
}

/// <summary>
/// Draws a simple window for displaying materials and their editors
/// </summary>
void DrawMaterialsWindow() {
	if (ImGui::Begin("Materials")) {
		ResourceManager::Each<Material>([](Material::Sptr material) {
			material->RenderImGui();
		});
	}
	ImGui::End();
}

/// <summary>
/// handles creating or loading the scene
/// </summary>
void CreateScene() {
	bool loadScene = false;  
	// For now we can use a toggle to generate our scene vs load from file
	if (loadScene) {
		ResourceManager::LoadManifest("manifest.json");
		scene = Scene::Load("scene.json");

		// Call scene awake to start up all of our components
		scene->Window = window;
		scene->Awake();
	} 
	else {
			// This time we'll have 2 different shaders, and share data between both of them using the UBO
			// This shader will handle reflective materials 
			Shader::Sptr reflectiveShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
				{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
				{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_environment_reflective.glsl" }
			});

			// This shader handles our basic materials without reflections (cause they expensive)
			Shader::Sptr basicShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
				{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
				{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_blinn_phong_textured.glsl" }
			});

			// This shader handles our basic materials without reflections (cause they expensive)
			Shader::Sptr specShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
				{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
				{ ShaderPartType::Fragment, "shaders/fragment_shaders/textured_specular.glsl" }
			});

			Shader::Sptr animShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
				{ ShaderPartType::Vertex, "shaders/vertex_shaders/morphAnim.glsl" },
				{ ShaderPartType::Fragment, "shaders/fragment_shaders/frag_blinn_phong_textured.glsl" }
			});

			///////////////////// NEW SHADERS ////////////////////////////////////////////

			// This shader handles our foliage vertex shader example
			Shader::Sptr foliageShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
				{ ShaderPartType::Vertex, "shaders/vertex_shaders/foliage.glsl" },
				{ ShaderPartType::Fragment, "shaders/fragment_shaders/screendoor_transparency.glsl" }
			});

			// This shader handles our cel shading example
			Shader::Sptr toonShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
				{ ShaderPartType::Vertex, "shaders/vertex_shaders/basic.glsl" },
				{ ShaderPartType::Fragment, "shaders/fragment_shaders/toon_shading.glsl" }
			});

		// Load in the meshes
		MeshResource::Sptr monkeyMesh = ResourceManager::CreateAsset<MeshResource>("Monkey.obj");
		MeshResource::Sptr cubeMesh = ResourceManager::CreateAsset<MeshResource>("cube.obj");
		MeshResource::Sptr boiMesh = ResourceManager::CreateAsset<MeshResource>("boi-tpose.obj");
		MeshResource::Sptr catcusMesh = ResourceManager::CreateAsset<MeshResource>("CatcusAnims/Catcus_Idle_001.obj");

		// Load in some textures
		Texture2D::Sptr    boxTexture = ResourceManager::CreateAsset<Texture2D>("textures/box-diffuse.png");
		Texture2D::Sptr    boxSpec    = ResourceManager::CreateAsset<Texture2D>("textures/box-specular.png");
		Texture2D::Sptr    monkeyTex  = ResourceManager::CreateAsset<Texture2D>("textures/monkey-uvMap.png");
		Texture2D::Sptr    leafTex    = ResourceManager::CreateAsset<Texture2D>("textures/leaves.png");
		Texture2D::Sptr	   catcusTex = ResourceManager::CreateAsset<Texture2D>("textures/cattusGood.png");

		leafTex->SetMinFilter(MinFilter::Nearest);
		leafTex->SetMagFilter(MagFilter::Nearest);

		//////////////Loading animation frames////////////////////////
		std::vector<MeshResource::Sptr> boiFrames;

		for (int i = 0; i < 8; i++)
		{
			boiFrames.push_back(ResourceManager::CreateAsset<MeshResource>("boi-" + std::to_string(i) + ".obj"));
		}

		std::vector<MeshResource::Sptr> catcusFrames;

		for (int i = 1; i < 8; i++)
		{
			catcusFrames.push_back(ResourceManager::CreateAsset<MeshResource>("CatcusAnims/Catcus_Idle_00" + std::to_string(i) + ".obj"));
		}
		//////////////////////////////////////////////////////////////


		// Here we'll load in the cubemap, as well as a special shader to handle drawing the skybox
		TextureCube::Sptr testCubemap = ResourceManager::CreateAsset<TextureCube>("cubemaps/ocean/ocean.jpg");
		Shader::Sptr      skyboxShader = ResourceManager::CreateAsset<Shader>(std::unordered_map<ShaderPartType, std::string>{
			{ ShaderPartType::Vertex, "shaders/vertex_shaders/skybox_vert.glsl" },
			{ ShaderPartType::Fragment, "shaders/fragment_shaders/skybox_frag.glsl" }
		});


		// Create an empty scene
		scene = std::make_shared<Scene>();

		// Setting up our enviroment map
		scene->SetSkyboxTexture(testCubemap);
		scene->SetSkyboxShader(skyboxShader);
		// Since the skybox I used was for Y-up, we need to rotate it 90 deg around the X-axis to convert it to z-up
		scene->SetSkyboxRotation(glm::rotate(MAT4_IDENTITY, glm::half_pi<float>(), glm::vec3(1.0f, 0.0f, 0.0f)));

		// Create our materials
		// This will be our box material, with no environment reflections
		Material::Sptr boxMaterial = ResourceManager::CreateAsset<Material>(basicShader);
		{
			boxMaterial->Name = "Box";
			boxMaterial->Set("u_Material.Diffuse", boxTexture);
			boxMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr boiMaterial = ResourceManager::CreateAsset<Material>(animShader);
		{
			boiMaterial->Name = "Boi";
			boiMaterial->Set("u_Material.Diffuse", boxTexture);
			boiMaterial->Set("u_Material.Shininess", 0.1f);
		}

		Material::Sptr catcusMaterial = ResourceManager::CreateAsset<Material>(animShader);
		{
			catcusMaterial->Name = "Catcus";
			catcusMaterial->Set("u_Material.Diffuse", catcusTex);
			catcusMaterial->Set("u_Material.Shininess", 0.1f);
		}

		// This will be the reflective material, we'll make the whole thing 90% reflective
		Material::Sptr monkeyMaterial = ResourceManager::CreateAsset<Material>(reflectiveShader);
		{
			monkeyMaterial->Name = "Monkey";
			monkeyMaterial->Set("u_Material.Diffuse", monkeyTex);
			monkeyMaterial->Set("u_Material.Shininess", 0.5f);
		}

		// This will be the reflective material, we'll make the whole thing 90% reflective
		Material::Sptr testMaterial = ResourceManager::CreateAsset<Material>(specShader);
		{
			testMaterial->Name = "Box-Specular";
			testMaterial->Set("u_Material.Diffuse", boxTexture);
			testMaterial->Set("u_Material.Specular", boxSpec);
		}

		// Our foliage vertex shader material
		Material::Sptr foliageMaterial = ResourceManager::CreateAsset<Material>(foliageShader);
		{
			foliageMaterial->Name = "Foliage Shader";
			foliageMaterial->Set("u_Material.Diffuse", leafTex);
			foliageMaterial->Set("u_Material.Shininess", 0.1f);
			foliageMaterial->Set("u_Material.Threshold", 0.1f);

			foliageMaterial->Set("u_WindDirection", glm::vec3(1.0f, 1.0f, 0.0f));
			foliageMaterial->Set("u_WindStrength",  0.5f);
			foliageMaterial->Set("u_VerticalScale", 1.0f);
			foliageMaterial->Set("u_WindSpeed",     1.0f);
		}

		// Our toon shader material
		Material::Sptr toonMaterial = ResourceManager::CreateAsset<Material>(toonShader);
		{
			toonMaterial->Name = "Toon";
			toonMaterial->Set("u_Material.Diffuse", boxTexture);
			toonMaterial->Set("u_Material.Shininess", 0.1f);
			toonMaterial->Set("u_Material.Steps", 8);
		}

		// Create some lights for our scene
		scene->Lights.resize(3);
		scene->Lights[0].Position = glm::vec3(0.0f, 1.0f, 3.0f);
		scene->Lights[0].Color = glm::vec3(1.0f, 1.0f, 1.0f);
		scene->Lights[0].Range = 100.0f;

		scene->Lights[1].Position = glm::vec3(1.0f, 0.0f, 3.0f);
		scene->Lights[1].Color = glm::vec3(0.2f, 0.8f, 0.1f);

		scene->Lights[2].Position = glm::vec3(0.0f, 1.0f, 3.0f);
		scene->Lights[2].Color = glm::vec3(1.0f, 0.2f, 0.1f);

		// We'll create a mesh that is a simple plane that we can resize later
		MeshResource::Sptr planeMesh = ResourceManager::CreateAsset<MeshResource>();
		planeMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(1.0f)));
		planeMesh->GenerateMesh();

		MeshResource::Sptr sphere = ResourceManager::CreateAsset<MeshResource>();
		sphere->AddParam(MeshBuilderParam::CreateIcoSphere(ZERO, ONE, 5));
		sphere->GenerateMesh();

		// Set up the scene's camera
		GameObject::Sptr camera = scene->CreateGameObject("Main Camera"); 
		{
			camera->SetPosition(glm::vec3(5.0f));
			camera->LookAt(glm::vec3(0.0f));

			camera->Add<SimpleCameraControl>();

			Camera::Sptr cam = camera->Add<Camera>();
			// Make sure that the camera is set as the scene's main camera!
			scene->MainCamera = cam;
			scene->WorldCamera = cam;
		}

		//Set up the scene's camera 
		GameObject::Sptr camera2 = scene->CreateGameObject("Main Camera 2");
		{
			camera2->SetPosition(glm::vec3(5.0f));
			camera2->LookAt(glm::vec3(0.0f));

			Camera::Sptr cam = camera2->Add<Camera>();
			// Make sure that the camera is set as the scene's main camera! 
			scene->MainCamera2 = cam;
		}

		GameObject::Sptr detachedCam = scene->CreateGameObject("Detached Camera");
		{
			ControllerInput::Sptr controller1 = detachedCam->Add<ControllerInput>();
			controller1->SetController(GLFW_JOYSTICK_1);

			detachedCam->SetPosition(glm::vec3(0.f, 0.f, 0.4f));

			FirstPersonCamera::Sptr cameraControl = detachedCam->Add<FirstPersonCamera>();

			Camera::Sptr cam = detachedCam->Add<Camera>();
			scene->PlayerCamera = cam;
		}

		GameObject::Sptr player1 = scene->CreateGameObject("Player 1");
		{
			ControllerInput::Sptr controller1 = player1->Add<ControllerInput>();
			controller1->SetController(GLFW_JOYSTICK_1);
			
			player1->SetPosition(glm::vec3(0.f, 0.f, 4.f));

			RenderComponent::Sptr renderer = player1->Add<RenderComponent>();
			renderer->SetMesh(monkeyMesh);
			renderer->SetMaterial(monkeyMaterial);

			player1->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

			RigidBody::Sptr physics = player1->Add<RigidBody>(RigidBodyType::Dynamic);
			physics->AddCollider(SphereCollider::Create());
			physics->SetAngularFactor(glm::vec3(0.f));
			physics->SetLinearDamping(0.8f);

			//FirstPersonCamera::Sptr cameraControl = player1->Add<FirstPersonCamera>();
			PlayerControl::Sptr controller = player1->Add<PlayerControl>();

			JumpBehaviour::Sptr jumping = player1->Add<JumpBehaviour>();

			player1->AddChild(detachedCam);

			//Camera::Sptr cam = player1->Add<Camera>();
			//scene->PlayerCamera = cam;
		}

		GameObject::Sptr detachedCam2 = scene->CreateGameObject("Detached Camera 2");
		{
			ControllerInput::Sptr controller2 = detachedCam2->Add<ControllerInput>();
			controller2->SetController(GLFW_JOYSTICK_2);

			detachedCam2->SetPosition(glm::vec3(10.f, 0.f, 0.4f));

			FirstPersonCamera::Sptr cameraControl = detachedCam2->Add<FirstPersonCamera>();

			Camera::Sptr cam = detachedCam2->Add<Camera>();
			scene->PlayerCamera2 = cam;
		}

		GameObject::Sptr player2 = scene->CreateGameObject("Player 2");
		{
			ControllerInput::Sptr controller2 = player2->Add<ControllerInput>();
			controller2->SetController(GLFW_JOYSTICK_2);

			player2->SetPosition(glm::vec3(10.f, 0.f, 4.f));

			RenderComponent::Sptr renderer = player2->Add<RenderComponent>();
			renderer->SetMesh(monkeyMesh);
			renderer->SetMaterial(monkeyMaterial);

			player2->SetScale(glm::vec3(0.5f, 0.5f, 0.5f));

			RigidBody::Sptr physics = player2->Add<RigidBody>(RigidBodyType::Dynamic);
			physics->AddCollider(SphereCollider::Create(0.7f));
			physics->SetAngularFactor(glm::vec3(0.f));
			physics->SetLinearDamping(0.8f);

			//FirstPersonCamera::Sptr cameraControl = player2->Add<FirstPersonCamera>();

			PlayerControl::Sptr controller = player2->Add<PlayerControl>();

			JumpBehaviour::Sptr jumping = player2->Add<JumpBehaviour>();

			//Camera::Sptr cam = player2->Add<Camera>();
			//scene->PlayerCamera2 = cam;
		}

		// Set up all our sample objects
		GameObject::Sptr plane = scene->CreateGameObject("Ground");
		{
			// Make a big tiled mesh
			MeshResource::Sptr tiledMesh = ResourceManager::CreateAsset<MeshResource>();
			tiledMesh->AddParam(MeshBuilderParam::CreatePlane(ZERO, UNIT_Z, UNIT_X, glm::vec2(100.0f), glm::vec2(20.0f)));
			tiledMesh->GenerateMesh();

			// Create and attach a RenderComponent to the object to draw our mesh
			RenderComponent::Sptr renderer = plane->Add<RenderComponent>();
			renderer->SetMesh(tiledMesh);
			renderer->SetMaterial(boxMaterial);

			BoxCollider::Sptr collider = BoxCollider::Create(glm::vec3(50.0f, 50.0f, 1.0f));
			collider->SetPosition({ 0,0,-1 });

			// Attach a plane collider that extends infinitely along the X/Y axis
			RigidBody::Sptr physics = plane->Add<RigidBody>(/*static by default*/);
			physics->AddCollider(collider);

			TriggerVolume::Sptr volume = plane->Add<TriggerVolume>();
			volume->AddCollider(BoxCollider::Create(glm::vec3(50.0f, 50.0f, 1.0f)))->SetPosition({ 0,0,-1});

			plane->Add<TriggerVolumeEnterBehaviour>();
		}

		// Set up all our sample objects
		GameObject::Sptr movingPlat = scene->CreateGameObject("GroundMoving");
		{
			// Set position in the scene
			movingPlat->SetPosition(glm::vec3(10.0f, 0.0f, 5.0f));
			// Scale down the plane
			movingPlat->SetScale(glm::vec3(1.0f, 1.0f, 0.5f));

			// Create and attach a render component
			RenderComponent::Sptr renderer = movingPlat->Add<RenderComponent>();
			renderer->SetMesh(cubeMesh);
			renderer->SetMaterial(boxMaterial);

			TriggerVolume::Sptr volume = movingPlat->Add<TriggerVolume>();

			ConvexMeshCollider::Sptr collider = ConvexMeshCollider::Create();
			collider->SetScale(glm::vec3(2.0f, 2.0f, 0.5f));

			RigidBody::Sptr physics = movingPlat->Add<RigidBody>(RigidBodyType::Kinematic);
			physics->AddCollider(collider);
			volume->AddCollider(collider);

			movingPlat->Add<TriggerVolumeEnterBehaviour>();
			
			movingPlat->Add<MovingPlatform>();
			
			std::vector<glm::vec3> nodes = { glm::vec3(10, 0, 5), glm::vec3(7, 0, 7), glm::vec3(4, 3, 5), glm::vec3(6, 2, 2) };

			movingPlat->Get<MovingPlatform>()->SetMode(MovingPlatform::MovementMode::LERP);
			movingPlat->Get<MovingPlatform>()->SetNodes(nodes, 3.0f);
			
		}

		GameObject::Sptr boomerang = scene->CreateGameObject("Boomerang");
		{
			// Set position in the scene
			boomerang->SetPosition(glm::vec3(2.0f, 0.0f, 0.0f));
			boomerang->SetScale(glm::vec3(0.5f, 0.25f, 0.5f));

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = boomerang->Add<RenderComponent>();
			renderer->SetMesh(cubeMesh);
			renderer->SetMaterial(boxMaterial);

			RigidBody::Sptr physics = boomerang->Add<RigidBody>(RigidBodyType::Dynamic);
			physics->AddCollider(ConvexMeshCollider::Create());
		}
		/*
		for (int i = 0; i < 8; i++)
		{
			GameObject::Sptr boiFrame = scene->CreateGameObject("Boi" + std::to_string(i));
			{
				// Set position in the scene
				boiFrame->SetPosition(glm::vec3(-100.0f, 0.0f, 0.0f));
				boiFrame->SetScale(glm::vec3(0.05f, 0.05f, 0.05f));
				boiFrame->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

				// Create and attach a renderer for the monkey
				RenderComponent::Sptr renderer = boiFrame->Add<RenderComponent>();
				renderer->SetMesh(boiFrames[i]);
				renderer->SetMaterial(boxMaterial);
			}
		}
		*/
		
		GameObject::Sptr boiBase = scene->CreateGameObject("Boi Base");
		{
			// Set position in the scene
			boiBase->SetPosition(glm::vec3(-10.0f, 0.0f, 0.0f));
			boiBase->SetScale(glm::vec3(0.05f, 0.05f, 0.05f));
			boiBase->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = boiBase->Add<RenderComponent>();
			renderer->SetMesh(boiMesh);
			renderer->SetMaterial(boiMaterial);

			//Only add an animator when you have a clip to add.
			MorphAnimator::Sptr animator = boiBase->Add<MorphAnimator>();

			//Add the walking clip
			animator->AddClip(boiFrames, 1.0f, "Walk");

			//Make sure to always activate an animation at the time of creation (usually idle)
			animator->ActivateAnim("walk");
		}
		
		GameObject::Sptr catcus = scene->CreateGameObject("Catcus Base");
		{
			// Set position in the scene
			catcus->SetPosition(glm::vec3(20.0f, 0.0f, 0.0f));
			catcus->SetScale(glm::vec3(1.0f, 1.0f, 1.0f));
			catcus->SetRotation(glm::vec3(90.0f, 0.0f, 0.0f));

			// Create and attach a renderer for the monkey
			RenderComponent::Sptr renderer = catcus->Add<RenderComponent>();
			renderer->SetMesh(catcusMesh);
			renderer->SetMaterial(catcusMaterial);

			
			//Only add an animator when you have a clip to add.
			MorphAnimator::Sptr animator = catcus->Add<MorphAnimator>();

			//Add the walking clip
			animator->AddClip(catcusFrames, 0.7f, "Idle");

			//Make sure to always activate an animation at the time of creation (usually idle)
			animator->ActivateAnim("Idle");
			
		}
		
		/////////////////////////// UI //////////////////////////////
		GameObject::Sptr canvas = scene->CreateGameObject("UI Canvas");
		{
			RectTransform::Sptr transform = canvas->Add<RectTransform>();
			transform->SetMin({ 0, 0 });
			transform->SetMax({ 256, 256 });

			GuiPanel::Sptr canPanel = canvas->Add<GuiPanel>();
			
			GameObject::Sptr subPanel = scene->CreateGameObject("Sub Item");
			{
				RectTransform::Sptr transform = subPanel->Add<RectTransform>();
				transform->SetMin({ 0, 0 });
				transform->SetMax({ 64, 64 });

				GuiPanel::Sptr panel = subPanel->Add<GuiPanel>();
				panel->SetColor(glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

				Font::Sptr font = ResourceManager::CreateAsset<Font>("fonts/Roboto-Medium.ttf", 16.0f);
				font->Bake();

				GuiText::Sptr text = subPanel->Add<GuiText>();
				text->SetText("Hello world!");
				text->SetFont(font);
			}

			canvas->AddChild(subPanel);
		}

		GuiBatcher::SetDefaultTexture(ResourceManager::CreateAsset<Texture2D>("textures/ui-sprite.png"));
		GuiBatcher::SetDefaultBorderRadius(8);


		// Call scene awake to start up all of our components
		scene->Window = window;
		scene->Awake();

		// Save the asset manifest for all the resources we just loaded
		ResourceManager::SaveManifest("manifest.json");
		// Save the scene to a JSON file
		scene->Save("scene.json");
	}
}

void arrive(GameObject::Sptr object, GameObject::Sptr target, float deltaT)
{
	//Find the current velocity, and the target velocity (which is just the position of the target - the position of the object)
	glm::vec3 currentVel =object->Get<RigidBody>()->GetLinearVelocity();
	glm::vec3 targVel = target->GetPosition() - object->GetPosition();

	//Scalar to be multiplied with the force. Proportionate to 1 / length of the target velocity, meaning it will speed up as it gets closer
	float scalar = glm::length(targVel - currentVel) > 25 ? 5.0f : 30.0f - glm::length(targVel - currentVel);

	//Find the mass of the object
	float objMass = object->Get<RigidBody>()->GetMass();

	glm::vec3 dir = glm::normalize(glm::normalize(targVel) - glm::normalize(currentVel));

	//if the length of dir is 0, meaning the target and current velocities are in the same direction,
	//just set the the direction to be the target velocity
	if (glm::length(dir) == 0)
		dir = glm::normalize(targVel);

	//Find the force which we want to apply, which is just the target velocity - the current velocity * some scalar
	//plus add the gravity back, since we don't want gravity to interrupt the seeking
	glm::vec3 forceToApply = scalar * dir + objMass * glm::vec3(0.0f, 0.0f, 9.8f);

	//Apply the force
	object->Get<RigidBody>()->ApplyForce(forceToApply);

}

int main() {
	Logger::Init(); // We'll borrow the logger from the toolkit, but we need to initialize it

	//Initialize GLFW
	if (!initGLFW())
		return 1;

	//Initialize GLAD
	if (!initGLAD())
		return 1;

	// Let OpenGL know that we want debug output, and route it to our handler function
	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
	glDebugMessageCallback(GlDebugMessage, nullptr);

	// Initialize our ImGui helper
	ImGuiHelper::Init(window);

	// Initialize our resource manager
	ResourceManager::Init();

	// Register all our resource types so we can load them from manifest files
	ResourceManager::RegisterType<Texture2D>();
	ResourceManager::RegisterType<TextureCube>();
	ResourceManager::RegisterType<Shader>();
	ResourceManager::RegisterType<Material>();
	ResourceManager::RegisterType<MeshResource>();

	// Register all of our component types so we can load them from files
	ComponentManager::RegisterType<ControllerInput>();
	ComponentManager::RegisterType<Camera>();
	ComponentManager::RegisterType<RenderComponent>();
	ComponentManager::RegisterType<RigidBody>();
	ComponentManager::RegisterType<TriggerVolume>();
	ComponentManager::RegisterType<RotatingBehaviour>();
	ComponentManager::RegisterType<JumpBehaviour>();
	ComponentManager::RegisterType<MaterialSwapBehaviour>();
	ComponentManager::RegisterType<TriggerVolumeEnterBehaviour>();
	ComponentManager::RegisterType<SimpleCameraControl>();
	ComponentManager::RegisterType<FirstPersonCamera>();
	ComponentManager::RegisterType<MovingPlatform>();
	ComponentManager::RegisterType<PlayerControl>();
	ComponentManager::RegisterType<MorphAnimator>();

	ComponentManager::RegisterType<RectTransform>();
	ComponentManager::RegisterType<GuiPanel>();
	ComponentManager::RegisterType<GuiText>();

	// GL states, we'll enable depth testing and backface fulling
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glCullFace(GL_BACK);
	glClearColor(0.2f, 0.2f, 0.2f, 1.0f);


	// Structure for our frame-level uniforms, matches layout from
	// fragments/frame_uniforms.glsl
	// For use with a UBO.
	struct FrameLevelUniforms {
		// The camera's view matrix
		glm::mat4 u_View;
		// The camera's projection matrix
		glm::mat4 u_Projection;
		// The combined viewProject matrix
		glm::mat4 u_ViewProjection;
		// The camera's position in world space
		glm::vec4 u_CameraPos;
		// The time in seconds since the start of the application
		float u_Time;
	};
	// This uniform buffer will hold all our frame level uniforms, to be shared between shaders
	UniformBuffer<FrameLevelUniforms>::Sptr frameUniforms = std::make_shared<UniformBuffer<FrameLevelUniforms>>(BufferUsage::DynamicDraw);
	// The slot that we'll bind our frame level UBO to
	const int FRAME_UBO_BINDING = 0;

	// Structure for our isntance-level uniforms, matches layout from
	// fragments/frame_uniforms.glsl
	// For use with a UBO.
	struct InstanceLevelUniforms {
		// Complete MVP
		glm::mat4 u_ModelViewProjection;
		// Just the model transform, we'll do worldspace lighting
		glm::mat4 u_Model;
		// Normal Matrix for transforming normals
		glm::mat4 u_NormalMatrix;
	};

	// This uniform buffer will hold all our instance level uniforms, to be shared between shaders
	UniformBuffer<InstanceLevelUniforms>::Sptr instanceUniforms = std::make_shared<UniformBuffer<InstanceLevelUniforms>>(BufferUsage::DynamicDraw);

	// The slot that we'll bind our instance level UBO to
	const int INSTANCE_UBO_BINDING = 1;

	////////////////////////////////
	///// SCENE CREATION MOVED /////
	////////////////////////////////
	CreateScene();

	// We'll use this to allow editing the save/load path
	// via ImGui, note the reserve to allocate extra space
	// for input!
	std::string scenePath = "scene.json"; 
	scenePath.reserve(256); 

	// Our high-precision timer
	double lastFrame = glfwGetTime();

	BulletDebugMode physicsDebugMode = BulletDebugMode::None;
	float playbackSpeed = 1.0f;

	nlohmann::json editorSceneState;

	GameObject::Sptr player1 = scene->FindObjectByName("Player 1");
	GameObject::Sptr player2 = scene->FindObjectByName("Player 2");
	GameObject::Sptr boomerang = scene->FindObjectByName("Boomerang");

	bool arriving = false;

	float t;

	float duration = 2.0f;

	float frameTimer = 0.0f;

	int currentFrame = 0;
	int nextFrame = 1;

	///// Game loop /////
	while (!glfwWindowShouldClose(window)) {
		glfwPollEvents();
		ImGuiHelper::StartFrame();
		
		// Calculate the time since our last frame (dt)
		double thisFrame = glfwGetTime();
		float dt = static_cast<float>(thisFrame - lastFrame);

		// Draw our material properties window!
		DrawMaterialsWindow();

		// Showcasing how to use the imGui library!
		bool isDebugWindowOpen = ImGui::Begin("Debugging");
		if (isDebugWindowOpen) {
			// Draws a button to control whether or not the game is currently playing
			static char buttonLabel[64];
			sprintf_s(buttonLabel, "%s###playmode", scene->IsPlaying ? "Exit Play Mode" : "Enter Play Mode");
			if (ImGui::Button(buttonLabel)) {
				// Save scene so it can be restored when exiting play mode
				if (!scene->IsPlaying) {
					editorSceneState = scene->ToJson();
				}

				// Toggle state
				scene->IsPlaying = !scene->IsPlaying;

				// If we've gone from playing to not playing, restore the state from before we started playing
				if (!scene->IsPlaying) {
					scene = nullptr;
					// We reload to scene from our cached state
					scene = Scene::FromJson(editorSceneState);
					// Don't forget to reset the scene's window and wake all the objects!
					scene->Window = window;
					scene->Awake();
				}
			}

			ImGui::Separator();
			if (ImGui::Button("Toggle Camera")) {
				if (scene->MainCamera == scene->WorldCamera)
				{
					scene->MainCamera = scene->PlayerCamera;
					scene->MainCamera2 = scene->PlayerCamera2;
				}
				else
				{
					scene->MainCamera = scene->WorldCamera;
					scene->MainCamera2 = scene->WorldCamera;
				}
			}


			// Make a new area for the scene saving/loading
			ImGui::Separator();
			if (DrawSaveLoadImGui(scene, scenePath)) {
				// C++ strings keep internal lengths which can get annoying
				// when we edit it's underlying datastore, so recalcualte size
				scenePath.resize(strlen(scenePath.c_str()));

				// We have loaded a new scene, call awake to set
				// up all our components
				scene->Window = window;
				scene->Awake();
			}
			ImGui::Separator();
			// Draw a dropdown to select our physics debug draw mode
			if (BulletDebugDraw::DrawModeGui("Physics Debug Mode:", physicsDebugMode)) {
				scene->SetPhysicsDebugDrawMode(physicsDebugMode);
			}
			LABEL_LEFT(ImGui::SliderFloat, "Playback Speed:    ", &playbackSpeed, 0.0f, 10.0f);
			ImGui::Separator();
		}

		// Clear the color and depth buffers
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Draw some ImGui stuff for the lights
		if (isDebugWindowOpen) {
			for (int ix = 0; ix < scene->Lights.size(); ix++) {
				char buff[256];
				sprintf_s(buff, "Light %d##%d", ix, ix);
				// DrawLightImGui will return true if the light was deleted
				if (DrawLightImGui(scene, buff, ix)) {
					// Remove light from scene, restore all lighting data
					scene->Lights.erase(scene->Lights.begin() + ix);
					scene->SetupShaderAndLights();
					// Move back one element so we don't skip anything!
					ix--;
				}
			}
			// As long as we don't have max lights, draw a button
			// to add another one
			if (scene->Lights.size() < scene->MAX_LIGHTS) {
				if (ImGui::Button("Add Light")) {
					scene->Lights.push_back(Light());
					scene->SetupShaderAndLights();
				}
			}
			// Split lights from the objects in ImGui
			ImGui::Separator();
		}

		dt *= playbackSpeed;

		if (glfwGetKey(window, GLFW_KEY_Q))
		{
			boomerang->SetPosition(player1->GetPosition() + glm::vec3(0.0f, 0.0f, 1.0f));
			boomerang->Get<RigidBody>()->SetLinearVelocity(glm::vec3(
				scene->PlayerCamera->GetView()[0][2],
				scene->PlayerCamera->GetView()[1][2],
				scene->PlayerCamera->GetView()[2][2]) * -10.0f);
			arriving = true;
		}

		

		// Perform updates for all components
		scene->Update(dt);

		// Update our worlds physics!
		scene->DoPhysics(dt);

		if (arriving)
		{
			arrive(boomerang, player2, dt);
		}

		GameObject::Sptr detachedCam = scene->FindObjectByName("Detached Camera");
		GameObject::Sptr player = scene->FindObjectByName("Player 1");

		//detachedCam->SetPosition(player->GetPosition());

		glViewport(0, windowSize.y / 2, windowSize.x, windowSize.y / 2);

		///////////////////////////////////////////////////////////////////////////////////Camera 1 Rendering
		{;
		// Grab shorthands to the camera and shader from the scene
		Camera::Sptr camera = scene->MainCamera;

		// Cache the camera's viewprojection
		glm::mat4 viewProj = camera->GetViewProjection();
		DebugDrawer::Get().SetViewProjection(viewProj);
		
		// The current material that is bound for rendering
		Material::Sptr currentMat = nullptr;
		Shader::Sptr shader = nullptr;

		// Bind the skybox texture to a reserved texture slot
		// See Material.h and Material.cpp for how we're reserving texture slots
		TextureCube::Sptr environment = scene->GetSkyboxTexture();
		if (environment) environment->Bind(0); 

		// Make sure depth testing and culling are re-enabled
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_CULL_FACE);

		// Here we'll bind all the UBOs to their corresponding slots
		scene->PreRender();
		frameUniforms->Bind(FRAME_UBO_BINDING);
		instanceUniforms->Bind(INSTANCE_UBO_BINDING);

		// Upload frame level uniforms
		auto& frameData = frameUniforms->GetData();
		frameData.u_Projection = camera->GetProjection();
		frameData.u_View = camera->GetView();
		frameData.u_ViewProjection = camera->GetViewProjection();
		frameData.u_CameraPos = glm::vec4(camera->GetGameObject()->GetPosition(), 1.0f);
		frameData.u_Time = static_cast<float>(thisFrame);
		frameUniforms->Update();

		// Render all our objects
		ComponentManager::Each<RenderComponent>([&](const RenderComponent::Sptr& renderable) {
			// Early bail if mesh not set
			if (renderable->GetMesh() == nullptr) { 
				return;
			}

			// If we don't have a material, try getting the scene's fallback material
			// If none exists, do not draw anything
			if (renderable->GetMaterial() == nullptr) {
				if (scene->DefaultMaterial != nullptr) {
					renderable->SetMaterial(scene->DefaultMaterial);
				} else {
					return;
				}
			}

			// If the material has changed, we need to bind the new shader and set up our material and frame data
			// Note: This is a good reason why we should be sorting the render components in ComponentManager
			if (renderable->GetMaterial() != currentMat) {
				currentMat = renderable->GetMaterial();
				shader = currentMat->GetShader();

				shader->Bind();
				currentMat->Apply();
			}

			// Grab the game object so we can do some stuff with it
			GameObject* object = renderable->GetGameObject();
			 
			// Use our uniform buffer for our instance level uniforms
			auto& instanceData = instanceUniforms->GetData();
			instanceData.u_Model = object->GetTransform();
			instanceData.u_ModelViewProjection = viewProj * object->GetTransform();
			instanceData.u_NormalMatrix = glm::mat3(glm::transpose(glm::inverse(object->GetTransform())));
			instanceUniforms->Update();  

			// Draw the object
			renderable->GetMesh()->Draw();
		});

		};
		// Use our cubemap to draw our skybox
		scene->DrawSkybox(scene->MainCamera);

		VertexArrayObject::Unbind();

		GameObject::Sptr detachedCam2 = scene->FindObjectByName("Detached Camera 2");
		GameObject::Sptr player2 = scene->FindObjectByName("Player 2");

		detachedCam2->SetPosition(player2->GetPosition());

		// Disable culling
		glDisable(GL_CULL_FACE);
		// Disable depth testing, we're going to use order-dependant layering
		glDisable(GL_DEPTH_TEST);
		// Disable depth writing
		glDepthMask(GL_FALSE);

		// Enable alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Our projection matrix will be our entire window for now
		glm::mat4 proj = glm::ortho(0.0f, (float)windowSize.x, (float)windowSize.y / 2, 0.f, -1.0f, 1.0f);
		GuiBatcher::SetProjection(proj);
		GuiBatcher::SetWindowSize({ windowSize.x, (float)windowSize.y / 2 });

		// Iterate over and render all the GUI objects
		scene->RenderGUI();

		// Flush the Gui Batch renderer
		GuiBatcher::Flush();

		// Disable alpha blending
		glDisable(GL_BLEND);
		// Disable scissor testing
		glDisable(GL_SCISSOR_TEST);
		// Re-enable depth writing
		glDepthMask(GL_TRUE);
		glEnable(GL_DEPTH_TEST);


		//split the screen
		glViewport(0, 0, windowSize.x, windowSize.y / 2);

		/////////////////////////////////////////////////////////////////////////////////Camera 2 Rendering
		{;
		// Grab shorthands to the camera and shader from the scene
		Camera::Sptr camera = scene->MainCamera2;

		// Cache the camera's viewprojection
		glm::mat4 viewProj = camera->GetViewProjection();
		DebugDrawer::Get().SetViewProjection(viewProj);

		// The current material that is bound for rendering
		Material::Sptr currentMat = nullptr;
		Shader::Sptr shader = nullptr;

		// Bind the skybox texture to a reserved texture slot
		// See Material.h and Material.cpp for how we're reserving texture slots
		TextureCube::Sptr environment = scene->GetSkyboxTexture();
		if (environment) environment->Bind(1);

		// Here we'll bind all the UBOs to their corresponding slots
		scene->PreRender();
		frameUniforms->Bind(FRAME_UBO_BINDING);
		instanceUniforms->Bind(INSTANCE_UBO_BINDING);

		// Upload frame level uniforms
		auto& frameData = frameUniforms->GetData();
		frameData.u_Projection = camera->GetProjection();
		frameData.u_View = camera->GetView();
		frameData.u_ViewProjection = camera->GetViewProjection();
		frameData.u_CameraPos = glm::vec4(camera->GetGameObject()->GetPosition(), 1.0f);
		frameData.u_Time = static_cast<float>(thisFrame);
		frameUniforms->Update();

		// Render all our objects
		ComponentManager::Each<RenderComponent>([&](const RenderComponent::Sptr& renderable) {
			// Early bail if mesh not set
			if (renderable->GetMesh() == nullptr) {
				return;
			}

			// If we don't have a material, try getting the scene's fallback material
			// If none exists, do not draw anything
			if (renderable->GetMaterial() == nullptr) {
				if (scene->DefaultMaterial != nullptr) {
					renderable->SetMaterial(scene->DefaultMaterial);
				}
				else {
					return;
				}
			}

			// If the material has changed, we need to bind the new shader and set up our material and frame data
			// Note: This is a good reason why we should be sorting the render components in ComponentManager
			if (renderable->GetMaterial() != currentMat) {
				currentMat = renderable->GetMaterial();
				shader = currentMat->GetShader();

				shader->Bind();
				currentMat->Apply();
			}

			// Grab the game object so we can do some stuff with it
			GameObject* object = renderable->GetGameObject();

			// Use our uniform buffer for our instance level uniforms
			auto& instanceData = instanceUniforms->GetData();
			instanceData.u_Model = object->GetTransform();
			instanceData.u_ModelViewProjection = viewProj * object->GetTransform();
			instanceData.u_NormalMatrix = glm::mat3(glm::transpose(glm::inverse(object->GetTransform())));
			instanceUniforms->Update();

			// Draw the object
			renderable->GetMesh()->Draw();
			});
		};

		// Draw object GUIs
		if (isDebugWindowOpen) {
			scene->DrawAllGameObjectGUIs();
		}

		// Use our cubemap to draw our skybox
		scene->DrawSkybox(scene->MainCamera2);

		// Disable culling
		glDisable(GL_CULL_FACE);
		// Disable depth testing, we're going to use order-dependant layering
		glDisable(GL_DEPTH_TEST);
		// Disable depth writing
		glDepthMask(GL_FALSE);

		// Enable alpha blending
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		GuiBatcher::SetProjection(proj);
		GuiBatcher::SetWindowSize({ windowSize.x, (float)windowSize.y / 2 });

		// Iterate over and render all the GUI objects
		scene->RenderGUI();

		// Flush the Gui Batch renderer
		GuiBatcher::Flush();

		// Disable alpha blending
		glDisable(GL_BLEND);
		// Disable scissor testing
		glDisable(GL_SCISSOR_TEST);
		// Re-enable depth writing
		glDepthMask(GL_TRUE);

		////////////////////////////////////////////////////////////////////////// END RENDERING
		// End our ImGui window
		ImGui::End();

		VertexArrayObject::Unbind();

		lastFrame = thisFrame;
		ImGuiHelper::EndFrame();
		InputEngine::EndFrame();
		glfwSwapBuffers(window);
	}

	// Clean up the ImGui library
	ImGuiHelper::Cleanup();

	// Clean up the resource manager
	ResourceManager::Cleanup();

	// Clean up the toolkit logger so we don't leak memory
	Logger::Uninitialize();
	return 0;
}