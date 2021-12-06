#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/TriggerVolume.h"
#include "Gameplay/Scene.h"

struct GLFWwindow;

class HealthManager :
    public Gameplay::IComponent
{

public:
	typedef std::shared_ptr<HealthManager> Sptr;

	HealthManager();
	virtual ~HealthManager();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	float GetHealth();
	float GetMaxHealth();

	void ResetHealth();

	float GetDamageOpacity();

	bool IsDead();

public:
	virtual void RenderImGui() override;

	virtual void OnEnteredTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger) override;

	MAKE_TYPENAME(HealthManager);
	virtual nlohmann::json ToJson() const override;
	static HealthManager::Sptr FromJson(const nlohmann::json& blob);

protected:
	float _healthVal = 3.0f;
	float _maxHealth = 3.0f;
	float _damageScreenOpacity = 0.0f;
	bool _loseHealth = false;
	bool _gotHit = false;
	int _playerID;
	int _enemyID;

	Gameplay::GameObject::Sptr _player;
};


