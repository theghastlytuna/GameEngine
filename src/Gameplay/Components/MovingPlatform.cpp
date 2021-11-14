#include "MovingPlatform.h"
#include "Gameplay/GameObject.h"
#include "Utils/GlmBulletConversions.h"

MovingPlatform::MovingPlatform() : 
	IComponent()
{ }

MovingPlatform::~MovingPlatform() = default;

void MovingPlatform::Awake()
{
}

void MovingPlatform::Update(float deltaTime)
{
	if (forward)
	{
		if (t < 1)
		{
			this->GetGameObject()->SetPostion(ToGlm(lerp(ToBt(startPos), ToBt(endPos), t)));
			t += deltaTime / duration;
		}
		else if (t >= 1)
		{
			t = 0;
			forward = false;
		}
	}
	else
	{
		if (t < 1)
		{
			this->GetGameObject()->SetPostion(ToGlm(lerp(ToBt(endPos), ToBt(startPos), t)));
			t += deltaTime / duration;
		}
		else if (t >= 1)
		{
			t = 0;
			forward = true;
		}

	}

}
void MovingPlatform::RenderImGui()
{
}

void MovingPlatform::OnEnteredTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger)
{
}

void MovingPlatform::OnLeavingTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger)
{
}

nlohmann::json MovingPlatform::ToJson() const
{
	return nlohmann::json();
}

MovingPlatform::Sptr MovingPlatform::FromJson(const nlohmann::json& blob)
{
	return MovingPlatform::Sptr();
}
