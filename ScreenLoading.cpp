#include "Networks.h"


void ScreenLoading::enable()
{
	for (int i = 0; i < BAR_COUNT; ++i)
	{
		float progressRatio = (float)i / (float)BAR_COUNT;
		float radians = 2.0f * PI * progressRatio;

		loadingBars[i] = Instantiate();
		loadingBars[i]->position = 30.0f * vec2{ sinf(radians), cosf(radians) };
		loadingBars[i]->angle = -360.0f * progressRatio;
		loadingBars[i]->size = vec2{ 4, 30 };
		loadingBars[i]->sprite = App->modRender->addSprite(loadingBars[i]);
		loadingBars[i]->sprite->color = vec4{ 1.0f, 1.0f, 1.0f, 1.0f };
		loadingBars[i]->sprite->pivot = vec2{ 0.5f, 0.5f };
	}
}

void ScreenLoading::update()
{
	const float ROUND_TIME = 3.0f;
	for (int i = 0; i < BAR_COUNT; ++i)
	{
		float progressRatio = (float)i / (float)BAR_COUNT;
		auto gameObject = loadingBars[i];
		gameObject->sprite->color.a = 1.0f - fractionalPart(((float)Time.time + progressRatio * ROUND_TIME)/ ROUND_TIME);
	}

	if (App->modResources->finishedLoading)
	{
		App->modScreen->swapScreensWithTransition(this, App->modScreen->screenMainMenu);

		// NOTE(jesus): The following is equivalent to the previous line but without transition.
		//this->enabled = false;
		//App->modScene->scenePingPong->enabled = true;
	}
}

void ScreenLoading::disable()
{
	for (int i = 0; i < BAR_COUNT; ++i)
	{
		Destroy(loadingBars[i]);
	}
}
