#pragma once

class ModulePlatform : public Module
{
public:

	// Virtual functions

	bool init() override;

	bool preUpdate() override;

	bool postUpdate() override;

	bool cleanUp() override;
};
