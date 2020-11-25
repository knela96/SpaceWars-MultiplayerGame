#include "Networks.h"


bool ModuleScreen::init()
{
	screenCount = 0;
	screens[screenCount++] = screenLoading  = new ScreenLoading;
	screens[screenCount++] = screenBackground = new ScreenBackground;
	screens[screenCount++] = screenMainMenu = new ScreenMainMenu;
	screens[screenCount++] = screenGame = new ScreenGame;
	screens[screenCount++] = screenOverlay  = new ScreenOverlay;

	screenLoading->enabled = true;
	screenBackground->enabled = true;

	return true;
}

bool ModuleScreen::update()
{
	for (int i = 0; i < screenCount; ++i)
	{
		auto screen = screens[i];

		if (!screen->enabled && screen->wasEnabled)
		{
			screen->disable();
			screen->wasEnabled = screen->enabled;
			//App->modGameObject->deleteGameObjectsInScene(screen);
		}
	}

	for (int i = 0; i < screenCount; ++i)
	{
		auto screen = screens[i];

		if (screen->enabled && !screen->wasEnabled)
		{
			screen->enable();
			screen->wasEnabled = screen->enabled;
		}
	}

	for (int i = 0; i < screenCount; ++i)
	{
		auto screen = screens[i];

		bool screenIsFullyEnabled = screen->enabled && screen->wasEnabled;

		if (screenIsFullyEnabled && screen->shouldUpdate)
		{
			screen->update();
		}
	}

	return true;
}

bool ModuleScreen::gui()
{
	for (int i = 0; i < screenCount; ++i)
	{
		auto scene = screens[i];

		if (scene->enabled)
		{
			scene->gui();
		}
	}

	return true;
}

bool ModuleScreen::cleanUp()
{
	for (int i = 0; i < screenCount; ++i)
	{
		auto scene = screens[i];
		delete scene;
	}
	return true;
}

void ModuleScreen::swapScreensWithTransition(Screen *oldScene, Screen *newScene)
{
	ASSERT(oldScene != nullptr && newScene != nullptr);
	ASSERT(screenOverlay->enabled == false);

	screenOverlay->oldScene = oldScene;
	screenOverlay->newScene = newScene;
	screenOverlay->enabled = true;
}
