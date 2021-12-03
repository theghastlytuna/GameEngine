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

	enum MovementMode
	{
		LERP = 0,
		CATMULL = 1,
		BEZIER = 2
	};

	void SetMode(MovementMode inMode);

	void SetNodes(std::vector<glm::vec3> inNodes, float inDuration);

protected:
	float timer;
	float t;
	float duration;
	bool forward;

	glm::vec3 Lerp(glm::vec3 p0, glm::vec3 p1);

	glm::vec3 Catmull(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);

	glm::vec3 Bezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);

	std::vector<glm::vec3> nodes;

	MovementMode currentMode;

	int currentInd;
	int nextInd;
};