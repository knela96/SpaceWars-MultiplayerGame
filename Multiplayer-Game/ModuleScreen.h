#pragma once

class ScreenOverlay;
class ScreenLoading;
class ScreenBackground;
class ScreenMainMenu;
class ScreenGame;

class ModuleScreen : public Module
{
public:

	bool init() override;

	bool update() override;

	bool gui() override;

	bool cleanUp() override;

	ScreenOverlay    *screenOverlay = nullptr;
	ScreenLoading    *screenLoading = nullptr;
	ScreenBackground *screenBackground = nullptr;
	ScreenMainMenu   *screenMainMenu = nullptr;
	ScreenGame *screenGame = nullptr;
	Screen *screens[MAX_SCREENS];
	int screenCount = 0;

	void swapScreensWithTransition(Screen *oldScene, Screen *newScene);
};
