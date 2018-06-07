#include "File.hpp"
#ifdef _MSC_VER
#include <direct.h>
#else
// TODO
#endif

#include "SDL.h"

//----------------------------------------------------------------------------//
// PathUtils
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
bool PathUtils::IsFullPath(const char* _path)
{
	if (_path && _path[0])
	{
#ifdef _WIN32
		const char* _dev = strchr(_path, ':');
		return _dev && (_dev[1] == '\\' || _dev[1] == '/');
#else
		// TODO
#endif
	}
	return false;
}
//----------------------------------------------------------------------------//
bool PathUtils::IsDelimeter(char _ch)
{
#ifdef _WIN32
	return _ch == '\\' || _ch == '/';
#else
	// TODO
#endif
}
//----------------------------------------------------------------------------//
String PathUtils::Extension(const char* _path)
{
	if (_path)
	{
		String _ext;
		for (const char* i = _path + strlen(_path); --i >= _path; )
		{
			if (*i == '.')
			{
				while (*i++ != 0)
					_ext += *i;
				return _ext;
			}
		}
	}

	return String::Empty;
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// FileStream
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
FileStream::~FileStream(void)
{
	Close();
}
//----------------------------------------------------------------------------//
bool FileStream::Open(const String& _name, Mode _mode)
{
	static const char* _modes[] =
	{
		"rb", // ReadOnly
		"rb+", // ReadWriteExistent
		"rb+", // ReadWrite
		"wb+", // Overwrite
	};

	Close();

	m_readOnly = _mode == Mode::ReadOnly;
	m_name = _name;
	m_handle = fopen(_name, _modes[(int)_mode]);
	if (!m_handle && _mode == Mode::ReadWrite)
		m_handle = fopen(_name, "wb+");

	if (!m_handle)
	{
		LOG("Error: Unable to %s file \"%s\"", (_mode == Mode::ReadWrite || _mode == Mode::Overwrite) ? "create" : "open", _name.CStr());
		return false;
	}

	uint _pos = Tell();
	Seek(0, SeekOrigin::End);
	m_size = Tell();
	Seek(_pos, SeekOrigin::Set);

	return true;
}
//----------------------------------------------------------------------------//
void FileStream::Close(void)
{
	if (m_handle)
	{
		fclose(m_handle);
		m_handle = nullptr;
	}
}
//----------------------------------------------------------------------------//
bool FileStream::EoF(void)
{
	return m_handle && feof(m_handle);
}
//----------------------------------------------------------------------------//
void FileStream::Seek(int _offset, SeekOrigin _origin)
{
	if (m_handle)
		fseek(m_handle, _offset, (int)_origin);
}
//----------------------------------------------------------------------------//
uint FileStream::Tell(void)
{
	return m_handle ? (uint)ftell(m_handle) : 0;
}
//----------------------------------------------------------------------------//
uint FileStream::Read(void* _dst, uint _size)
{
	ASSERT(!_size || _dst);
	return m_handle ? (uint)fread(_dst, 1, _size, m_handle) : 0;
}
//----------------------------------------------------------------------------//
uint FileStream::Write(const void* _src, uint _size)
{
	ASSERT(!_size || _src);
	return (m_handle && !m_readOnly) ? (uint)fwrite(_src, 1, _size, m_handle) : 0;
}
//----------------------------------------------------------------------------//
void FileStream::Flush(void)
{
	if (m_handle && !m_readOnly)
		fflush(m_handle);
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// FileSystem
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
FileSystem::FileSystem(void)
{
	LOG("Initialize FileSystem");

	char* _base = SDL_GetBasePath();
	LOG("\"%s\"", _base);

	chdir(_base);
	AddPath(""); // root dir
}
//----------------------------------------------------------------------------//
FileSystem::~FileSystem(void)
{
}
//----------------------------------------------------------------------------//
bool FileSystem::OnEvent(int _type, void* _data)
{
	switch (_type)
	{
	case SystemEvent::PreloadEngineSettings:
		_PreloadEngineSettings(*reinterpret_cast<Json*>(_data));
		break;

	case SystemEvent::SaveEngineSettings:
		_SaveEngineSettings(*reinterpret_cast<Json*>(_data));
		break;
	}
	return false;
}
//----------------------------------------------------------------------------//
void FileSystem::AddPath(const String& _path)
{
	String _fp;

	if (!PathUtils::IsFullPath(_path.CStr()))
	{
		char _cd[4096];
		getcwd(_cd, sizeof(_cd));
		size_t _len = strlen(_cd);
		if (!PathUtils::IsDelimeter(_cd[_len - 1]))
			_cd[_len] = '/', _cd[_len + 1] = 0;

		_fp = _cd + _path;
	}
	else
	{
		_fp = _path;
	}

	if (!PathUtils::IsDelimeter(_fp.Back()))
		_fp += "/";

	uint _hash = String::IHash(_fp);
	if (m_paths.Find(_hash) == m_paths.End())
	{
		LOG("Add Path \"%s\" as \"%s\"", _path.CStr(), _fp.CStr());
		m_paths[_hash] = { _path, _fp };
	}
}
//----------------------------------------------------------------------------//
bool FileSystem::FileExists(const String& _name, String* _path)
{
	if (PathUtils::IsFullPath(_name))
	{
		FILE* _test = fopen(_name, "rb");
		if (_test)
		{
			fclose(_test);
			if (_path)
				*_path = _name;
			return true;
		}
	}
	else
	{
		for (const auto& i : m_paths)
		{
			ASSERT(PathUtils::IsFullPath(i.second.fp));
			if (FileExists(i.second.fp + _name, _path))
				return true;
		}
	}
	return false;
}
//----------------------------------------------------------------------------//
StreamPtr FileSystem::OpenFile(const String& _name, FileStream::Mode _mode)
{
	FileStreamPtr _file = new FileStream;
	String _path;

	if (FileExists(_name, &_path))
	{
		_file->Open(_path, _mode);
	}
	else if(_mode == FileStream::Mode::Overwrite || _mode == FileStream::Mode::ReadWrite)
	{
		_file->Open(_name, _mode);
	}
	else
	{
		LOG("Error: File \"%s\" not found", _name.CStr());
	}

	return _file.Cast<Stream>();
}
//----------------------------------------------------------------------------//
bool FileSystem::LoadString(const String& _path, String& _dst)
{
	auto _file = OpenFile(_path);

	if (_file->IsOpened())
	{
		_dst.Resize(_file->Size());
		_file->Read(_dst.Data(), _dst.Length());
		return true;
	}

	return false;
}
//----------------------------------------------------------------------------//
String FileSystem::LoadString(const String& _path)
{
	String _str;
	LoadString(_path, _str);
	return _str;
}
//----------------------------------------------------------------------------//
bool FileSystem::SaveString(const String& _path, const String& _src)
{
	auto _file = OpenFile(_path, FileStream::Mode::Overwrite);
	if (_file->IsOpened())
	{
		_file->Write(_src.CStr(), _src.Length());
		_file->Flush();
		return true;
	}

	return false;
}
//----------------------------------------------------------------------------//
Json FileSystem::LoadJson(const String& _path)
{
	Json _dst;
	String _err;
	if (!_dst.Parse(LoadString(_path), &_err))
	{
		LOG("%s%s", _path.CStr(), _err.CStr());
	}
	return _dst;
}
//----------------------------------------------------------------------------//
void FileSystem::SaveJson(const String& _path, const Json& _src)
{
	SaveString(_path, _src.Print());
}
//----------------------------------------------------------------------------//
void FileSystem::_PreloadEngineSettings(Json& _cfg)
{
	Json _paths = _cfg.Get("FileSystem");

	for (uint i = 0; i < _paths.Size(); ++i)
	{
		AddPath(_paths[i]);
	}
}
//----------------------------------------------------------------------------//
void FileSystem::_SaveEngineSettings(Json& _cfg)
{
	Json _paths;
	for (const auto& i : m_paths)
	{
		if(i.second.src.NonEmpty())
			_paths.Push(i.second.src);
	}
	_cfg["FileSystem"] = _paths;
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
//
//----------------------------------------------------------------------------//
