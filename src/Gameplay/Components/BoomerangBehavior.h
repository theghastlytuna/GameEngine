#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"

class BoomerangBehavior :
    public Gameplay::IComponent
{
private:
    struct Transform
    {
        glm::vec3 position;
        glm::vec3 rotation;
        glm::vec3 scale;
    };
    
    Gameplay::Physics::RigidBody _rigidBody;
    Transform _playerTransform;
    Transform _targetEntityTransform;
    //BoomerangToss _toss; //Reference to the Boomerang Toss Component of the player who threw it

    bool _targetLocked = false;
    bool _returning = false;
    float _boomerangAcceleration = 5.f;
    glm::vec3 _targetPosition;

    BoomerangBehavior();
    ~BoomerangBehavior();

    void FixedUpdate();
    void Update();

    /// <summary>
    /// Seeks whatever target is given to it
    /// </summary>
    /// <param name="Target"></param>
    void Seek(glm::vec3 Target);

    void UpdateTarget(glm::vec3 newTarget);
    void UpdateTarget(Transform targetEntityTransform);

    void OnCollisionEnter();
    void returnBoomerang();
};

