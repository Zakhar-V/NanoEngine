#include "Base.hpp"
#include <Windows.h> // remove it

//----------------------------------------------------------------------------//
// Assert
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
void Assert(const char* _type, const char* _func, const char* _file, int _line, const char* _desc, const char* _msg, ...)
{
	LogF("%s(%d):", _file, _line);
	LogF("%s: %s", _type, _desc);

	char _info[512];
	memset(_info, 0, sizeof(_info));
	if (_msg)
	{
		va_list _args;
		va_start(_args, _msg);

		LogV(_msg, _args);
		_vsnprintf(_info, sizeof(_info), _msg, _args);

		va_end(_args);
	}

	char _text[1024];
	_snprintf(_text, sizeof(_text), "%s\n%s\nat <%s>\n%s(%d)\n", _desc, _info, _func, _file, _line);

	if (IsDebuggerPresent())
	{
		DebugBreak();
	}
	else
	{
		int _action = MessageBoxA(nullptr, _text, _type, MB_ABORTRETRYIGNORE | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND | MB_SYSTEMMODAL);
		if (_action == IDABORT)
		{
			LogF("terminate process");
			ExitProcess(-1);
		}
	}
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// Log
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
void LogV(const char* _msg, va_list _args)
{
	if (_msg)
	{
		vprintf(_msg, _args);
		printf("\n");
	}
}
//----------------------------------------------------------------------------//
void LogF(const char* _msg, ...)
{
	va_list _args;
	va_start(_args, _msg);
	LogV(_msg, _args);
	va_end(_args);
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// Linked list 
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
void** _LL_Field(void* _ptr, size_t _offset)
{
	return (void**)((uint8*)_ptr + _offset);
}
//----------------------------------------------------------------------------//
void _LL_Link(void* _head, void* _this, void* _node)
{
	/*
	next = head;
	if(next) next->prev = this;
	head = this;
	*/

	size_t _offset = (uint8*)_node - (uint8*)_this;
	void** _next = 1 + (void**)_node;
	*_next = *(void**)_head;
	if (*_next)
		_LL_Field(*_next, _offset)[0] = _this;
	*(void**)_head = _this;
}
//----------------------------------------------------------------------------//
void _LL_Unlink(void* _head, void* _this, void* _node)
{
	/*
	if (m_next) m_next->m_prev = m_prev;
	if (m_prev) m_prev->m_next = m_next;
	else if(head == this) head = m_next;
	*/

	size_t _offset = (uint8*)_node - (uint8*)_this;
	void** _prev = 0 + (void**)_node;
	void** _next = 1 + (void**)_node;

	if (*_next)
		_LL_Field(*_next, _offset)[0] = *_prev;
	if (*_prev)
		_LL_Field(*_prev, _offset)[1] = *_next;
	else if (*(void**)_head == _this)
		*(void**)_head = *_next;

	*_prev = nullptr;
	*_next = nullptr;
}
//----------------------------------------------------------------------------//
void _LL_LinkFirst(void* _first, void* _last, void* _this, void* _node)
{
	/*
	next = first;
	if(next) next->prev = this;
	first = this;
	if(!last) last = this;

	*/

	size_t _offset = (uint8*)_node - (uint8*)_this;
	void** _prev = 0 + (void**)_node;
	void** _next = 1 + (void**)_node;

	*_next = *(void**)_first;
	if (*_next)
		_LL_Field(*_next, _offset)[0] = _this;
	*(void**)_first = _this;
	if (!*(void**)_last)
		*(void**)_last = _this;
}
//----------------------------------------------------------------------------//
void _LL_LinkLast(void* _first, void* _last, void* _this, void* _node)
{
	/*
	prev = last;
	if(prev) prev->next = this;
	last = this;
	if(!first) first = this;
	*/

	size_t _offset = (uint8*)_node - (uint8*)_this;
	void** _prev = 0 + (void**)_node;
	void** _next = 1 + (void**)_node;

	*_prev = *(void**)_last;
	if (*_prev)
		_LL_Field(*_prev, _offset)[1] = _this;
	*(void**)_last = _this;
	if (!*(void**)_first)
		*(void**)_first = _this;
}
//----------------------------------------------------------------------------//
void _LL_Unlink(void* _first, void* _last, void* _this, void* _node)
{
	/*
	if(m_next) m_next->m_prev = m_prev;
	else last = m_prev;
	if (m_prev) m_prev->m_next = m_next;
	else first = m_next;
	*/

	size_t _offset = (uint8*)_node - (uint8*)_this;
	void** _prev = 0 + (void**)_node;
	void** _next = 1 + (void**)_node;

	if (*_next)
		_LL_Field(*_next, _offset)[0] = *_prev;
	else
		*(void**)_last = *_prev;

	if (*_prev)
		_LL_Field(*_prev, _offset)[1] = *_next;
	else
		*(void**)_first = *_next;

	*_prev = nullptr;
	*_next = nullptr;
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// String
//----------------------------------------------------------------------------//

const String String::Empty;

//----------------------------------------------------------------------------//
String::String(const char* _str1, int _length1, const char* _str2, int _length2)
{
	_length1 = Length(_str1, _length1);
	_length2 = Length(_str2, _length2);
	Reserve(_length1 + _length2);
	Append(_str1, _length1);
	Append(_str2, _length2);
}
//----------------------------------------------------------------------------//
String& String::operator = (const String& _rhs)
{
	if (_rhs.m_data != m_data)
	{
		Reserve(_rhs.m_length);
		memmove(m_data, _rhs.m_data, _rhs.m_length);
		m_data[_rhs.m_length] = 0;
		m_length = _rhs.m_length;
	}
	return *this;
}
//----------------------------------------------------------------------------//
String& String::operator = (String&& _rhs)
{
	Swap(m_length, _rhs.m_length);
	Swap(m_capacity, _rhs.m_capacity);
	Swap(m_data, _rhs.m_data);
	return *this;
}
//----------------------------------------------------------------------------//
String& String::operator = (const char* _str)
{
	uint _length = Length(_str);
	Reserve(_length);
	memmove(m_data, _str, _length);
	m_data[_length] = 0;
	m_length = _length;
	return *this;
}
//----------------------------------------------------------------------------//
String& String::Reserve(uint _maxLength)
{
	if (m_capacity < _maxLength)
	{
		_maxLength = GrowTo(m_capacity, _maxLength) | 15;
		char* _newData = Allocate<char>(_maxLength + 1);
		memcpy(_newData, m_data, m_length);
		_newData[m_length] = 0;
		Deallocate(m_data);
		m_data = _newData;
		m_capacity = _maxLength;
	}
	return *this;
}
//----------------------------------------------------------------------------//
String& String::Resize(uint _newLength, char _ch)
{
	if (m_length < _newLength)
	{
		Append(_newLength - m_length, _ch);
	}
	else if (m_length > _newLength)
	{
		m_data[_newLength] = 0;
		m_length = _newLength;
	}
	return *this;
}
//----------------------------------------------------------------------------//
String& String::Append(const char* _str, int _length)
{
	_length = Length(_str, _length);
	if (_length)
	{
		uint _newLength = m_length + _length;
		Reserve(_newLength);
		memmove(m_data + m_length, _str, _length);
		m_length = _newLength;
		m_data[m_length] = 0;
	}
	return *this;
}
//----------------------------------------------------------------------------//
String& String::Append(char _ch)
{
	uint _newLength = m_length + 1;
	Reserve(_newLength);
	m_data[m_length] = _ch;
	m_data[_newLength] = 0;
	m_length = _newLength;
	return *this;
}
//----------------------------------------------------------------------------//
String& String::Append(uint _count, char _ch)
{
	uint _newLength = m_length + _count;
	Reserve(_newLength);
	memset(m_data + m_length, _ch, _count);
	m_length = _newLength;
	m_data[m_length] = 0;
	return *this;
}
//----------------------------------------------------------------------------//
String String::SubStr(uint _offset, int _length) const
{
	if (_offset > m_length)
		_offset = m_length;
	if (_length < 0 || _offset + _length > m_length)
		_length = m_length - _offset;
	return String(m_data + _offset, _length);
}
//----------------------------------------------------------------------------//
uint String::Hash(const char* _str, uint _hash)
{
	if (_str) while (*_str)
		_hash = *_str++ + (_hash << 6) + (_hash << 16) - _hash;
	return _hash;
}
//----------------------------------------------------------------------------//
uint String::IHash(const char* _str, uint _hash)
{
	if (_str) while (*_str)
		_hash = Lower(*_str++) + (_hash << 6) + (_hash << 16) - _hash;
	return _hash;
}
//----------------------------------------------------------------------------//
char* String::Lower(char* _str, int _length)
{
	ASSERT(_length < (int)Length(_str, _length));
	for (const char* _end = _str + Length(_str, _length); _str < _end; ++_str)
		*_str = Lower(*_str);
	return _str;
}
//----------------------------------------------------------------------------//
char* String::Upper(char* _str, int _length)
{
	ASSERT(_length < (int)Length(_str, _length));
	for (const char* _end = _str + Length(_str, _length); _str < _end; ++_str)
		*_str = Upper(*_str);
	return _str;
}
//----------------------------------------------------------------------------//
int String::Compare(const char* _lhs, const char* _rhs, bool _ignoreCase)
{
	_lhs = _lhs ? _lhs : "";
	_rhs = _rhs ? _rhs : "";
	if (_ignoreCase)
	{
		while (*_lhs && Lower(*_lhs++) == Upper(*_rhs++));
		return *_lhs - *_rhs;
	}
	return strcmp(_lhs, _rhs);
}
//----------------------------------------------------------------------------//
String String::Format(const char* _fmt, ...)
{
	va_list _args;
	va_start(_args, _fmt);
	String _str = FormatV(_fmt, _args);
	va_end(_args);
	return _str;
}
//----------------------------------------------------------------------------//
String String::FormatV(const char* _fmt, va_list _args)
{
#if 0
	char _buff[4096];
	vsnprintf_s(_buff, sizeof(_buff), _fmt, _args);
	return _buff;
#else
	//this code from ptypes: pputf.cxx
	struct
	{
		bool operator () (char _ch) const
		{
			return strchr(" #+-~.0123456789", _ch) != 0;
		}
	} _check_fmtopts;
	enum fmt_type_t { FMT_NONE, FMT_CHAR, FMT_SHORT, FMT_INT, FMT_LONG, FMT_LARGE, FMT_STR, FMT_PTR, FMT_DOUBLE, FMT_LONG_DOUBLE };

	String _res;
	fmt_type_t fmt_type;
	const char *e, *p = _fmt;
	char buf[4096], fbuf[128];
	while (p && *p != 0)
	{
		// write out raw data between format specifiers
		e = strchr(p, '%');
		if (e == 0)
			e = p + strlen(p);
		if (e > p)
			_res.Append(p, e);
		if (*e != '%')
			break;
		if (*++e == '%') // write out a single '%' 
		{
			_res += '%';
			p = e + 1;
			continue;
		}


		// build a temporary buffer for the conversion specification
		fbuf[0] = '%';
		char* f = fbuf + 1;
		bool modif = false;

		// formatting flags and width specifiers
		while (_check_fmtopts(*e) && uint(f - fbuf) < sizeof(fbuf) - 5)
			*f++ = *e++, modif = true;

		// prefixes
		switch (*e)
		{
		case 'h':
			fmt_type = FMT_SHORT;
			*f++ = *e++;
			break;

		case 'L':
			fmt_type = FMT_LONG_DOUBLE;
			*f++ = *e++;
			break;

		case 'l':
		{
			if (*++e == 'l')
			{
#                   if defined(_MSC_VER) || defined(__BORLANDC__)
				*f++ = 'I'; *f++ = '6'; *f++ = '4';
#                   else
				*f++ = 'l'; *f++ = 'l';
#                   endif
				++e;
				fmt_type = FMT_LARGE;
			}
			else
			{
				*f++ = 'l';
				fmt_type = FMT_LONG;
				// x64 ?
			}
		} break;

		default:
			fmt_type = FMT_NONE;
			break;
		};

		// format specifier
		switch (*e)
		{
		case 'c':
			fmt_type = FMT_CHAR;
			*f++ = *e++;
			break;

		case 'd':
		case 'i':
		case 'o':
		case 'u':
		case 'x':
		case 'X':
			if (fmt_type < FMT_SHORT || fmt_type > FMT_LARGE)
				fmt_type = FMT_INT;
			*f++ = *e++;
			break;

		case 'e':
		case 'E':
		case 'f':
		case 'g':
		case 'G':
			if (fmt_type != FMT_LONG_DOUBLE)
				fmt_type = FMT_DOUBLE;
			*f++ = *e++;
			break;

		case 's':
			fmt_type = FMT_STR;
			*f++ = *e++;
			break;

		case 'p':
			fmt_type = FMT_PTR;
			*f++ = *e++;
			break;
		};
		if (fmt_type == FMT_NONE)
			break;
		*f = 0;

		// some formatters are processed here 'manually', while others are passed to snprintf
		int s = 0;
		switch (fmt_type)
		{
		case FMT_NONE: // to avoid compiler warning 
			break;
		case FMT_CHAR:
			if (modif)
				s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int));
			else
				_res += (char)(va_arg(_args, int));
			break;

		case FMT_SHORT:
			s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int));
			break;

		case FMT_INT:
			s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int));
			break;

		case FMT_LONG:
			s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, long));
			break;

		case FMT_LARGE:
			s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, int64));
			break;

		case FMT_STR:
		{
			if (modif)
				s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, const char*));
			else
			{
				const char* _str = va_arg(_args, const char*);
				if (_str)
					_res += _str;
				//else _res += "<null>";
			}
		}break;

		case FMT_PTR:
			s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, void*));
			break;
		case FMT_DOUBLE:

			s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, double));
			for (; s > 0; --s)
			{
				if (buf[s - 1] == '0')
					buf[s - 1] = 0;
				else if (buf[s - 1] == '.')
				{
					buf[--s] = 0;
					break;
				}
				else
					break;
			}
			break;

		case FMT_LONG_DOUBLE:
			s = _snprintf(buf, sizeof(buf), fbuf, va_arg(_args, long double));
			break;
		}

		if (s > 0)
			_res.Append(buf, s);

		p = e;
	}
	return _res;
