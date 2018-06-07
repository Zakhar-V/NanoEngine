#pragma once

#include "Base.hpp"

//----------------------------------------------------------------------------//
// RefCounted
//----------------------------------------------------------------------------//

//!
class RefCounted : public NonCopyable
{
public:

	//!
	class WeakReference final : public NonCopyable
	{
	public:
		friend class RefCounted;

		//!
		void AddRef(void);
		//!
		void Release(void);

		RefCounted* GetPtr(void);

	private:
		//!
		WeakReference(RefCounted* _ptr) : m_ptr(_ptr) { }
		//!
		~WeakReference(void)
		{
			ASSERT(m_ptr == nullptr && m_refCount == 0, "Incorrect deletion");
		}

		//!
		void _Reset(void);

		RefCounted* m_ptr;
		int m_refCount = 1;
	};

	//!
	RefCounted(void) = default;
	//!
	~RefCounted(void)
	{
		ASSERT(m_weakRef == nullptr && m_refCount == 0, "Incorrect deletion");
	}

	//!
	void AddRef(void);
	//! Safe increment a counter of references. Uses for weak references. 
	bool SafeAddRef(void);
	//!
	void Release(void);

	//!
	WeakReference* GetWeakRef(void);

protected:
	//!
	void _ResetRef(void);
	//!
	virtual void _DeleteThis(void)
	{
		_ResetRef();
	}

private:
	//!
	WeakReference* m_weakRef = nullptr;
	int m_refCount = 0;
};

//----------------------------------------------------------------------------//
// SharedPtr
//----------------------------------------------------------------------------//

//!
enum NoAddRef_t { NoAddRef };

//!
template <class T> class SharedPtr
{
public:
	//!
	SharedPtr(void) = default;
	//!
	~SharedPtr(void)
	{
		if (m_ptr)
			m_ptr->Release();
	}
	//!
	SharedPtr(const SharedPtr& _ptr) : SharedPtr(_ptr.m_ptr) {}
	//!
	SharedPtr(SharedPtr&& _ptr) : m_ptr(_ptr.m_ptr)
	{
		_ptr.m_ptr = nullptr;
	}
	//!
	SharedPtr(const T* _ptr) : m_ptr(const_cast<T*>(_ptr))
	{
		if (m_ptr)
			m_ptr->AddRef();
	}
	//!
	SharedPtr(const T* _ptr, NoAddRef_t) : m_ptr(const_cast<T*>(_ptr)) { }

	//!
	SharedPtr& operator = (const T* _ptr)
	{
		if (_ptr)
			const_cast<T*>(_ptr)->AddRef();
		if (m_ptr)
			m_ptr->Release();
		m_ptr = const_cast<T*>(_ptr);
		return *this;
	}
	//!
	SharedPtr& operator = (const SharedPtr& _ptr)
	{
		return *this = _p.m_ptr;
	}
	//!
	SharedPtr& operator = (SharedPtr&& _ptr)
	{
		Swap(_ptr.m_ptr, m_ptr);
		return *this;
	}

	//!
	operator T* (void) const { return const_cast<T*>(m_ptr); }
	//!
	T* operator -> (void) const { return const_cast<T*>(m_ptr); }
	//!
	T& operator * (void) const { return *const_cast<T*>(p); }

	//!
	T* Get(void) const { return const_cast<T*>(m_ptr); }
	//!
	template <class X> X* Cast(void) const { return static_cast<X*>(const_cast<T*>(m_ptr)); }

protected:
	T* m_ptr = nullptr;
};

//----------------------------------------------------------------------------//
// WeakRef
//----------------------------------------------------------------------------//

//!
template <class T> class WeakRef
{
public:
	//!
	WeakRef(void) = default;
	//!
	~WeakRef(void) = default;
	//!
	WeakRef(const WeakRef& _ref) : m_ref(_ref.m_ref) { }
	//!
	WeakRef(WeakRef&& _ref) : m_ref(Move(_ref.m_ref)) { }
	//!
	WeakRef(const SharedPtr<T>& _ptr) : WeakRef(_ptr.Get()) { }
	//!
	WeakRef(const T* _ptr) : m_ref(_ptr ? const_cast<T*>(_ptr)->GetweakRef() : nullptr) { }

	//!
	WeakRef& operator = (const WeakRef& _ref)
	{
		m_ref = _ref.m_ref;
		return *this;
	}
	//!
	WeakRef& operator = (WeakRef&& _ref)
	{
		Swap(m_ref, _ref.m_ref);
		return *this;
	}
	//!
	WeakRef& operator = (const SharedPtr<T>& _ptr) { return *this = _ptr.Get(); }
	//!
	WeakRef& operator = (const T* _ptr)
	{
		m_ref = _ptr ? const_cast<T*>(_ptr)->GetweakRef() : nullptr;
		return *this;
	}

	//!
	operator SharedPtr<T>(void) const
	{
		return Lock();
	}

	//!
	T* Get(void) const
	{
		return m_ref ? m_ref->GetPtr() : nullptr;
	}

	//!
	SharedPtr<T> Lock(void) const
	{
		T* _ptr = Get();
		if (_ptr && _ptr->SafeAddRef())
			return SharedPtr<T>(_ptr, NoAddRef);

		return nullptr;
	}

protected:
	SharedPtr<RefCounted::WeakReference> m_ref;
};

