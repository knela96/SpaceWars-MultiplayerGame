#include "Networks.h"
#include "Application.h"

#define ADD_MODULE(ModuleClass, module) \
	module = new ModuleClass(); \
	modules[numModules++] = module;

#define ADD_MODULE_DISABLED(ModuleClass, module) \
	module = new ModuleClass(); \
	module->setEnabled(false); \
	modules[numModules++] = module;

Application::Application()
{
	ADD_MODULE          (ModulePlatform,         modPlatform);
	ADD_MODULE          (ModuleRender,           modRender);
	ADD_MODULE          (ModuleSound,            modSound);
	ADD_MODULE_DISABLED (ModuleNetworkingServer, modNetServer);
	ADD_MODULE_DISABLED (ModuleNetworkingClient, modNetClient);
	ADD_MODULE          (ModuleLinkingContext,   modLinkingContext);
	ADD_MODULE          (ModuleTaskManager,      modTaskManager);
	ADD_MODULE          (ModuleTextures,         modTextures);
	ADD_MODULE          (ModuleResources,        modResources);
	ADD_MODULE          (ModuleGameObject,       modGameObject);
	ADD_MODULE          (ModuleCollision,        modCollision);
	ADD_MODULE          (ModuleBehaviour,        modBehaviour);
	ADD_MODULE          (ModuleScreen,           modScreen);
	ADD_MODULE          (ModuleUI,               modUI);
}


Application::~Application()
{
	for (int i = 0; i < numModules; ++i)
	{
		Module* module = modules[i];

		delete module;
	}
}


bool Application::init()
{
	for (int i = 0; i < numModules; ++i)
	{
		Module* module = modules[i];

		if (module->init() == false)
		{
			return false;
		}
	}

	if (doStart() == false) return false;

	return true;
}

bool Application::update()
{
	BEGIN_TIMED_BLOCK(Frame);

	if (doStart() == false) return false;

	if (doPreUpdate() == false) return false;
	
	if (doUpdate() == false) return false;

	if (doGui() == false) return false;

	if (doPostUpdate() == false) return false;

	if (doStop() == false) return false;

	modRender->present();

	END_TIMED_BLOCK(Frame);

	END_PROFILING_FRAME();

	return (exitFlag == false);
}

bool Application::cleanUp()
{
	for (int i = numModules - 1; i >= 0; --i)
	{
		modules[i]->setEnabled(false);
	}

	if (doStop() == false) return false;

	for (int i = numModules - 1; i >= 0; --i)
	{
		Module* module = modules[i];

		if (module->cleanUp() == false)
		{
			return false;
		}
	}

	return true;
}

bool Application::doStart()
{
	for (int i = 0; i < numModules; ++i)
	{
		Module* module = modules[i];

		if (module->needsStart())
		{
			module->updateEnabledState();

			if (module->start() == false)
			{
				return false;
			}
		}
	}

	return true;
}

bool Application::doPreUpdate()
{
	BEGIN_TIMED_BLOCK(PreUpdate);

	for (int i = 0; i < numModules; ++i)
	{
		Module* module = modules[i];

		if (module->isEnabled())
		{
			if (module->preUpdate() == false) return false;
		}
	}

	END_TIMED_BLOCK(PreUpdate);

	return true;
}

bool Application::doUpdate()
{
	BEGIN_TIMED_BLOCK(Update);

	static float accumulator = 0.0f;

	accumulator += Time.frameTime;

	unsigned int count = 0;

	//while (accumulator >= Time.deltaTime)
	{
		accumulator -= Time.deltaTime;
		count++;

		for (int i = 0; i < numModules; ++i)
		{
			Module* module = modules[i];

			if (module->isEnabled())
			{
				if (module->update() == false) return false;
			}
		}
	}

	// TO DO: Interpolation of world values.
	// This is not an easy one... as we advance the simulation in
	// fixed time steps, the simulated results (positions, etc) are
	// a small amount of time ahead from the current time. To solve
	// this the ideal solution would be to interpolate all the values
	// between the previous state and the current state using this
	// deviation in time.

	END_TIMED_BLOCK(Update);

	return true;
}

bool Application::doGui()
{
	BEGIN_TIMED_BLOCK(GuiUpdate);

	if (modUI->isEnabled())
	{
		for (int i = 0; i < numModules; ++i)
		{
			Module* module = modules[i];

			if (module->isEnabled())
			{
				if (module->gui() == false) return false;
			}
		}
	}

	END_TIMED_BLOCK(GuiUpdate);

	return true;
}

bool Application::doPostUpdate()
{
	BEGIN_TIMED_BLOCK(PostUpdate);

	for (int i = 0; i < numModules; ++i)
	{
		Module* module = modules[i];

		if (module->isEnabled())
		{
			if (module->postUpdate() == false) return false;
		}
	}

	END_TIMED_BLOCK(PostUpdate);

	return true;
}

bool Application::doStop()
{
	for (int i = numModules - 1; i >= 0; --i)
	{
		Module* module = modules[i];

		if (module->needsStop())
		{
			module->updateEnabledState();

			if (module->stop() == false)
			{
				return false;
			}
		}
	}

	return true;
}
