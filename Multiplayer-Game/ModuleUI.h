#pragma once

class ModuleUI : public Module
{
public:

	// Virtual functions

	bool init() override;

	bool preUpdate() override;

	bool gui() override;

	bool postUpdate() override;

	bool cleanUp() override;


	// Public methods

	void setInputsEnabled(bool enabled);

	LRESULT HandleWindowsEvents(UINT msg, WPARAM wParam, LPARAM lParam);

	bool inputsEnabled = true;
};
