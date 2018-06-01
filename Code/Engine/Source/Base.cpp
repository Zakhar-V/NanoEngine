#include "Base.hpp"

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
		if (*_p == '*') // подстановка
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
		if (*_p == '?') // любой символ 
			goto $_match;
		if (*_p == ' ') // любое количество пробелов
		{
			if (*_s == ' ')
			{
				for (; _p[1] == ' '; ++_p);
				for (; _s[1] == ' '; ++_s);
				goto $_match;
			}
			goto $_false;
		}
		if (*_p == '[') // набор символов
		{
			for (++_p; ; )
			{
				if (*_p == ']' || *_p == 0)
					goto $_false;
				if (*_p == '\\') // управл€ющий символ
				{
					if (_p[1] == '[' || _p[1] == ']' || _p[1] == '\\')
						++_p;
				}
				if (_ignoreCase ? (Lower(*_p) == Lower(*_s)) : (*_p == *_s))
					break;

				if (_p[1] == '-') // диапазон
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
				if (*_p == '\\' && (_p[1] == '[' || _p[1] == ']' || _p[1] == '\\')) // управл€ющий символ
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
		if (*_p == '\\' && (_p[1] == '[' || _p[1] == ']' || _p[1] == '\\')) // управл€ющий символ
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
// 
//----------------------------------------------------------------------------//