//----------------------------------------------------------------------------//
// Object
//----------------------------------------------------------------------------//

#ifndef _MSC_VER
#define __SUPER(BASE) 	typedef BASE __super
#else
#define __SUPER(BASE)
#	pragma warning(disable : 4307) // overflow of integer constant
#endif

//!
#define RTTI(TYPE, BASE) \
	__SUPER(BASE); \
	enum : uint { TypeID = String::ConstHash(TYPE) }; \
	uint GetTypeID(void) override { return TypeID; } \
	bool IsTypeOf(uint _type) override { return _type == TypeID || __super::IsTypeOf(_type); } \
	template <class T> bool IsTypeOf(void) { return IsTypeOf(T::TypeID); } \
	static constexpr const char* TypeName = TYPE; \
	const char* GetTypeName(void) override { return TypeName; }

//!
class Object : public RefCounted
{
public:
	//!
	enum : uint { TypeID = String::ConstHash("Object") };
	//!
	virtual uint GetTypeID(void) { return TypeID; }
	//!
	virtual bool IsTypeOf(uint _type) { return _type == TypeID; }
	//!
	template <class T> bool IsTypeOf(void) { return IsTypeOf(T::TypeID); }
	//!
	static constexpr const char* TypeName = "Object";
	//!
	virtual const char* GetTypeName(void) { return TypeName; }

	//!
	typedef SharedPtr<Object>(*Factory)(void);

	//!
	template <class T> static SharedPtr<Object> NewInstance() { return new T; }

	//!
	struct TypeInfo
	{
		//!
		TypeInfo* SetFactory(Object::Factory _factory) { Factory = _factory; return this; }
		//!
		TypeInfo* SetFlags(uint _flags) { flags = _flags; return this; }
		//!
		TypeInfo* AddFlags(uint _flags) { flags |= _flags; return this; }
		//!
		bool HasAnyOfFlags(uint _flags) { return (flags & _flags) != 0; }


		uint type;
		const char* name;

		Object::Factory Factory = nullptr;
		uint flags = 0; //!< type-specific flags
	};

	//!
	static TypeInfo* GetOrCreateTypeInfo(const char* _name);
	//!
	template <class T> static TypeInfo* GetOrCreateTypeInfo(void) { return GetOrCreateTypeInfo(T::TypeName); }

	//!
	static TypeInfo* GetTypeInfo(uint _type);
	//!
	template <class T> static TypeInfo* GetTypeInfo(void) { return GetTypeInfo(T::TypeID); }

	//!
	template <class T> static void Register(uint _flags = 0) { GetOrCreateTypeInfo<T>()->SetFactory(&Object::NewInstance<T>)->SetFlags(_flags); }

	//!
	static SharedPtr<Object> Create(uint _type, const char* _name);
	//!
	template <class T> static SharedPtr<T> Create(void) { return Create(T::TypeID, T::TypeName).Cast<T>(); }



	//!
	virtual bool SendEvent(int _type, void* _data = nullptr) { return false; }
	//!
	//virtual void BroadcastEvent(int _type, void* _data = nullptr, Object* _sender = nullptr) { }
	//!
	virtual bool OnEvent(int _type, void* _data) { return false; }


protected: // private?

	static HashMap<uint, TypeInfo> s_types;
};

//!
typedef SharedPtr<Object> ObjectPtr;

//----------------------------------------------------------------------------//
// Module
//----------------------------------------------------------------------------//

//!
template <class T> class Module : public Object, public Singleton<T>
{

};

//----------------------------------------------------------------------------//
// Context
//----------------------------------------------------------------------------//

#define gContext Context::Instance

//!
class Context : public Module<Context>
{
public:
	RTTI("Context", Object);

	//!
	Context(void);
	//!
	~Context(void);

	//!
	virtual bool Startup(void);
	//!
	virtual void Shutdown(void);

	//!
	void AddSystem(Object* _system);

	//!
	bool SendEvent(int _type, void* _data = nullptr) override;
	//!
	virtual bool OnEvent(int _type, void* _data);


protected:

	List<ObjectPtr> m_systems;
};

//----------------------------------------------------------------------------//
// 	SystemEvent
//----------------------------------------------------------------------------//

struct SystemEvent
{
	enum Type
	{
		Startup = String::ConstHash("SystemEvent::Startup"),
		Shutdown = String::ConstHash("SystemEvent::Shutdown"),

		PreloadEngineSettings = String::ConstHash("SystemEvent::PreloadEngineSettings"),
		SaveEngineSettings = String::ConstHash("SystemEvent::SaveEngineSettings"),

		LoadUserSettings = String::ConstHash("SystemEvent::LoadUserSettings"),
		SaveUserSettings = String::ConstHash("SystemEvent::SaveUserSettings"),

		BeginFrame = String::ConstHash("SystemEvent::BeginFrame"),
		SDL = String::ConstHash("SystemEvent::SDL"), //!< internal sdl event
		EndFrame = String::ConstHash("SystemEvent::EndFrame"),
	};
};

//----------------------------------------------------------------------------//
// 
//----------------------------------------------------------------------------//
