#include "MorphAnimator.h"

#include "Gameplay/GameObject.h"
#include "Gameplay/Scene.h"
#include "Utils/JsonGlmHelpers.h"
#include "Utils/ImGuiHelper.h"

MorphAnimator::MorphAnimator()
	: IComponent(),
	switchClip(false),
	timer(0.0f)
{ }

MorphAnimator::~MorphAnimator() = default;

void MorphAnimator::Awake()
{
	thisObject = this->GetComponent<RenderComponent>()->GetMesh();
}

void MorphAnimator::Update(float deltaTime)
{

	if (switchClip)
	{
		timer = 0.0f;
		switchClip = false;
	}

	else
	{
		timer += deltaTime;
		
	}

	float t = timer / currentClip.frameDuration;

	if (t > 1)
	{
		t = 0;
		timer = 0.0f;
		currentClip.currentFrame++;
		currentClip.nextFrame++;
		if (currentClip.currentFrame == currentClip.frames.size())
		{
			currentClip.currentFrame = 0;
		}

		else if (currentClip.nextFrame == currentClip.frames.size())
		{
			currentClip.nextFrame = 0;
		}
	}

	//This shit whack
	std::vector<BufferAttribute> pos0 = currentClip.frames[currentClip.currentFrame]->Mesh->GetBufferBinding(AttribUsage::Position)->Attributes;
	std::vector<BufferAttribute> pos1 = currentClip.frames[currentClip.nextFrame]->Mesh->GetBufferBinding(AttribUsage::Position)->Attributes;

	//Resize the buffer attributes of the first frame to only hold the position
	pos0.resize(1);

	//Change the position's slot to 4, which is equal to inPosition2 in the shader
	pos1[0].Slot = static_cast<GLint>(4);
	//Resize this frame's buffer attributes to only hold the position as well
	pos1.resize(1);

	thisObject->AddVertexBuffer(currentClip.frames[currentClip.currentFrame]->Mesh->GetBufferBinding(AttribUsage::Position)->Buffer, pos0);
	thisObject->AddVertexBuffer(currentClip.frames[currentClip.nextFrame]->Mesh->GetBufferBinding(AttribUsage::Position)->Buffer, pos1);

	//Pass the lerp param as a uniform
	this->GetComponent<RenderComponent>()->GetMaterial()->Set("t", t);
}

void MorphAnimator::AddClip(std::vector<Gameplay::MeshResource::Sptr> inFrames, float dur, std::string inName)
{
	animInfo clip;

	//Make a temporary string
	std::string tempStr = "";

	//Go through the input name, convert it to lowercase, and add each lowercase character to tempStr
	//Note: converted to lowercase to make it easier to search for names
	for (int i = 0; i < inName.length(); i++)
	{
		tempStr += std::tolower(inName[i]);
	}

	//Assign all the variables
	clip.animName = tempStr;
	clip.frames = inFrames;
	clip.frameDuration = dur;
	clip.currentFrame = 0;
	
	if (clip.frames.size() == 0) clip.nextFrame = 0;
	else clip.nextFrame = 1;

	animClips.push_back(clip);
}

void MorphAnimator::ActivateAnim(std::string name)
{
	std::string tempStr = "";

	for (int i = 0; i < name.length(); i++)
	{
		tempStr += std::tolower(name[i]);
	}

	for (int j = 0; j < animClips.size(); j++)
	{
		if (animClips[j].animName == tempStr)
		{
			currentClip = animClips[j];
			switchClip = true;
			return;
		}
	}

	std::cout << "No animation clip of this name: " << tempStr << std::endl;
}

void MorphAnimator::RenderImGui()
{
}

nlohmann::json MorphAnimator::ToJson() const
{
	return nlohmann::json();
}

MorphAnimator::Sptr MorphAnimator::FromJson(const nlohmann::json& blob)
{
	return MorphAnimator::Sptr();
}
