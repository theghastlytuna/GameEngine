#include "Gameplay/Components/JumpBehaviour.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/Components/MaterialSwapBehaviour.h"

void JumpBehaviour::Awake()
{
	_body = GetComponent<Gameplay::Physics::RigidBody>();
	if (_body == nullptr) {
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
	if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_SPACE) && _onGround) 
	{
		_body->ApplyForce(glm::vec3(0.0f, 0.0f, _impulse));
	}
	if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_W) && _onGround)
	{
		_body->ApplyForce(this->GetGameObject()->GetRotation() * glm::vec3(0.f, 10.f, 0.f));
	}
	if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_A) && _onGround)
	{
		_body->ApplyForce(this->GetGameObject()->GetRotation() * glm::vec3(-10.f, 0.f, 0.f));
	}
	if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_S) && _onGround)
	{
		_body->ApplyForce(this->GetGameObject()->GetRotation() * glm::vec3(0.f, -10.f, 0.f));
	}
	if (glfwGetKey(GetGameObject()->GetScene()->Window, GLFW_KEY_D) && _onGround)
	{
		_body->ApplyForce(this->GetGameObject()->GetRotation() * glm::vec3(10.f, 0.f, 0.f));
	}

}

