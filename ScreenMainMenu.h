#pragma once

class ScreenMainMenu : public Screen
{
	void gui() override;
public:
	bool showInvalidUserName = false;
};
