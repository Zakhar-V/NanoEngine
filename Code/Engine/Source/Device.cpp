#include "Device.hpp"
#include "SDL.h"

//----------------------------------------------------------------------------//
// Device
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
Device::Device(void)
{
}
//----------------------------------------------------------------------------//
Device::~Device(void)
{
}
//----------------------------------------------------------------------------//
void Device::RequireExit(bool _exit)
{
	m_opened = !_exit;
}
//----------------------------------------------------------------------------//
void Device::ShowWindow(bool _show)
{
	if (_show)
		SDL_ShowWindow(m_window);
	else
		SDL_HideWindow(m_window);
}
//----------------------------------------------------------------------------//
bool Device::OnEvent(int _type, void* _data)
{
	switch (_type)
	{
	case SystemEvent::PreloadEngineSettings:
		_PreloadEngineSettings(*reinterpret_cast<Json*>(_data));
		break;

	case SystemEvent::SaveEngineSettings:
		_SaveEngineSettings(*reinterpret_cast<Json*>(_data));
		break;

	case SystemEvent::LoadUserSettings:
		_LoadUserSettings(*reinterpret_cast<Json*>(_data));
		break;

	case SystemEvent::SaveUserSettings:
		_SaveUserSettings(*reinterpret_cast<Json*>(_data));
		break;

	case SystemEvent::Startup:
		if (!_Startup())
		{
			*reinterpret_cast<bool*>(_data) = false;
			return true;
		}
		break;

	case SystemEvent::Shutdown:
		_Shutdown();
		break;

	case SystemEvent::BeginFrame:
		_BeginFrame();
		break;

	case SystemEvent::EndFrame:
		_EndFrame();
		break;

	case SystemEvent::SDL:
		_OnSDLEvent(reinterpret_cast<SDL_Event*>(_data));
		break;
	}
	return false;
}
//----------------------------------------------------------------------------//
void Device::_PreloadEngineSettings(Json& _cfg)
{
	m_settings = _cfg.Get("Device");
}
//----------------------------------------------------------------------------//
void Device::_SaveEngineSettings(Json& _cfg)
{
	//
	_cfg["Device"] = m_settings;
}
//----------------------------------------------------------------------------//
void Device::_LoadUserSettings(Json& _cfg)
{
	const Json& _src = _cfg.Get("Device");
	m_fullscreen = _src["Fullscreen"];
}
//----------------------------------------------------------------------------//
void Device::_SaveUserSettings(Json& _cfg)
{
	Json& _dst = _cfg["Device"];
	_dst["Fullscreen"] = m_fullscreen;
}
//----------------------------------------------------------------------------//
bool Device::_Startup(void)
{
	LOG("Device::Startup");

	SDL_Init(SDL_INIT_EVERYTHING);

	SDL_DisplayMode _dm;
	SDL_GetDesktopDisplayMode(0, &_dm);
	LOG("Desktop Display Mode: %dx%d @ %d Hz", _dm.w, _dm.h, _dm.refresh_rate);

	m_window = SDL_CreateWindow("Demo", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, _dm.w / 3 * 2, _dm.h / 3 * 2, SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL);

	if (!m_window)
	{
		LOG("Unable to create of window: %s", SDL_GetError());
		return false;
	}

	SDL_GetWindowSize(m_window, &m_size.x, &m_size.y);
	m_opened = true;

	return true;
}
//----------------------------------------------------------------------------//
void Device::_Shutdown(void)
{
	LOG("Device::Shutdown");

	if (m_window)
		SDL_DestroyWindow(m_window);
	m_window = nullptr;
	m_opened = false;
}
//----------------------------------------------------------------------------//
void Device::_BeginFrame(void)
{
	SDL_Event _event;
	while (SDL_PollEvent(&_event))
	{
		gContext->SendEvent(SystemEvent::SDL, &_event);
	}
}
//----------------------------------------------------------------------------//
void Device::_EndFrame(void)
{
}
//----------------------------------------------------------------------------//
void Device::_OnSDLEvent(SDL_Event* _event)
{
	switch (_event->type)
	{
	case SDL_WINDOWEVENT:
	{
		switch (_event->window.event)
		{
		case SDL_WINDOWEVENT_RESIZED:
		{
			LOG("SDL_WINDOWEVENT_RESIZED %d %d", _event->window.data1, _event->window.data2);
			m_size.x = _event->window.data1;
			m_size.y = _event->window.data2;

			IntVector2 _size = m_size;
			gContext->SendEvent(DeviceEvent::Resized, &_size);

		} break;

		case SDL_WINDOWEVENT_CLOSE:
			LOG("SDL_WINDOWEVENT_CLOSE");
			m_userRequireExit = true;
			gContext->SendEvent(DeviceEvent::Close);
			break;
		}

	} break;
	}
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// 
//----------------------------------------------------------------------------//
