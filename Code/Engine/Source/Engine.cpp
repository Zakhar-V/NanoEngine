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
	LOG("Startup...");

	if (!Context::Startup())
		return false;

	return true;
}
//----------------------------------------------------------------------------//
void Engine::Shutdown(void)
{
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
