#include "HealthManager.h"
#include <GLFW/glfw3.h>
#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/ImGuiHelper.h"
#include "BoomerangBehavior.h"


HealthManager::HealthManager()
	: IComponent()
{ }

HealthManager::~HealthManager() = default;

void HealthManager::Awake()
{

	if (GetGameObject()->Name == "Player 1")
	{
		_playerID = 1;
		_enemyID = 2;
	}

	else
	{
		_playerID = 2;
		_enemyID = 1;
	}
}

void HealthManager::Update(float deltaTime)
{
	if (_loseHealth)
	{
		_healthVal -= 1.0f;
	}

	if (_gotHit)
	{
		_damageScreenOpacity = 1.0f;
		_gotHit = false;
	}

	else if (_damageScreenOpacity > 0.0f)
	{
		_damageScreenOpacity -= deltaTime / 1.2f;

		if (_damageScreenOpacity < 0.0f)
			_damageScreenOpacity = 0.0f;
	}

	_loseHealth = false;
}

void HealthManager::OnEnteredTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger)
{
	//LOG_INFO("Entered trigger: {}", trigger->GetGameObject()->Name);
	if (trigger->GetGameObject()->Name == "Boomerang " + std::to_string(_enemyID))
	{
		_loseHealth = true;
		std::cout << GetGameObject()->Name << " lost health, now down to " << std::to_string(_healthVal - 1) << std::endl;

		trigger->GetGameObject()->Get<BoomerangBehavior>()->returnBoomerang();
		_gotHit = true;
	}

	else if (trigger->GetGameObject()->Name == "Boomerang " + std::to_string(_playerID))
	{
		trigger->GetGameObject()->Get<BoomerangBehavior>()->makeBoomerangInactive();
	}
}

float HealthManager::GetHealth()
{
	return _healthVal;
}

float HealthManager::GetMaxHealth()
{
	return _maxHealth;
}

void HealthManager::ResetHealth()
{
	_healthVal = _maxHealth;
}

float HealthManager::GetDamageOpacity()
{
	return _damageScreenOpacity;
}

bool HealthManager::IsDead()
{
	return _healthVal == 0.0f;
}

void HealthManager::RenderImGui()
{
}

nlohmann::json HealthManager::ToJson() const
{
	return nlohmann::json();
}

HealthManager::Sptr HealthManager::FromJson(const nlohmann::json& blob)
{
	return HealthManager::Sptr();
}
