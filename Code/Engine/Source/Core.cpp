#include "Core.hpp"

//----------------------------------------------------------------------------//
// RefCounted::WeakReference
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
void RefCounted::WeakReference::AddRef(void)
{
#ifndef NO_THREADS
	AtomicAdd(m_refCount, 1);
#else
	++m_refCount;
#endif
}
//----------------------------------------------------------------------------//
void RefCounted::WeakReference::Release(void)
{
#ifndef NO_THREADS
	if (AtomicSubtract(m_refCount, 1) == 1)
		delete this;
#else
	if (!--m_refCount)
		delete this;
#endif
}
//----------------------------------------------------------------------------//
RefCounted* RefCounted::WeakReference::GetPtr(void)
{
#ifndef NO_THREADS
	return AtomicGet(m_ptr);
#else
	return m_ptr;
#endif
}
//----------------------------------------------------------------------------//
void RefCounted::WeakReference::_Reset(void)
{
#ifndef NO_THREADS
	AtomicSet(m_ptr, nullptr);
#else
	m_ptr = nullptr;
#endif
	Release();
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// RefCounted
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
void RefCounted::AddRef(void)
{
#ifndef NO_THREADS
	AtomicAdd(m_refCount, 1);
#else
	++m_refCount;
#endif
}
//----------------------------------------------------------------------------//
bool RefCounted::SafeAddRef(void)
{
#ifndef NO_THREADS
	for (;;)
	{
		int _rc = AtomicGet(m_refCount, MemoryOrder::Relaxed);

		if (!_rc)
			break;

		if (AtomicCompareExchange(m_refCount, _rc, _rc + 1))
			return true;
	}
	return false;
#else
	AddRef();
	return true;
#endif
}
//----------------------------------------------------------------------------//
void RefCounted::Release(void)
{
#ifndef NO_THREADS
	if (AtomicSubtract(m_refCount, 1) == 1)
		_DeleteThis();
#else
	if (!--m_refCount)
		_DeleteThis();
#endif
}
//----------------------------------------------------------------------------//
RefCounted::WeakReference* RefCounted::GetWeakRef(void)
{
#ifndef NO_THREADS
	for (WeakReference* c = nullptr; (c = AtomicGet(m_weakRef, MemoryOrder::Relaxed)) == nullptr || c == (WeakReference*)-1;)
	{
		if (AtomicCompareExchange<WeakReference>(m_weakRef, (WeakReference*)0, (WeakReference*)-1))
		{
			AtomicSet(m_weakRef, new WeakReference(this));
			break;
		}
	}
#else
	if (!m_weakRef)
		m_weakRef = new WeakReference(this);
#endif
	return m_weakRef;
}
//----------------------------------------------------------------------------//
void RefCounted::_ResetRef(void)
{
	if (m_weakRef)
	{
		m_weakRef->_Reset();
		m_weakRef = nullptr;
	}
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// Object
//----------------------------------------------------------------------------//

HashMap<uint, Object::TypeInfo> Object::s_types;

//----------------------------------------------------------------------------//
Object::TypeInfo* Object::GetOrCreateTypeInfo(const char* _name)
{
	uint _type = String::Hash(_name);
	auto _iter = s_types.Find(_type);
	if (_iter != s_types.End())
		return &_iter->second;

	LOG("Register %s(0x%08x) typeinfo", _name, _type);

	auto& _typeInfo = s_types[_type];
	_typeInfo.type = _type;
	_typeInfo.name = _name;

	return &_typeInfo;
}
//----------------------------------------------------------------------------//
Object::TypeInfo* Object::GetTypeInfo(uint _type)
{
	auto _iter = s_types.Find(_type);
	if (_iter != s_types.End())
		return &_iter->second;
	return nullptr;
}
//----------------------------------------------------------------------------//
SharedPtr<Object> Object::Create(uint _type, const char* _name)
{
	auto _iter = s_types.Find(_type);
	if (_iter == s_types.End())
	{
		LOG("Error: Factory for '%s'(0x%08x) not found", _name, _type); // or better use of FATAL?
		return nullptr;
	}

	return _iter->second.Factory();
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// Context
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
Context::Context(void)
{

}
//----------------------------------------------------------------------------//
Context::~Context(void)
{

}
//----------------------------------------------------------------------------//
bool Context::Startup(void)
{
	bool _fail = false;
	SendEvent(SystemEvent::Startup, &_fail);

	if (_fail)
	{
		LOG("Unable to start up of engine");
		return false;
	}
	return true;
}
//----------------------------------------------------------------------------//
void Context::Shutdown(void)
{
	for (auto i = m_systems.RBegin(); i != m_systems.End(); --i)
	{
		(*i)->OnEvent(SystemEvent::Shutdown, nullptr);
	}
}
//----------------------------------------------------------------------------//
void Context::AddSystem(Object* _system)
{
	TODO("");

	m_systems.Push(_system);
}
//----------------------------------------------------------------------------//
bool Context::SendEvent(int _type, void* _data)
{
	for (auto i : m_systems)
	{
		if(i->OnEvent(_type, _data))
			return true;
	}
	return false;
}
//----------------------------------------------------------------------------//
bool Context::OnEvent(int _type, void* _data)
{
	return false;
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//
