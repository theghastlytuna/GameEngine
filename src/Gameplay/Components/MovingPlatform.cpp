#include "MovingPlatform.h"
#include "Gameplay/GameObject.h"
#include "Utils/GlmBulletConversions.h"

MovingPlatform::MovingPlatform() : 
	IComponent(),
	currentMode(LERP),
	currentInd(0),
	nextInd(0),
	duration(0.0f),
	timer(0.0f),
	forward(true)
{ }

MovingPlatform::~MovingPlatform() = default;

void MovingPlatform::Awake()
{
}

void MovingPlatform::Update(float deltaTime)
{
	if (nodes.size() == 0 || duration == 0) return;
	
	else
		switch (currentMode)
		{
		case LERP:
		{
			timer += deltaTime;

			t = timer / duration;

			glm::vec3 point0;
			glm::vec3 point1;

			if (t > 1)
			{
				timer = 0.0f;
				t = 0.0f;

				if (forward)
				{
					currentInd++;

					if (currentInd >= nodes.size() - 1)
					{
						forward = false;
					}

				}

				else
				{
					currentInd--;

					if (currentInd == 0)
					{
						forward = true;
					}
				}
			}
				
			if (forward)
			{
				point0 = nodes[currentInd];
				point1 = nodes[currentInd + 1];
			}

			else
			{
				point0 = nodes[currentInd];
				point1 = nodes[currentInd - 1];
			}
			
			this->GetGameObject()->SetPosition(Lerp(point0, point1));
			
		}
		break;

		case CATMULL:
		{
			timer += deltaTime;

			t = timer / duration;

			if (t > 1)
			{
				timer = 0.0f;
				t = 0.0f;

				if (forward)
				{
					currentInd++;

					if (currentInd + 1 >= nodes.size())
					{
						forward = false;
					}
				}

				else
				{
					currentInd--;

					if (currentInd == 0)
					{
						forward = true;
					}
				}
			}

			if (nodes.size() < 4) return;

			glm::vec3 point0;
			glm::vec3 point1;
			glm::vec3 point2;
			glm::vec3 point3;

			if (forward)
			{
				point0 = nodes[(currentInd == 0) ? nodes.size() - 1 : currentInd - 1];
				point1 = nodes[currentInd];
				point2 = nodes[(currentInd + 1) % nodes.size()];
				point3 = nodes[(currentInd + 2) % nodes.size()];
			}

			else
			{
				point0 = nodes[(currentInd == nodes.size() - 1) ? 0 : currentInd + 1];
				point1 = nodes[currentInd];
				point2 = nodes[(glm::abs(currentInd - 1)) % nodes.size()];
				point3 = nodes[(glm::abs(currentInd - 2)) % nodes.size()];
			}

			this->GetGameObject()->SetPosition(Catmull(point0, point1, point2, point3));
		}
		break;

		case BEZIER:
		{
			timer += deltaTime;

			t = timer / duration;

			if (t > 1)
			{
				timer = 0.0f;
				t = 0.0f;

				if (forward)
				{
					currentInd++;

					if (currentInd + 3 >= nodes.size())
					{
						forward = false;
					}
				}

				else
				{
					currentInd--;

					if (currentInd == 0)
					{
						forward = true;
					}
				}
			}

			if (nodes.size() < 4) return;

			glm::vec3 point0;
			glm::vec3 point1;
			glm::vec3 point2;
			glm::vec3 point3;

			if (forward)
			{
				
				point0 = nodes[currentInd];
				point1 = nodes[currentInd + 1];
				point2 = nodes[currentInd + 2];
				point3 = nodes[currentInd + 3];
			}

			else
			{
				point0 = nodes[currentInd + 2];
				point1 = nodes[currentInd + 1];
				point2 = nodes[currentInd];
				point3 = nodes[currentInd - 1];
			}

			this->GetGameObject()->SetPosition(Catmull(point0, point1, point2, point3));
		}
		break;
	}

}

void MovingPlatform::SetMode(MovementMode inMode)
{
	timer = 0.0f;
	currentMode = inMode;
	currentInd = 0;
	nextInd = 1;
}

void MovingPlatform::SetNodes(std::vector<glm::vec3> inNodes, float inDuration)
{
	nodes = inNodes;
	duration = inDuration;
}

glm::vec3 MovingPlatform::Lerp(glm::vec3 p0, glm::vec3 p1)
{
	return (1.0f - t) * p0 + t * p1;
}

glm::vec3 MovingPlatform::Catmull(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
{
	return 0.5f * (2.0f * p1 + t * (-p0 + p2)
		+ t * t * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3)
		+ t * t * t * (-p0 + 3.0f * p1 - 3.0f * p2 + p3));
}

glm::vec3 MovingPlatform::Bezier(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2, glm::vec3 p3)
{
	return ((p0)+t * (3.0f * -p0 + 3.0f * p1)
		+ t * t * (3.0f * p0 - 6.0f * p1 + 3.0f * p2)
		+ t * t * t * (-p0 + 3.0f * p1 - 3.0f * p2 + p3));
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
