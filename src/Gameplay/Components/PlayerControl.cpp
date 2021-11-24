#include "Gameplay/Components/PlayerControl.h"
#include <GLFW/glfw3.h>
#define  GLM_SWIZZLE
#include <GLM/gtc/quaternion.hpp>

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/GameObject.h"

#include "Gameplay/Physics/RigidBody.h"

PlayerControl::PlayerControl() 
	: IComponent(),
	_mouseSensitivity({ 0.2f, 0.2f }),
	_moveSpeeds(glm::vec3(10.0f)),
	_shiftMultipler(2.0f),
	_currentRot(glm::vec2(0.0f)),
	_isMousePressed(false)
{ }

PlayerControl::~PlayerControl() = default;

void PlayerControl::Awake()
{
	_window = GetGameObject()->GetScene()->Window;

}

void PlayerControl::Update(float deltaTime)
{
	if (glfwGetMouseButton(_window, 0)) {
		if (_isMousePressed == false) {
			glfwGetCursorPos(_window, &_prevMousePos.x, &_prevMousePos.y);
		}
		_isMousePressed = true;
	}
	else {
		_isMousePressed = false;
	}

	if (_isMousePressed) {
		glm::dvec2 currentMousePos;
		glfwGetCursorPos(_window, &currentMousePos.x, &currentMousePos.y);

		_currentRot.x += static_cast<float>(currentMousePos.x - _prevMousePos.x) * _mouseSensitivity.x;
		_currentRot.y += static_cast<float>(currentMousePos.y - _prevMousePos.y) * _mouseSensitivity.y;
		glm::quat rotX = glm::angleAxis(glm::radians(_currentRot.x), glm::vec3(0, 0, 1));
		glm::quat rotY = glm::angleAxis(glm::radians(90.f), glm::vec3(1, 0, 0));
		glm::quat currentRot = rotX * rotY;
		GetGameObject()->SetRotation(currentRot);

		_prevMousePos = currentMousePos;

		glm::vec3 input = glm::vec3(0.0f);
		if (glfwGetKey(_window, GLFW_KEY_W)) {
			input.z -= _moveSpeeds.x;
		}
		if (glfwGetKey(_window, GLFW_KEY_S)) {
			input.z += _moveSpeeds.x;
		}
		if (glfwGetKey(_window, GLFW_KEY_A)) {
			input.x -= _moveSpeeds.y;
		}
		if (glfwGetKey(_window, GLFW_KEY_D)) {
			input.x += _moveSpeeds.y;
		}
		if (glfwGetKey(_window, GLFW_KEY_LEFT_CONTROL)) {
			input.y -= _moveSpeeds.z;
		}
		if (glfwGetKey(_window, GLFW_KEY_LEFT_SHIFT)) {
			input *= _shiftMultipler;
		}

		input *= deltaTime;

		glm::vec3 worldMovement = glm::vec3((currentRot * glm::vec4(input, 1.0f)).x, (currentRot * glm::vec4(input, 1.0f)).y, 0.0f);

		if (worldMovement != glm::vec3(0.0f))
		{
			worldMovement = 10.0f * glm::normalize(worldMovement);
		}
		GetGameObject()->Get<Gameplay::Physics::RigidBody>()->ApplyForce(worldMovement);
	}
}

void PlayerControl::RenderImGui()
{
}

nlohmann::json PlayerControl::ToJson() const
{
	return nlohmann::json();
}

PlayerControl::Sptr PlayerControl::FromJson(const nlohmann::json& blob)
{
	return PlayerControl::Sptr();
}
