#include "Networks.h"


void ScreenBackground::enable()
{
	background = Instantiate();
	background->size = { 1920.f, 1080.f };
	background->sprite = App->modRender->addSprite(background);
	background->sprite->texture = App->modResources->background;
	background->sprite->order = -2;
}

void ScreenBackground::update()
{
	background->size = { (float)Window.width, (float)Window.height };
}

void ScreenBackground::disable()
{
	Destroy(background);
}
