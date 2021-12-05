#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/GameObject.h"

class BoomerangBehavior :
    public Gameplay::IComponent
{
private:
    //-----External References-----//
    Gameplay::Physics::RigidBody* _rigidBody; //Reference to the Boomerang's own rb
    Gameplay::GameObject* _player; //This is the m8 who frew da boomiewang (for return)
    Gameplay::GameObject* _targetEntity; //This is the cunt you locked unto innit (target tracking)
    Gameplay::GameObject* _boomerangEntity; //The freekin' wang 'imself
    glm::vec3 _targetPoint; //This is the position we are tracking to (point tracking)

    enum class boomerangState {
        FORWARD = 0,
        POINTTRACK = 1, //Will chase after a point in 3D space
        LOCKTRACK = 2, //Will chase an entity, automatically updating the 3D position
        //of that target as it goes
        RETURNING = 3, //Chases the player so that it can become inactive
        INACTIVE = 4 //Ready to be thrown again
    };

    //-----Boomerang Properies-----//
    float _boomerangAcceleration = 5.f;
    bool _targetLocked = false;
    bool _returning = false;
    boomerangState _state = boomerangState::FORWARD;
    glm::vec3 _inactivePosition = glm::vec3(0.f, 0.f, -100.f);
    float _projectileSpacing = 1.f;
    float _boomerangLaunchForce;

    //Constructor and Destructor//
    //Do these even so anything?
    BoomerangBehavior();
    ~BoomerangBehavior();

    /// <summary>
    /// Seeks the _targetPoint. This always set before the seek function is called.
    /// </summary>
    void Seek();

    void returnBoomerang();
    void makeBoomerangInactive();

public:
    //IComponent overrides
    void Awake() override;
    void Update(float deltaTime) override;
    void RenderImGui() override;
    void OnCollisionEnter();

    /// <summary>
    /// Used when the player is initially throwing out the boomerang.
    /// </summary>
    /// <param name="playerPosition">The player's position in world space.
    /// Projecile spacing is provided by this function.</param>
    /// <param name="cameraRotation">The camera's rotation in a euler vec3</param>
    void throwWang(glm::vec3 playerPosition, glm::vec3 cameraRotation);

    /// <summary>
    /// Steers the projecile towards this new point in 3D Space.
    /// Does nothing if the boomerang is locked on
    /// </summary>
    /// <param name="newTarget">Point in 3D space. Should be where the raycast hits if 
    /// the raycast didn't hit a valid target.</param>
    void UpdateTarget(glm::vec3 newTarget);

    /// <summary>
    /// Locks the Boomerang's target to a player if targetted. Player controller should determine
    /// if this is function we want to use and if the game object is a valid target
    /// </summary>
    /// <param name="targetEntity">The cunt we're tracking</param>
    void LockTarget(Gameplay::GameObject* targetEntity);

    /// <summary>
    /// Changes how fast the Boomerang can accelerate.
    /// </summary>
    /// <param name="newAccel"></param>
    void SetAcceleration(float newAccel);

    /// <summary>
    /// Sets where the wang will go when it's chilling.
    /// </summary>
    /// <param name="newPosition">Where's it going???</param>
    void SetInactivePosition(glm::vec3 newPosition);
};

