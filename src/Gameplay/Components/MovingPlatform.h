#pragma once
#include "IComponent.h"
#include "Gameplay/Physics/RigidBody.h"
#include "Gameplay/Physics/TriggerVolume.h"


class MovingPlatform : public Gameplay::IComponent {
public:
	typedef std::shared_ptr<MovingPlatform> Sptr;

	MovingPlatform();
	virtual ~MovingPlatform();
	virtual void Awake() override;
	virtual void Update(float deltaTime) override;
	virtual void RenderImGui() override;
	virtual void OnEnteredTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger) override;
	virtual void OnLeavingTrigger(const std::shared_ptr<Gameplay::Physics::TriggerVolume>& trigger) override;
	MAKE_TYPENAME(MovingPlatform);
	virtual nlohmann::json ToJson() const override;
	static MovingPlatform::Sptr FromJson(const nlohmann::json& blob);

	glm::vec3 startPos;
	glm::vec3 endPos;
	float duration;
	bool forward;
protected:
	float t;
};