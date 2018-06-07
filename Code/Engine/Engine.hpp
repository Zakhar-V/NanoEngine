#pragma once

#include "Include/Base.hpp"
#include "Include/Core.hpp"
#include "Include/Math.hpp"

#include "Include/File.hpp"

#include "Include/Time.hpp"
#include "Include/Device.hpp"
#include "Include/Graphics.hpp"

//----------------------------------------------------------------------------//
// Engine
//----------------------------------------------------------------------------//

#define gEngine Engine::Get<Engine>()

//!
class Engine : public Context
{
public:
	RTTI("Engine", Context);

	//!
	Engine(void);
	//!
	~Engine(void);

	//!
	bool Startup(void) override;
	//!
	void Shutdown(void) override;


	//!
	void OneFrame(void);

protected:
};

//----------------------------------------------------------------------------//
// 
//----------------------------------------------------------------------------//
