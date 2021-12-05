#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"


class PlayerBoomerangToss :
    public Gameplay::IComponent
{
	//-----Boomerang References-----//
	//Each of these is determined when the Boomerang is initialized
	GameObject _boomerangGO; //Reference to the player’s Boomerang Game Object in the scene.
	Rigidbody _boomerangRigidbody;
	BoomerangBehavior _boomerangBehavior;

	//-----Boomerang Properties-----//
	bool _thrown = false;
	float _projectileLaunchForce = 1000f;
	float _projectileSpacing = 1f;

	//-----Player References-----//
	Transform _cameraTransform;



};

