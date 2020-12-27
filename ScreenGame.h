#pragma once

class ScreenGame : public Screen
{
public:

	bool isServer = true;
	int serverPort;
	const char *serverAddress = "127.0.0.1";
	const char *playerName = "player";
	uint8 spaceshipType = 0;

	float timeSpawn = 0;
	GameObject* asteroids [MAX_GAME_OBJECTS];

private:

	void enable() override;

	void update() override;

	void GameLogic();

	void gui() override;

	void disable() override;
};

