#include "Gameplay/Components/ControllerInput.h"
#include <GLFW/glfw3.h>
#define  GLM_SWIZZLE
#include <GLM/gtc/quaternion.hpp>

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"

ControllerInput::ControllerInput()
	: IComponent(),
	_controllerConnected(false)
{ }

ControllerInput::~ControllerInput() = default;

void ControllerInput::Awake()
{
	_window = GetGameObject()->GetScene()->Window;
}

void ControllerInput::Update(float deltaTime)
{

	_controllerConnected = glfwJoystickPresent(_controllerID);
}

void ControllerInput::RenderImGui()
{
}

nlohmann::json ControllerInput::ToJson() const
{
	return nlohmann::json();
}

ControllerInput::Sptr ControllerInput::FromJson(const nlohmann::json& blob)
{
	return ControllerInput::Sptr();
}

bool ControllerInput::IsValid()
{
	return _controllerConnected;
}

void ControllerInput::SetController(int ID)
{
	_controllerID = ID;
	_controllerConnected = glfwJoystickPresent(ID);
}

bool ControllerInput::GetButtonDown(int ID)
{
	auto buttons = glfwGetJoystickButtons(_controllerID, &_buttonCount);

	return buttons[ID];
}

float ControllerInput::GetAxisValue(int ID)
{
	auto axes = glfwGetJoystickAxes(_controllerID, &_axesCount);

	return axes[ID];
}