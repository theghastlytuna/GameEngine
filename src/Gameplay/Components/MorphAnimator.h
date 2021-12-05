#pragma once
#include "IComponent.h"
#include "Gameplay/GameObject.h"
#include "Gameplay/Components/RenderComponent.h"

class MorphAnimator :
    public Gameplay::IComponent
{
public:
	typedef std::shared_ptr<MorphAnimator> Sptr;

	MorphAnimator();
	virtual ~MorphAnimator();

	virtual void Awake() override;
	virtual void Update(float deltaTime) override;

	void AddClip(std::vector<Gameplay::MeshResource::Sptr> inFrames, float dur, std::string inName);

	void ActivateAnim(std::string name);

	bool IsEndOfClip();

	std::string GetActiveAnim();

	//Holds the info for an animation clip
	struct animInfo
	{
		//std::vector<Gameplay::GameObject::Sptr> frames;

		std::vector<Gameplay::MeshResource::Sptr> frames;

		int currentFrame;
		int nextFrame;
		float frameDuration;

		std::string animName;
	};

	std::vector<animInfo> animClips;

public:

	virtual void RenderImGui() override;
	MAKE_TYPENAME(MorphAnimator);
	virtual nlohmann::json ToJson() const override;
	static MorphAnimator::Sptr FromJson(const nlohmann::json& blob);

protected:

	VertexArrayObject::Sptr thisObject;

	animInfo currentClip;

	float timer;

	bool switchClip;
	bool reachedEnd = false;
};

