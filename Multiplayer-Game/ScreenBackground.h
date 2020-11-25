#pragma once

class ScreenBackground : public Screen
{
	void enable() override;

	void update() override;

	void disable() override;

	GameObject *background = nullptr;
};
