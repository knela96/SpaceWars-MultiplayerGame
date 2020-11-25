#pragma once

class Module
{
private:

	bool enabled;
	bool nextEnabled;


public:

	// Constructor and destructor

	Module(bool startEnabled = true) : enabled(false), nextEnabled(startEnabled) { }

	virtual ~Module() { }


	// Virtual functions

	virtual bool init() { return true; }

	virtual bool start() { return true; }

	virtual bool preUpdate() { return true; }

	virtual bool update() { return true; }

	virtual bool gui() { return true; }

	virtual bool postUpdate() { return true; }

	virtual bool stop() { return true; }

	virtual bool cleanUp() { return true;  }


	// Enable disable modules

	void setEnabled(bool pEnabled)
	{
		nextEnabled = pEnabled;
	}

	bool isEnabled() const
	{
		return enabled;
	}

	bool needsStart() const
	{
		return !enabled && nextEnabled;
	}

	bool needsStop() const
	{
		return enabled && !nextEnabled;
	}

	void updateEnabledState()
	{
		enabled = nextEnabled;
	}


	// For tasks

	virtual void onTaskFinished(Task *) { }
};
