#include "Gameplay/Components/JumpBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"

void JumpBehaviour::Awake()
{
	_body = GetComponent<Gameplay::Physics::RigidBody>();
	if (_body == nullptr) {
		IsEnabled = false;
	}

	_controller = GetComponent<ControllerInput>();

	if (_controller == nullptr)
	{
		IsEnabled = false;
	}
}

void JumpBehaviour::RenderImGui() {
	LABEL_LEFT(ImGui::DragFloat, "Impulse", &_impulse, 1.0f);
}

void JumpBehaviour::OnEnteredTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger)
{
	LOG_INFO("Entered trigger: {}", trigger->GetGameObject()->Name);
	if (trigger->GetGameObject()->Name.find("Ground") != std::string::npos) _onGround = true;
}

void JumpBehaviour::OnLeavingTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger)
{
	LOG_INFO("Exited trigger: {}", trigger->GetGameObject()->Name);
	if (trigger->GetGameObject()->Name.find("Ground") != std::string::npos) _onGround = false;
}

nlohmann::json JumpBehaviour::ToJson() const {
	return {
		{ "impulse", _impulse }
	};
}

JumpBehaviour::JumpBehaviour() :
	IComponent(),
	_impulse(10.0f)
{ }

JumpBehaviour::~JumpBehaviour() = default;

JumpBehaviour::Sptr JumpBehaviour::FromJson(const nlohmann::json& blob) {
	JumpBehaviour::Sptr result = std::make_shared<JumpBehaviour>();
	result->_impulse = blob["impulse"];
	return result;
}

void JumpBehaviour::Update(float deltaTime) {
	//Find whether the attached object is on the ground
	
	//If the object has a controller connected, use that to find input
	_startingJump = false;

	if (_controller->IsValid())
	{
		if (_controller->GetButtonDown(GLFW_GAMEPAD_BUTTON_A) && _onGround)
		{
			_body->ApplyImpulse(glm::vec3(0.0f, 0.0f, _impulse));
			_startingJump = true;
		}
	}

	//Else, use the keyboard
	else
	{
		if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_SPACE) && _onGround)
		{
			_body->ApplyImpulse(glm::vec3(0.0f, 0.0f, _impulse));
			_startingJump = true;
		}
	}
}

bool JumpBehaviour::IsInAir()
{
	return !_onGround;
}

bool JumpBehaviour::IsStartingJump()
{
	return _startingJump;
}

