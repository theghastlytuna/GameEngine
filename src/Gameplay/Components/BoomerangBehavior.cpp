#include "BoomerangBehavior.h"

BoomerangBehavior::~BoomerangBehavior()
{
}

void BoomerangBehavior::Awake()
{
	//Set up references
	_player = GetGameObject();
	_rigidBody = this->GetComponent<Gameplay::Physics::RigidBody>();
	//Why is this broken?
}

void BoomerangBehavior::Update(float deltaTime)
{
	switch (_state) {
	case(boomerangState::POINTTRACK):
		Seek();
		break;
	case(boomerangState::LOCKTRACK):
		LockTarget(_targetEntity);
		Seek();
		break;
	case(boomerangState::RETURNING):
		LockTarget(_player);
		Seek();
		break;
	default:
		break;
	}
}

void BoomerangBehavior::RenderImGui()
{
}

void BoomerangBehavior::Seek()
{
	glm::vec3 desiredVector = _targetPoint - _boomerangEntity->GetPosition();
	glm::vec3 currentVector = _rigidBody->GetLinearVelocity();

	desiredVector = glm::normalize(desiredVector);
	currentVector = glm::normalize(currentVector);

	glm::vec3 appliedVector = (desiredVector - currentVector);
	appliedVector = glm::normalize(appliedVector) * _boomerangAcceleration; // * timer::deltaTime;
	_rigidBody->ApplyForce(appliedVector);

	//TODO: Limit Angle of the applied vector to enforce turning speeds?
	//Might make it more interesting to control
}

void BoomerangBehavior::throwWang(glm::vec3 playerPosition, glm::vec3 cameraRotation)
{
	_state = boomerangState::FORWARD;
	_targetLocked = false;
	_returning = false;
	glm::vec3 cameraLocalForward = glm::vec3(0, 1, 0);
	//TODO: Get the camera's local forward vector
	_boomerangEntity->SetPosition(playerPosition + cameraLocalForward * _projectileSpacing);
	_boomerangEntity->SetRotation(cameraRotation);
	_rigidBody->ApplyImpulse(cameraLocalForward * _boomerangLaunchForce);
}

void BoomerangBehavior::UpdateTarget(glm::vec3 newTarget)
{
	_targetPoint = newTarget;
	if (!_targetLocked) {
		_state = boomerangState::POINTTRACK;
	}
}

void BoomerangBehavior::LockTarget(Gameplay::GameObject* targetEntity)
{
	_targetEntity = targetEntity;
	_targetLocked = true;
	if (!_returning) {
		_state = boomerangState::LOCKTRACK;
	}
}

void BoomerangBehavior::returnBoomerang()
{
	_returning = true;
	_targetLocked = true;
	_state = boomerangState::RETURNING;
}

void BoomerangBehavior::SetAcceleration(float newAccel)
{
	_boomerangAcceleration = newAccel;
}

void BoomerangBehavior::SetInactivePosition(glm::vec3 newPosition)
{
	_inactivePosition = newPosition;
}

void BoomerangBehavior::OnCollisionEnter()
{
	_state = boomerangState::RETURNING;

	//Check if this is the owner, set state to
	//Check if this is a player and do some DAMAGE
}

void BoomerangBehavior::makeBoomerangInactive()
{
	_boomerangEntity->SetPosition(_inactivePosition);
	_state = boomerangState::INACTIVE;
	_rigidBody->SetLinearVelocity(glm::vec3(0, 0, 0));
}
