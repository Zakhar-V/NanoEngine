#include "Graphics.hpp"
#include "Device.hpp"
#include "SDL.h"

#include <windows.h>
#include "GL/gl.h"

//----------------------------------------------------------------------------//
// Graphics
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
Graphics::Graphics(void)
{
}
//----------------------------------------------------------------------------//
Graphics::~Graphics(void)
{
}
//----------------------------------------------------------------------------//
bool Graphics::OnEvent(int _type, void* _data)
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
	}
	return false;
}
//----------------------------------------------------------------------------//
void Graphics::_PreloadEngineSettings(Json& _cfg)
{
	m_settings = _cfg.Get("Graphics");
}
//----------------------------------------------------------------------------//
void Graphics::_SaveEngineSettings(Json& _cfg)
{
	//
	m_settings["CoreProfile"] = true;
	m_settings["ForwardCompatible"] = false;
	m_settings["DebugContext"] = false;

	_cfg["Graphics"] = m_settings;
}
//----------------------------------------------------------------------------//
void Graphics::_LoadUserSettings(Json& _cfg)
{
	const Json& _src = _cfg.Get("Graphics");
	m_vsync = _src["VSync"];
}
//----------------------------------------------------------------------------//
void Graphics::_SaveUserSettings(Json& _cfg)
{
	Json& _dst = _cfg["Graphics"];
	_dst["VSync"] = m_vsync;
}
//----------------------------------------------------------------------------//
bool Graphics::_Startup(void)
{
	LOG("Graphics::Startup");

	int _ctx_flags = SDL_GL_CONTEXT_FORWARD_COMPATIBLE_FLAG | SDL_GL_CONTEXT_DEBUG_FLAG;
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_FLAGS, _ctx_flags);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);

	m_context = SDL_GL_CreateContext(gDevice->WindowHandle());

	if (!m_context)
	{
		LOG("Unable to create of OpenGL context: %s", SDL_GetError());
		return false;
	}

	LOG("%s on %s, %s", glGetString(GL_VERSION), glGetString(GL_RENDERER), glGetString(GL_VENDOR));	// temp


	return true;
}
//----------------------------------------------------------------------------//
void Graphics::_Shutdown(void)
{
	LOG("Graphics::Shutdown");

	if (m_context)
	{
		SDL_GL_DeleteContext(m_context);
		m_context = nullptr;
	}
}
//----------------------------------------------------------------------------//
void Graphics::_BeginFrame(void)
{
	//_ResetState
}
//----------------------------------------------------------------------------//
void Graphics::_EndFrame(void)
{
	SDL_GL_SetSwapInterval(m_vsync ? 1 : 0);
	SDL_GL_SwapWindow(gDevice->WindowHandle());
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//

