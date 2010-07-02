#ifndef MY_STR_H
#define MY_STR_H

#include <cstddef>
#include <string>
#include <algorithm>

namespace my { namespace str {


/* Поиск конца буфера - определяется по завершающему нулю */

template<class Char>
inline const Char* end(const Char *buf)
{
	while (*buf) buf++;
	return buf;
}

template<class Char>
inline Char* end(Char *buf)
{
	while (*buf) buf++;
	return buf;
}

/* Поиск конца буфера. Если задан размер - простым вычеслением,
	если размер не задан (-1) - поиск завершающего нуля.
	Эта возможность только для функций чтения! */

template<class Char>
inline const Char* end(const Char *buf, std::size_t buf_sz)
{
	return buf_sz == (std::size_t)-1 ? my::str::end(buf) : buf + buf_sz;
}

template<class Char>
inline std::size_t length(const Char *str)
	{ return end(str) - str; }

/* Вывод символа - функция сделана для удобства, чтобы лишний
	раз не проверять выход за пределы буфера */
template<class Char>
inline std::size_t put(Char *buf, std::size_t buf_sz, Char c)
{
	if (buf_sz < 2)
	{
		if (buf_sz == 1)
			*buf = 0;
		
		return 0;
	}

	*buf++ = c;
	*buf = 0;

	return 1;
}

/* Вывод строки (до завершающего символа) */
template<class Char>
inline std::size_t put(Char *buf, std::size_t buf_sz, const Char *str)
{
	Char *ptr = buf;

	if (buf_sz)
	{
		Char *end = buf + buf_sz - 1;

		while (*str && ptr != end)
			*ptr++ = *str++;
	
		*ptr = 0;
	}
	
	return ptr - buf;
}

/* Вывод бинарной строки заданного размера */
template<class Char>
std::size_t put(Char *buf, std::size_t buf_sz,
	const Char *str, std::size_t str_sz)
{
	Char *ptr = buf;

	if (buf_sz)
	{
		const Char *end = str + (buf_sz > str_sz ? str_sz : buf_sz - 1);

		while (str != end)
			*ptr++ = *str++;

		*ptr = 0;
	}
	
	return ptr - buf;
}


std::string to_string(const wchar_t *str, int len = -1);
inline std::string to_string(const std::wstring &str)
	{ return my::str::to_string(str.c_str(), (int)str.size()); }

std::wstring to_wstring(const char *str, int len = -1);
inline std::wstring to_wstring(const std::string &str)
	{ return my::str::to_wstring(str.c_str(), (int)str.size()); }


enum
{
	escape_double_quotes = 1,
	escape_single_quotes = 2,
	escape_all_quotes = 3,
	quote_by_double_quotes = 4,
	quote_by_single_quotes = 8,
	quote_only_if_need = 16,
	escape_default = 21,
	escape_cntrl_only = 0
};

std::string escape(const char *str, int flags = escape_default, int len = -1);
inline std::string escape(const std::string &str, int flags = escape_default)
	{ return my::str::escape(str.c_str(), flags, (int)str.size()); }

std::wstring escape(const wchar_t *str, int flags = escape_default, int len = -1);
inline std::wstring escape(const std::wstring &str, int flags = escape_default)
	{ return my::str::escape(str.c_str(), flags, (int)str.size()); }

std::string to_hex(const char *str, int len = -1);
inline std::string to_hex(const std::string &str)
	{ return my::str::to_hex(str.c_str(), (int)str.size()); }

template<class Char>
std::string from_hex(const Char *str, int len = -1)
{
	/* Проверка на ошибки: out.size() != len */

	const Char *ptr_in = str;

	if (len < 0)
		len = my::str::length(str);

	const Char *end_in = ptr_in + len;

	std::string out( (len + 1) / 2, ' ' );

	if (len)
	{
		char *begin_out = &*out.begin();
		char *ptr_out = begin_out;

		bool even = false;

		while (ptr_in != end_in)
		{
			Char ch = *ptr_in++;
			
			if (ch >= 48 && ch <= 57) /* 0..9 */
				ch -= 48;
			else if (ch >= 65 && ch <= 70) /* A..F */
				ch -= 55;
			else if (ch >= 97 && ch <= 102) /* a..f */
				ch -= 87;
			else
				break;

			if (!even)
				*ptr_out = (char)(ch << 4);
			else
				*ptr_out++ = *ptr_out | (char)ch;

			even = !even;
		}

		out.resize(ptr_out - begin_out);
	}

	return out;
}

template<class Char>
inline std::string from_hex(const std::basic_string<Char> &str)
	{ return my::str::from_hex(str.c_str(), (int)str.size()); }

} }

#endif
