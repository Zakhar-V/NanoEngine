#pragma once

#include "Core.hpp"

//----------------------------------------------------------------------------//
// 
//----------------------------------------------------------------------------//

//...

//----------------------------------------------------------------------------//
// Graphics
//----------------------------------------------------------------------------//

#define gGraphics Graphics::Instance

//!
class Graphics : public Module<Graphics>
{
public:
	RTTI("Graphics", Object);

	//!
	Graphics(void);
	//!
	~Graphics(void);

	//!
	bool OnEvent(int _type, void* _data) override;

protected:
	//!
	bool _Startup(void);
	//!
	void _Shutdown(void);
	//!
	void _BeginFrame(void);
	//!
	void _EndFrame(void);

	void* m_context = nullptr;
	bool m_vsync = true;
};

//----------------------------------------------------------------------------//
// 
//----------------------------------------------------------------------------//
