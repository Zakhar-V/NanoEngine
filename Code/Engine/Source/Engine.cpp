#include "../Engine.hpp"

//----------------------------------------------------------------------------//
// Engine
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
Engine::Engine(void)
{ 
	AddSystem(new FileSystem);
	AddSystem(new Time);
	AddSystem(new Device);
	AddSystem(new Graphics);
}
//----------------------------------------------------------------------------//
Engine::~Engine(void)
{
}
//----------------------------------------------------------------------------//
bool Engine::Startup(void)
{
	LOG("Preload engine settings");
	{
		Json _settings = gFileSystem->LoadJson("EngineSettings.json");
		SendEvent(SystemEvent::PreloadEngineSettings, &_settings);
	}

	LOG("Startup...");

	if (!Context::Startup())
		return false;

	LOG("Load user settings");
	{
		Json _settings = gFileSystem->LoadJson("UserSettings.json");
		SendEvent(SystemEvent::LoadUserSettings, &_settings);
	}

	return true;
}
//----------------------------------------------------------------------------//
void Engine::Shutdown(void)
{
	LOG("Flush settings");
	{
		Json _settings;
		SendEvent(SystemEvent::SaveEngineSettings, &_settings);
		gFileSystem->SaveJson("EngineSettings.json", _settings);
	}
	{
		Json _settings;
		SendEvent(SystemEvent::SaveUserSettings, &_settings);
		gFileSystem->SaveJson("UserSettings.json", _settings);
	}

	LOG("Shutdown...");
	Context::Shutdown();
}
//----------------------------------------------------------------------------//
void Engine::OneFrame(void)
{
	SendEvent(SystemEvent::BeginFrame);

	// ...

	SendEvent(SystemEvent::EndFrame);
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//
