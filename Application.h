#pragma once

class Application
{
public:

	// Constructor and destructor

	Application();

	~Application();


	// Application methods

	void exit() { exitFlag = true; }


	// Application lifetime methods

	bool init();

	bool update();

	bool cleanUp();


private:

	// Private lifetime methods

	bool doStart();

	bool doPreUpdate();

	bool doUpdate();

	bool doGui();

	bool doPostUpdate();

	bool doStop();


public:

	// Modules
	ModulePlatform *modPlatform = nullptr;
	ModuleTaskManager *modTaskManager = nullptr;
	//ModuleNetworking *modNet = nullptr;
	ModuleNetworkingServer *modNetServer = nullptr;
	ModuleNetworkingClient *modNetClient = nullptr;
	ModuleLinkingContext *modLinkingContext = nullptr;
	ModuleTextures *modTextures = nullptr;
	ModuleResources *modResources = nullptr;
	ModuleGameObject *modGameObject = nullptr;
	ModuleCollision *modCollision = nullptr;
	ModuleBehaviour *modBehaviour = nullptr;
	ModuleSound *modSound = nullptr;
	ModuleScreen *modScreen = nullptr;
	ModuleUI *modUI = nullptr;
	ModuleRender *modRender = nullptr;


private:

	// All modules
	static const int MAX_MODULES = 16;
	Module* modules[MAX_MODULES] = {};
	int numModules = 0;

	// Exit flag
	bool exitFlag = false;
};

extern Application* App;
