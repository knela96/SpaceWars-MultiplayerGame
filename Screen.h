#pragma once

class Screen
{
public:

	// Enable / disable screens

	bool enabled = false;
	bool shouldUpdate = true;


private:

	// Virtual methods

	virtual void enable() { }  // Called each time the screen is enabled

	virtual void update() { }  // Called at each frame (if enabled)

	virtual void gui() { }     // Called at each frame (if enabled)

	virtual void disable() { } // Called each time the screen is disabled

	bool wasEnabled = false;
	friend class ModuleScreen;
};
