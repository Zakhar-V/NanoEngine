#pragma once 

#include "Core.hpp"
#include "Math.hpp"

struct SDL_Window;
union SDL_Event;

//----------------------------------------------------------------------------//
// Device
//----------------------------------------------------------------------------//

struct DeviceEvent
{
	enum Type
	{
		Resized = String::ConstHash("DeviceEvent::Resized"), //!< arg: IntVector2* size;
		Close = String::ConstHash("DeviceEvent::Close"), 
	};
};

//----------------------------------------------------------------------------//
// Device
//----------------------------------------------------------------------------//

#define gDevice Device::Instance

//!
class Device : public Module<Device>
{
public:
	RTTI("Device", Object);

	//!
	Device(void);
	//!
	~Device(void);

	//!
	bool OnEvent(int _type, void* _data) override;

	//!
	const IntVector2& WindowSize(void) { return m_size; }
	//!
	SDL_Window* WindowHandle(void) { return m_window; }

	//!
	bool IsOpened(void) { return m_opened; }
	//!
	bool UserRequireExit(void) { return m_userRequireExit; }
	//!
	void RequireExit(bool _exit = true);

	//!
	void ShowWindow(bool _show = true);

protected:
	//!
	bool _Startup(void);
	//!
	void _Shutdown(void);
	//!
	void _BeginFrame(void);
	//!
	void _EndFrame(void);
	//!
	void _OnSDLEvent(SDL_Event* _event);

	SDL_Window* m_window = nullptr;
	IntVector2 m_size = { 0, 0 };

	bool m_opened = false;
	bool m_userRequireExit = false;
};

//----------------------------------------------------------------------------//
//	
//----------------------------------------------------------------------------//

