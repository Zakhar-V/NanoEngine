#pragma once

#include "Include/Base.hpp"
#include "Include/Core.hpp"
#include "Include/Math.hpp"

//----------------------------------------------------------------------------//
// Engine
//----------------------------------------------------------------------------//

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

protected:
};

//----------------------------------------------------------------------------//
// 
//----------------------------------------------------------------------------//
