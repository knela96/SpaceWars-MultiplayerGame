#pragma once

class ScreenLoading : public Screen
{
	void enable() override;

	void update() override;

	void disable() override;

	static const int BAR_COUNT = 12;
	GameObject *loadingBars[BAR_COUNT];
};
