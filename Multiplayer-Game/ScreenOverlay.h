#pragma once

class ScreenOverlay : public Screen
{
public:

	Screen *oldScene = nullptr;
	Screen *newScene = nullptr;

private:

	void enable() override;

	void update() override;

	void disable() override;

	GameObject *overlay = nullptr;

	float transitionTimeElapsed = 0.0f;
	float transitionTimeMax = SCENE_TRANSITION_TIME_SECONDS;
};