#endif
}
//----------------------------------------------------------------------------//
bool String::Match(const char* _str, const char* _pattern, bool _ignoreCase)
{
	// '*' - any symbols sequence
	// ' '(space) - any number of spaces
	// '['charset']' - set of symbols
	// '['lower'-'upper']' - range of symbols
	// "file.ext" matched "*.ext"

	if (!_str || !_str[0])
		return false;
	if (!_pattern || !_pattern[0])
		return false;

	Array<Pair<const char*, const char*>> _stack; // <string, pattern>
	_stack.Push({ _str, _pattern });
	for (const char *_s = _str, *_p = _pattern;;)
	{
	$_frame:
		if (*_p == 0 && *_s == 0)
			goto $_true;
		if (*_s == 0 && *_p != '*')
			goto $_false;
		if (*_p == '*') // �����������
		{
			++_p;
			if (*_p == 0) goto $_true;
			for (;; )
			{
				_stack.Back().first = _s;
				_stack.Back().second = _p;
				_stack.Push({ _s, _p });
				goto $_frame;
			$_xmatch_true:
				goto $_true;
			$_xmatch_false:
				if (*_s == 0)
					goto $_false;
				++_s;
			}
		}
		if (*_p == '?') // ����� ������ 
			goto $_match;
		if (*_p == ' ') // ����� ���������� ��������
		{
			if (*_s == ' ')
			{
				for (; _p[1] == ' '; ++_p);
				for (; _s[1] == ' '; ++_s);
				goto $_match;
			}
			goto $_false;
		}
		if (*_p == '[') // ����� ��������
		{
			for (++_p; ; )
			{
				if (*_p == ']' || *_p == 0)
					goto $_false;
				if (*_p == '\\') // ����������� ������
				{
					if (_p[1] == '[' || _p[1] == ']' || _p[1] == '\\')
						++_p;
				}
				if (_ignoreCase ? (Lower(*_p) == Lower(*_s)) : (*_p == *_s))
					break;

				if (_p[1] == '-') // ��������
				{
					if (!_p[2])
						goto $_false;
					if (_ignoreCase)
					{
						char l = Lower(*_s);
						char u = Upper(*_s);
						if (_p[0] <= l && _p[2] >= l)
							break;
						if (_p[0] >= l && _p[2] <= l)
							break;
						if (_p[0] <= u && _p[2] >= u)
							break;
						if (_p[0] >= u && _p[2] <= u)
							break;
					}
					else
					{
						if (_p[0] <= *_s && _p[2] >= *_s)
							break;
						if (_p[0] >= *_s && _p[2] <= *_s)
							break;
					}
					_p += 2;
				}
				++_p;
			}
			while (*_p != ']')
			{
				if (*_p == '\\' && (_p[1] == '[' || _p[1] == ']' || _p[1] == '\\')) // ����������� ������
					++_p;
				if (*_p == 0)
				{
					--_p;
					break;
				}
				++_p;
			}
			goto $_match;
		}
		if (*_p == '\\' && (_p[1] == '[' || _p[1] == ']' || _p[1] == '\\')) // ����������� ������
			++_p;
		if (_ignoreCase ? (Lower(*_p) != Lower(*_s)) : (*_p != *_s))
			goto $_false;

	$_match:
		++_p;
		++_s;
		continue;
	$_true:
		if (_stack.Size() > 1)
		{
			_stack.Pop();
			_s = _stack.Back().first;
			_p = _stack.Back().second;
			goto $_xmatch_true;
		}
		return true;
	$_false:
		if (_stack.Size() > 1)
		{
			_stack.Pop();
			_s = _stack.Back().first;
			_p = _stack.Back().second;
			goto $_xmatch_false;
		}
		break;
	}
	return false;
}
//----------------------------------------------------------------------------//
const char* String::Find(const char* _str1, const char* _str2, bool _ignoreCase)
{
	_str1 = _str1 ? _str1 : "";
	_str2 = _str2 ? _str2 : "";
	if (_ignoreCase)
	{
		for (; *_str1; *_str1++)
		{
			for (const char *a = _str1, *b = _str2; Lower(*a++) == Lower(*b++);)
			{
				if (!*b)
					return _str1;
			}
		}
		return nullptr;
	}
	return strstr(_str1, _str2);
}
//----------------------------------------------------------------------------//
char* String::Find(char* _str1, const char* _str2, bool _ignoreCase)
{
	if (!_str1)
		return nullptr;

	_str2 = _str2 ? _str2 : "";
	if (_ignoreCase)
	{
		for (; *_str1; *_str1++)
		{
			for (const char *a = _str1, *b = _str2; Lower(*a++) == Lower(*b++);)
			{
				if (!*b)
					return _str1;
			}
		}
		return nullptr;
	}
	return strstr(_str1, _str2);
}
//----------------------------------------------------------------------------//
void String::Split(const char* _str, const char* _delimiters, Array<String>& _dst)
{
	if (_str)
	{
		const char* _end = _str;
		if (_delimiters)
		{
			while (*_str)
			{
				while (*_str && strchr(_delimiters, *_str))
					++_str;
				_end = _str;
				while (*_end && !strchr(_delimiters, *_end))
					++_end;
				if (_str != _end)
					_dst.Push(String(_str, _end));
				_str = _end;
			}
		}
		else
			_end = _str + strlen(_str);
		if (_str != _end)
			_dst.Push(String(_str, _end));
	}
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// Tokenizer
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
void Tokenizer::Advance(int _num)
{
	while (_num-- && *s)
		++s;
}
//----------------------------------------------------------------------------//
int Tokenizer::SkipWhiteSpace(void)
{
	const char* _start = s;
	while (AnyOf(" \t\n\r"))
		Advance();
	return s - _start;
}
//----------------------------------------------------------------------------//
int Tokenizer::SkipComments(void)
{
	const char* _start = s;
	if (s[0] == '/' && (s[1] == '/' || s[1] == '*')) // comment
	{
		if (s[1] == '/')
		{
			Advance(2);
			while (!EoF() && !AnyOf("\n\r"))
				Advance();
		}
		else
		{
			Advance(2);
			while (!EoF())
			{
				if (Cmp("*/", 2))
				{
					Advance(2);
					break;
				}
				if (EoF())
					e = "EoF in multiline comment";
				Advance();
			}
		}
	}
	return s - _start;
}
//----------------------------------------------------------------------------//
void Tokenizer::NextToken(void)
{
	while ((SkipWhiteSpace() || SkipComments()) && !e);
}
//----------------------------------------------------------------------------//
bool Tokenizer::IsNumber(void) const
{
	return AnyOf("0123456789+-.");
}
//----------------------------------------------------------------------------//
bool Tokenizer::ParseNumber(Number& _val)
{
	if (!IsNumber())
		return RaiseError("Expected numeric constant not found");

	const char* _start = s;
	if (s[0] == '-')
		Advance();

	int _readed = 0, _result = 0;
	while (AnyOf("0123456789"))
		Advance(), _readed++;

	if (s[0] == '.')
	{
		_val.isFloat = true;

		Advance();
		_readed = 0;
		while (AnyOf("0123456789"))
			Advance(), _readed++;

		if (!_readed)
			return RaiseError("Wrong numeric constant");

		if (AnyOf("eE"))
		{
			Advance();
			if (!AnyOf("+-"))
				return RaiseError("Wrong numeric constant");
			Advance();

			_readed = 0;
			while (AnyOf("0123456789"))
				Advance(), _readed++;

			if (!_readed)
				return RaiseError("Wrong numeric constant");

			_result = sscanf(_start, "%e", &_val.fValue);
		}
		else
		{
			_result = sscanf(_start, "%f", &_val.fValue);
		}
	}
	else if (!_readed)
	{
		return RaiseError("Wrong numeric constant");
	}
	else
	{
		_val.isFloat = false;
		_result = sscanf(_start, "%d", &_val.iValue);
	}

	if (!_result)
		return RaiseError("Wrong numeric constant");

	return true;
}
//----------------------------------------------------------------------------//
bool Tokenizer::IsString(void) const
{
	return *s == '"';
}
//----------------------------------------------------------------------------//
bool Tokenizer::ParseString(String& _val)
{
	if (!IsString())
		return RaiseError("Expected string constant not found");

	Advance();
	for (;;)
	{
		if (s[0] == '\\')
		{
			switch (s[1])
			{
			case '\\':
				Advance(2);
				_val += "\\";
				break;
			case '/':
				Advance(2);
				_val += "/";
				break;
			case 'b':
				Advance(2);
				_val += "\b";
				break;
			case 'f':
				Advance(2);
				_val += "\f";
				break;
			case 'n':
				Advance(2);
				_val += "\n";
				break;
			case 'r':
				Advance(2);
				_val += "\r";
				break;
			case 't':
				Advance(2);
				_val += "\t";
				break;
			case 'u':
			{
				Advance(2);
				char _buff[5];
				for (uint i = 0; i < 4; ++i)
				{
					if (!AnyOf("0123456789abcdefABCDEF"))
						return RaiseError("Expected numeric literal");
					_buff[i] = *s;
					Advance();
				}
				_buff[4] = 0;
				uint16 _char = 0;
				sscanf(_buff, "%hx", &_char);

				if (_char > 0xff)
					return RaiseError("Unicode character not supported"); // TODO:

				_val += (char)_char;

			} break;

			default:
				return RaiseError("Unknown escape sequence");
			}
		}
		else if (*s == '"')
		{
			Advance();
			break;
		}
		else if (AnyOf("\n\r"))
		{
			return RaiseError("New line in string constant");
		}
		else
		{
			_val.Append(s, 1);
			Advance();
		}
	}

	return true;
}
//----------------------------------------------------------------------------//
bool Tokenizer::RaiseError(const char* _error)
{
	e = _error;
	return false;
}
//----------------------------------------------------------------------------//
void Tokenizer::GetErrorPos(const char* _start, const char* _pos, int& _line, int& _column)
{
	// Windows \r\n
	// Linux \n
	// Macintosh(Mac OSX) \n
	// Macintosh(old) \r

	_line = 1;
	_column = 1;
	while (_start < _pos)
	{
		if (_start[0] == '\r')
		{
			if (_start[1] == '\n')
				++_start;
			++_line;
			_column = 0;
		}
		else if (_start[0] == '\n')
		{
			++_line;
			_column = 0;
		}
		++_start;
		++_column;
	}
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// Json
//----------------------------------------------------------------------------//

const Json Json::Null;
const Json Json::EmptyArray(Json::Type::Array);
const Json Json::EmptyObject(Json::Type::Object);

//----------------------------------------------------------------------------//
Json::Json(const Json& _other)
{
	SetType(_other.m_type);
	if (IsString())
		_String() = _other._String();
	else if (IsNode())
		_Node() = _other._Node();
	else
		m_int = _other.m_int;
}
//----------------------------------------------------------------------------//
Json::Json(Json&& _temp)
{
	SetType(_temp.m_type);
	if (IsString())
		_String() = std::move(_temp._String());
	else if (IsNode())
		_Node() = std::move(_temp._Node());
	else
		m_int = _temp.m_int;
}
//----------------------------------------------------------------------------//
Json::Json(bool _value)
{
	SetType(Type::Bool);
	_Bool() = _value;
}
//----------------------------------------------------------------------------//
Json::Json(int _value)
{
	SetType(Type::Int);
	_Int() = _value;
}
//----------------------------------------------------------------------------//
Json::Json(uint _value)
{
	SetType(Type::Int);
	_Int() = _value;
}
//----------------------------------------------------------------------------//
Json::Json(float _value)
{
	SetType(Type::Float);
	_Float() = _value;
}
//----------------------------------------------------------------------------//
Json::Json(const char* _value)
{
	SetType(Type::String);
	_String() = _value;
}
//----------------------------------------------------------------------------//
Json::Json(const String& _value)
{
	SetType(Type::String);
	_String() = _value;
}
//----------------------------------------------------------------------------//
Json::Json(String&& _value)
{
	SetType(Type::String);
	_String() = std::move(_value);
}
//----------------------------------------------------------------------------//
Json& Json::operator = (const Json& _rhs)
{
	SetType(_rhs.m_type);
	if (IsString())
		_String() = _rhs._String();
	else if (IsNode())
		_Node() = _rhs._Node();
	else
		m_int = _rhs.m_int;

	return *this;
}
//----------------------------------------------------------------------------//
Json& Json::operator = (Json&& _rhs)
{
	SetType(_rhs.m_type);
	if (IsString())
		_String() = std::move(_rhs._String());
	else if (IsNode())
		_Node() = std::move(_rhs._Node());
	else
		m_int = _rhs.m_int;
	return *this;
}
//----------------------------------------------------------------------------//
Json& Json::SetType(Type _type)
{
	if (m_type != _type)
	{
		switch (m_type)
		{
		case Type::String:
			_String().~String();
			break;
		case Type::Array:
		case Type::Object:
			_Node().~Node();
			break;
		};

		m_type = _type;

		switch (m_type)
		{
		case Type::String:
			new(&_String()) String();
			break;
		case Type::Array:
		case Type::Object:
			new(&_Node()) Node();
			break;
		default:
			m_int = 0;
			break;
		};
	}
	/*else if (IsNode() && (_type == Type::Array || _type == Type::Object))
	{
		m_type = _type;
	}*/

	return *this;
}
//----------------------------------------------------------------------------//
Json& Json::SetArray(InitializerList<Json> _value)
{
	Clear();
	for (auto i : _value)
		Push(i);
	return *this;
}
//----------------------------------------------------------------------------//
Json& Json::SetObject(InitializerList<KeyValue> _value)
{
	Clear();
	for (auto i : _value)
		Set(i.first, i.second);
	return *this;
}
//----------------------------------------------------------------------------//
bool Json::AsBool(void) const
{
	switch (m_type)
	{
	case Type::Null:
	case Type::Bool:
	case Type::Int:
		return m_int != 0;
	case Type::Float:
		return m_flt != 0;
	}
	return false;
}
//----------------------------------------------------------------------------//
int Json::AsInt(void) const
{
	switch (m_type)
	{
	case Type::Null:
	case Type::Bool:
	case Type::Int:
		return m_int;
	case Type::Float:
		return (int)m_flt;
	}
	return 0;
}
//----------------------------------------------------------------------------//
float Json::AsFloat(void) const
{
	switch (m_type)
	{
	case Type::Null:
	case Type::Bool:
	case Type::Int:
		return (float)m_int;
	case Type::Float:
		return m_flt;
	}
	return 0;
}
//----------------------------------------------------------------------------//
String Json::AsString(void) const
{
	switch (m_type)
	{
	case Type::Null:
		return "null";
	case Type::Bool:
		return m_bool ? "true" : "false";
	case Type::Int:
		return String::Format("%d", m_int);
	case Type::Float:
		return String::Format("%f", m_flt);
	case Type::String:
		return _String();
	}
	return "";
}
//----------------------------------------------------------------------------//
uint Json::Size(void) const
{
	switch (m_type)
	{
	case Type::Array:
	case Type::Object:
		return (uint)_Node().Size();
	}
	return 0;
}
//----------------------------------------------------------------------------//
Json& Json::Clear(void)
{
	if (IsNode())
		_Node().Clear();
	return *this;
}
//----------------------------------------------------------------------------//
Json& Json::Resize(uint _size)
{
	SetType(Type::Array);
	_Node().Resize(_size);
	return *this;
}
//----------------------------------------------------------------------------//
Json& Json::Get(uint _index)
{
	SetType(Type::Array);
	return _Node()[_index].second;
}
//----------------------------------------------------------------------------//
const Json& Json::Get(uint _index) const
{
	return IsArray() ? _Node()[_index].second : Null;
}
//----------------------------------------------------------------------------//
Json& Json::Insert(uint _pos, const Json& _value)
{
	SetType(Type::Array);
	_Node().Emplace(_pos, "", _value);
	return *this;
}
//----------------------------------------------------------------------------//
Json& Json::Insert(uint _pos, Json&& _value)
{
	SetType(Type::Array);
	_Node().Emplace(_pos, "", Move(_value));
	return *this;
}
//----------------------------------------------------------------------------//
Json& Json::Push(const Json& _value)
{
	SetType(Type::Array);
	_Node().Push({ "", _value });
	return *this;
}
//----------------------------------------------------------------------------//
Json& Json::Push(Json&& _value)
{
	SetType(Type::Array);
	_Node().Push({ "", Move(_value) });
	return *this;
}
//----------------------------------------------------------------------------//
Json& Json::Append(void)
{
	SetType(Type::Array);
	_Node().Push({ "", Null });
	return _Node().Back().second;
}
//----------------------------------------------------------------------------//
Json& Json::Pop(void)
{
	if (IsArray())
		_Node().Pop();
	return *this;
}
//----------------------------------------------------------------------------//
Json& Json::Erase(uint _start, uint _num)
{
	if (IsArray())
		_Node().Erase(_start, _num);
	return *this;
}
//----------------------------------------------------------------------------//
Json& Json::GetOrAdd(const String& _key)
{
	SetType(Type::Object);

	Json* _value = Find(_key);
	if (_value)
		return *_value;

	_Node().Push({ _key, Null });
	return _Node().Back().second;
}
//----------------------------------------------------------------------------//
const Json& Json::Get(const String& _key) const
{
	const Json* _value = Find(_key);
	return _value ? *_value : Null;
}
//----------------------------------------------------------------------------//
Json* Json::Find(const String& _key)
{
	if (IsObject())
	{
		for (auto& i : _Node())
		{
			if (i.first == _key)
				return &i.second;
		}
	}
	return nullptr;
}
//----------------------------------------------------------------------------//
const Json* Json::Find(const String& _key) const
{
	if (IsObject())
	{
		for (auto& i : _Node())
		{
			if (i.first == _key)
				return &i.second;
		}
	}
	return nullptr;
}
//----------------------------------------------------------------------------//
Json& Json::Set(const String& _key, const Json& _value)
{
	GetOrAdd(_key) = _value;
	return *this;
}
//----------------------------------------------------------------------------//
bool Json::Erase(const String& _key)
{
	if (IsObject())
	{
		for (auto i = _Node().Begin(); i != _Node().End(); ++i)
		{
			if (i->first == _key)
			{
				_Node().Erase(i);
				return true;
			}
		}
	}
	return false;
}
//----------------------------------------------------------------------------//
Json::Node& Json::Container(void)
{
	SetType(Type::Object);
	return _Node();
}
//----------------------------------------------------------------------------//
const Json::Node& Json::Container(void) const
{
	return IsObject() ? _Node() : EmptyObject._Node();
}
//----------------------------------------------------------------------------//
bool Json::Parse(const char* _str, String* _error)
{
	if (!_str)
		return true;

	Tokenizer _stream;
	_stream.s = _str;

	if (!_Parse(_stream))
	{
		if (_error)
		{
			int _l, _c;
			Tokenizer::GetErrorPos(_str, _stream.s, _l, _c);
			*_error = String::Format("(%d:%d) : JSON error : %s", _l, _c, _stream.e);
		}

		return false;
	}

	return true;
}
//----------------------------------------------------------------------------//
String Json::Print(void) const
{
	String _str;
	_Print(_str, 0);
	return _str;
}
//----------------------------------------------------------------------------//
/*bool Json::Load(Stream* _src)
{
	ASSERT(_src != nullptr);

	Array<char> _data;
	_data.resize(_src->Size());
	_src->Read(_data.data(), (uint)_data.size());

	String _err;
	if (!Parse(_data.data(), &_err))
	{
		LOG("%s%s", _src->Name().c_str(), _err.c_str());
		return false;
	}
	return true;
}
//----------------------------------------------------------------------------//
void Json::Save(Stream* _dst)
{
	ASSERT(_dst != nullptr);

	String _str = Print();
	_dst->Write(_str.c_str(), (uint)_str.length());
}*/
//----------------------------------------------------------------------------//
bool Json::_Parse(Tokenizer& _str)
{
	//http://www.json.org/json-ru.html

	_str.NextToken();

	if (_str.e)
		return false;

	if (_str.EoF()) // eof
	{
		SetNull();
	}
	else if (_str.IsNumber()) // int or float
	{
		Tokenizer::Number _val;
		if (!_str.ParseNumber(_val))
			return false;

		if (_val.isFloat)
			SetFloat(_val.fValue);
		else
			SetInt(_val.iValue);
	}
	else if (_str.IsString()) // string
	{
		SetType(Type::String);
		if (!_str.ParseString(_String()))
			return false;
	}
	else if (_str.Cmpi("true", 4)) // bool
	{
		_str += 4;
		SetBool(true);
	}
	else if (_str.Cmpi("false", 5))	// bool
	{
		_str += 5;
		SetBool(false);
	}
	else if (_str.Cmpi("null", 4)) // null
	{
		_str += 4;
		SetNull();
	}
	else if (_str[0] == '[') // array
	{
		++_str;
		SetType(Type::Array);
		for (;;)
		{
			if (_str[0] == ']')
			{
				++_str;
				break;
			}

			if (_str.EoF())
				return _str.RaiseError("Unexpectd EoF");

			if (!Append()._Parse(_str))
				return false;

			_str.NextToken();
			if (_str[0] == ',') // divisor (not necessarily) 
				++_str;
		}
	}
	else if (_str[0] == '{') // object
	{
		++_str;
		SetType(Type::Object);
		for (;;)
		{
			_str.NextToken();

			if (_str[0] == '}')
			{
				++_str;
				break;
			}

			if (_str.EoF())
				return _str.RaiseError("Unexpectd EoF");

			_Node().Push({ "", Null });
			KeyValue& _pair = _Node().Back();

			if (!_str.ParseString(_pair.first))
				return false;

			_str.NextToken();
			if (_str[0] == ':')
				++_str;
			else
				return _str.RaiseError("Expected ':' not found");

			_str.NextToken();
			if (!_pair.second._Parse(_str))
				return false;

			_str.NextToken();
			if (_str[0] == ',') // divisor (not necessarily) 
				++_str;
		}
	}
	else
	{
		return _str.RaiseError("Unknown symbol");
	}

	return true;
}
//----------------------------------------------------------------------------//
void Json::_Print(String& _dst, int _depth) const
{
	switch (m_type)
	{
	case Type::Null:
		_dst += "null";
		break;
	case Type::Bool:
		_dst += _Bool() ? "true" : "false";
		break;
	case Type::Int:
		_dst += String::Format("%d", _Int());
		break;
	case Type::Float:
		_dst += String::Format("%f", _Float());
		break;
	case Type::String:
	{
		_PrintString(_dst, _String(), _depth);

	} break;
	case Type::Array:
	{
		bool _oneLine = true;
		if (_Node().Size() < 5)
		{
			for (const auto& i : _Node())
			{
				if (i.second.IsNode())
				{
					_oneLine = false;
					break;
				}
			}
		}
		else
			_oneLine = false;

		_dst += "[";

		for (const auto& i : _Node())
		{
			if (!_oneLine)
			{
				_dst += '\n';
				for (int i = 0; i <= _depth; ++i)
					_dst += "\t";
			}
			i.second._Print(_dst, _depth + 1);

			if (&i != &_Node().Back())
			{
				_dst += ",";
				if (_oneLine)
					_dst += " ";
			}
		}

		if (!_oneLine)
		{
			_dst += "\n";
			for (int i = 0; i < _depth; ++i)
				_dst += "\t";
		}
		_dst += "]";

	} break;
	case Type::Object:
	{
		_dst += "{\n";

		for (const auto& i : _Node())
		{
			for (int i = 0; i <= _depth; ++i)
				_dst += "\t";

			_PrintString(_dst, i.first, _depth + 1);

			_dst += " : ";
			i.second._Print(_dst, _depth + 1);

			if (&i != &_Node().Back())
				_dst += ",\n";
		}

		_dst += "\n";
		for (int i = 0; i < _depth; ++i)
			_dst += "\t";
		_dst += "}";

	} break;
	}
}
//----------------------------------------------------------------------------//
void Json::_PrintString(String& _dst, const String& _src, int _depth)
{
	_dst += "\"";
	for (char s : _src)
	{
		if (s == '\n')
			_dst += "\\n";
		else if (s == '\r')
			_dst += "\\r";
		else if (s == '\\')
			_dst += "\\\\";
		else
			_dst += s; // TODO:
	}
	_dst += "\"";
}
//----------------------------------------------------------------------------//

//----------------------------------------------------------------------------//
// 
//----------------------------------------------------------------------------//
