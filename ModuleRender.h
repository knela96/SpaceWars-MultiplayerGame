#pragma once

struct Texture;

struct Sprite
{
	GameObject *gameObject = nullptr;
	vec2 pivot = vec2{ 0.5f, 0.5f };             // NOTE(jesus): 0.5 means centered
	vec4 color = vec4{ 1.0f, 1.0f, 1.0f, 1.0f }; // NOTE(jesus): Color to tint the sprite
	Texture * texture = nullptr;                 // NOTE(jesus): Texture with the actual image
	int  order = 0;                              // NOTE(jesus): determines the drawing order
};

const uint8 MAX_ANIMATION_CLIP_FRAMES = 25;

struct AnimationClip
{
	vec4 frameRect[MAX_ANIMATION_CLIP_FRAMES];
	float frameTime = 0.1f;
	uint16 id = 0;
	uint8 frameCount = 0;
	bool loop = false;

	void addFrameRect(vec4 rect)
	{
		frameRect[frameCount++] = rect;
	}
};

struct Animation
{
	GameObject *gameObject = nullptr;
	AnimationClip *clip = nullptr;
	float elapsedTime = 0.0f;
	uint8 currentFrame = 0;

	void update(float deltaTime)
	{
		elapsedTime = elapsedTime + deltaTime;
		currentFrame = (uint8)min(clip->frameCount - 1, elapsedTime / clip->frameTime);

		float duration = clip->frameCount * clip->frameTime;
		while (clip->loop && elapsedTime > duration)
		{
			elapsedTime -= duration;
			currentFrame = 0;
		}
	}

	void rewind()
	{
		elapsedTime = 0.0f;
		currentFrame = 0;
	}

	vec4 currentFrameRect() const
	{
		return clip->frameRect[currentFrame];
	}

	bool finished() const
	{
		return !clip->loop && elapsedTime > clip->frameCount * clip->frameTime;
	}
};

class ModuleRender : public Module
{
public:

	// Virtual functions

	bool init() override;

	bool update() override;

	bool postUpdate() override;

	bool cleanUp() override;


	// Public methods

	Sprite *addSprite(GameObject *parent);

	void removeSprite(GameObject * parent);

	Animation *addAnimation(GameObject *parent);

	void removeAnimation(GameObject *parent);

	AnimationClip *addAnimationClip();

	AnimationClip *getAnimationClip(uint16 animationClipId);

	void removeAnimationClip(AnimationClip *clip);

	void resizeBuffers(unsigned int width, unsigned int height);

	void present();


	// Attributes

	vec2 cameraPosition = {};

	bool mustRenderColliders = false;


private:

	void renderScene();

	bool CreateDeviceD3D(HWND hWnd);
	void CleanupDeviceD3D();
	void CreateRenderTarget();
	void CleanupRenderTarget();

	Texture * whitePixel = nullptr;
	Texture * blackPixel = nullptr;

	GameObject* orderedGameObjects[MAX_GAME_OBJECTS] = {};

	uint8 shaderSource[Kilobytes(128)];

	uint32 spriteCount = 0;
	Sprite sprites[MAX_GAME_OBJECTS] = {};

	uint32 animationCount = 0;
	Animation animations[MAX_GAME_OBJECTS] = {};

	AnimationClip animationClips[MAX_ANIMATION_CLIPS] = {};
};

