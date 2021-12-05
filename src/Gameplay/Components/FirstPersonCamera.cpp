#include "Gameplay/Components/FirstPersonCamera.h"
#include <GLFW/glfw3.h>
#define  GLM_SWIZZLE
#include <GLM/gtc/quaternion.hpp>

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"
#include "Gameplay/GameObject.h"

#include "Gameplay/Physics/RigidBody.h"

FirstPersonCamera::FirstPersonCamera()
	: IComponent(),
	_mouseSensitivity({ 0.2f, 0.2f }),
	_moveSpeeds(glm::vec3(10.0f)),
	_shiftMultipler(2.0f),
	_currentRot(glm::vec2(0.0f, 180.0f)),
	_isMousePressed(false),
	_controllerSensitivity({ 1.1f, 1.1f })

{}

FirstPersonCamera::~FirstPersonCamera() = default;

void FirstPersonCamera::Awake()
{
	_window = GetGameObject()->GetScene()->Window;

	_controller = GetComponent<ControllerInput>();
	
	if (_controller == nullptr)
	{
		IsEnabled = false;
	}
	
}

void FirstPersonCamera::Update(float deltaTime)
{
	if (_controller->IsValid())
	{
		float rightX = _controller->GetAxisValue(GLFW_GAMEPAD_AXIS_RIGHT_X);
		float rightY = _controller->GetAxisValue(GLFW_GAMEPAD_AXIS_RIGHT_Y);

		//Since controller joysticks are physical, they often won't be perfect, meaning at a neutral state it might still have slight movement.
		//Check to make sure that the axes aren't outputting an extremely small number, and if they are then set the input to 0.
		if (rightX > 0.2 || rightX < -0.2) _currentRot.x += static_cast<float>(rightX) * _controllerSensitivity.x;
		if (rightY > 0.2 || rightY < -0.2) _currentRot.y += static_cast<float>(rightY) * _controllerSensitivity.y;

		glm::quat rotX = glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1));
		glm::quat rotY = glm::angleAxis(glm::radians(-_currentRot.y), glm::vec3(1, 0, 0));

		glm::quat currentRot = rotX * rotY;

		GetGameObject()->SetRotation(currentRot);
	}

	//Else, use KBM
	else {
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
			glm::quat rotX = glm::angleAxis(glm::radians(180.0f), glm::vec3(0, 0, 1));
			glm::quat rotY = glm::angleAxis(glm::radians(_currentRot.y), glm::vec3(1, 0, 0));
			glm::quat currentRot = rotX * rotY;
			GetGameObject()->SetRotation(currentRot);

			_prevMousePos = currentMousePos;
		}
	}
}

void FirstPersonCamera::RenderImGui()
{
	LABEL_LEFT(ImGui::DragFloat2, "Mouse Sensitivity", &_mouseSensitivity.x, 0.01f);
	LABEL_LEFT(ImGui::DragFloat3, "Move Speed       ", &_moveSpeeds.x, 0.01f, 0.01f);
	LABEL_LEFT(ImGui::DragFloat, "Shift Multiplier ", &_shiftMultipler, 0.01f, 1.0f);
}

nlohmann::json FirstPersonCamera::ToJson() const
{
	return {
		{ "mouse_sensitivity", GlmToJson(_mouseSensitivity) },
		{ "move_speed", GlmToJson(_moveSpeeds) },
		{ "shift_mult", _shiftMultipler }
	};
}

FirstPersonCamera::Sptr FirstPersonCamera::FromJson(const nlohmann::json& blob)
{
	FirstPersonCamera::Sptr result = std::make_shared<FirstPersonCamera>();
	result->_mouseSensitivity = ParseJsonVec2(blob["mouse_sensitivity"]);
	result->_moveSpeeds = ParseJsonVec3(blob["move_speed"]);
	result->_shiftMultipler = JsonGet(blob, "shift_mult", 2.0f);
	return result;
}
