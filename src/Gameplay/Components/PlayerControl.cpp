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
	_isMousePressed(false),
	_isMoving(false),
	_isSprinting(false),
	_spintVal(2.5f),
	_controllerSensitivity({1.1f, 1.1f})
{ }

PlayerControl::~PlayerControl() = default;

void PlayerControl::Awake()
{
	_window = GetGameObject()->GetScene()->Window;

	_controller = GetComponent<ControllerInput>();

	if (_controller == nullptr)
	{
		IsEnabled = false;
	}
	
	if (GetGameObject()->Name == "Player 1") {
		playerID = 1;
	}
	else {
		playerID = 2;
	}

	_boomerang = GetGameObject()->GetScene()->FindObjectByName("Boomerang " + std::to_string(playerID));
	if (_boomerang->Has<BoomerangBehavior>()) {
		_boomerangBehavior = _boomerang->Get<BoomerangBehavior>();
	}
	else {
		std::cout << "Ayo the pizza here" << std::endl;
	}

	if (playerID == 1) {
		_camera = GetGameObject()->GetScene()->PlayerCamera;
	}
	else {
		_camera = GetGameObject()->GetScene()->PlayerCamera2;
	}
}

void PlayerControl::Update(float deltaTime)
{
	//If there is a valid controller connected, then use it to find input
	if (_controller->IsValid())
	{
		_isMoving = false;
		_justThrew = false;

		bool Wang = _controller->GetButtonDown(GLFW_GAMEPAD_BUTTON_X);
		bool Point = _controller->GetButtonDown(GLFW_GAMEPAD_BUTTON_Y);
		bool Target = _controller->GetButtonDown(GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER);
		bool returnaloni = _controller->GetButtonDown(GLFW_GAMEPAD_BUTTON_LEFT_BUMPER);

		float leftX = _controller->GetAxisValue(GLFW_GAMEPAD_AXIS_LEFT_X);
		float leftY = _controller->GetAxisValue(GLFW_GAMEPAD_AXIS_LEFT_Y);

		float rightX = _controller->GetAxisValue(GLFW_GAMEPAD_AXIS_RIGHT_X);
		float rightY = _controller->GetAxisValue(GLFW_GAMEPAD_AXIS_RIGHT_Y);

		float leftTrigger = _controller->GetAxisValue(GLFW_GAMEPAD_AXIS_LEFT_TRIGGER);
		float rightTrigger = _controller->GetAxisValue(GLFW_GAMEPAD_AXIS_RIGHT_TRIGGER);

		_isSprinting = _controller->GetButtonDown(GLFW_GAMEPAD_BUTTON_B);

		//Since controller joysticks are physical, they often won't be perfect, meaning at a neutral state it might still have slight movement.
		//Check to make sure that the axes aren't outputting an extremely small number, and if they are then set the input to 0.
		if (rightX > 0.2 || rightX < -0.2) _currentRot.x += static_cast<float>(rightX) * _controllerSensitivity.x;	
		if (rightY > 0.2 || rightY < -0.2) _currentRot.y += static_cast<float>(rightY) * _controllerSensitivity.y;

		glm::quat rotX = glm::angleAxis(glm::radians(-_currentRot.x), glm::vec3(0, 0, 1));
		glm::quat rotY = glm::angleAxis(glm::radians(90.0f), glm::vec3(1, 0, 0));

		glm::quat currentRot = rotX * rotY;

		GetGameObject()->SetRotation(currentRot);

		glm::vec3 input = glm::vec3(0.0f);

		//Since controller joysticks are physical, they often won't be perfect, meaning at a neutral state it might still have slight movement.
		//Check to make sure that the axes aren't outputting an extremely small number, and if they are then set the input to 0.
		if (leftY < 0.2 && leftY > -0.2) input.z = 0.0f;
		else
		{
			_isMoving = true;
			input.z = leftY * -_moveSpeeds.x;
		}

		if (leftX < 0.2 && leftX > -0.2) input.x = 0.0f;
		else
		{
			_isMoving = true;
			input.x = leftX * -_moveSpeeds.y;
		}

		input *= deltaTime;

		glm::vec3 worldMovement = glm::vec3((currentRot * glm::vec4(input, 1.0f)).x, (currentRot * glm::vec4(input, 1.0f)).y, 0.0f);

		if (worldMovement != glm::vec3(0.0f))
		{
			worldMovement = 10.0f * glm::normalize(worldMovement);

			if (_isSprinting) worldMovement *= _spintVal;
		}
		GetGameObject()->Get<Gameplay::Physics::RigidBody>()->ApplyForce(worldMovement);

		if (Wang) {
			if (_boomerangBehavior->getReadyToThrow()) {
				_boomerangBehavior->throwWang(GetGameObject()->GetPosition(), playerID);
				_justThrew = true;
			}
			else if (Point){
				glm::vec3 cameraLocalForward = glm::vec3(_camera->GetView()[0][2], _camera->GetView()[1][2], _camera->GetView()[2][2]) * -1.0f;
				btVector3 btCamLocF = btVector3(cameraLocalForward.x, cameraLocalForward.y, cameraLocalForward.z);
				glm::vec3 playerPosition = GetGameObject()->GetPosition();
				btVector3 btPlayerPosition = btVector3(playerPosition.x, playerPosition.y, playerPosition.z);
				btCollisionWorld::ClosestRayResultCallback result(btPlayerPosition, btPlayerPosition * btCamLocF * 100);
				GetGameObject()->GetScene()->GetPhysicsWorld()->rayTest(btPlayerPosition, btPlayerPosition * btCamLocF * 100, result);
				if (result.hasHit()) {
					glm::vec3 point = glm::vec3(result.m_hitPointWorld.x(), result.m_hitPointWorld.y(), result.m_hitPointWorld.z());
					_boomerangBehavior->UpdateTarget(point);
				}
			}
			else if (Target) {
				if (playerID == 1) {
					//Tracks Player 2
					_boomerangBehavior->LockTarget(GetGameObject()->GetScene()->FindObjectByName("Player 2"));
				}
				else {
					//Tracks Player 1
					_boomerangBehavior->LockTarget(GetGameObject()->GetScene()->FindObjectByName("Player 1"));
				}
			}
		}
		if (returnaloni) {
			_boomerangBehavior->makeBoomerangInactive();
		}
	}

	//Else, use KBM
	else
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
}

bool PlayerControl::IsMoving()
{
	return _isMoving;
}

bool PlayerControl::IsSprinting()
{
	return _isSprinting;
}

bool PlayerControl::GetJustThrew()
{
	return _justThrew;
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