/*
	Смысл всех этих самописных функций преобразования в том, что
	стандартные функции из boost::date_time грандиозно медленны
	
	Тест (преобразование 1000 строк, avg - среднее время):

	1) my::time::to_duration: avg=00:00:00.000250
	2) duration_from_string:  avg=00:00:00.014063 (в 56 раз)
	3) istringstream >> (без создания потока на каждой итерации):
	                          avg=00:00:00.013594 (в 54 раза)
	4) + создание потока:     avg=00:00:00.921929 (в 3690 раз)
	5) + форматирование:      avg=00:00:00.945355 (в 3780 раз)
*/

#ifndef MY_TIME_H
#define MY_TIME_H

#include "my_exception.h"
#include "my_num.h"
#include "my_str.h"

#include <cstddef> /* std::size_t */
#include <algorithm> /* std::copy, std::max*/
#include <sstream>
#include <istream>
#include <ostream>
#include <string>
#include <locale> /* facet */

#include <boost/date_time/special_defs.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/local_time/local_time.hpp>

namespace gregorian=boost::gregorian;
namespace posix_time=boost::posix_time;
namespace local_time=boost::local_time;
namespace date_time=boost::date_time;

#include <boost/functional/hash.hpp>

/* hash_value для boost::unordered_map */
namespace boost {
std::size_t hash_value(const posix_time::ptime &t);
}

namespace my { namespace time {

/* hash для std::unordered_map */
struct ptime_hash : std::unary_function<posix_time::ptime, std::size_t>
{
	std::size_t operator()(const posix_time::ptime &t) const
	{
		int size = sizeof(t) / sizeof(std::size_t);

		std::size_t seed = 0;
		std::size_t *ptr = (std::size_t*)&t;

		while (size--)
			boost::hash_combine(seed, *ptr);

		return seed;
	}
};

posix_time::ptime utc_now();
posix_time::ptime local_now();

double div(
	const posix_time::time_duration &time1,
	const posix_time::time_duration &time2);

posix_time::time_duration div(
	const posix_time::time_duration &time,
	double divisor);

posix_time::time_duration mul(
	const posix_time::time_duration &time,
	double rhs);

/* UTC <-> local */
posix_time::ptime utc_to_local(const posix_time::ptime &utc_time);
posix_time::ptime local_to_utc(const posix_time::ptime &local_time);

/* Округление времени с точностью до prec.
	Возвращает значение, равное или меньшее заданному:
		floor(00:00:03.500, 00:00:01) = 00:00:03 */
posix_time::ptime floor(const posix_time::ptime &time,
	const posix_time::time_duration &prec);
posix_time::time_duration floor(const posix_time::time_duration &dur,
	const posix_time::time_duration &prec);

/* Округление времени с точностью до prec.
	Возвращает значение, равное или большее заданному:
		ceil(00:00:03.500, 00:00:01) = 00:00:04 */
posix_time::ptime ceil(const posix_time::ptime &time,
	const posix_time::time_duration &prec);
posix_time::time_duration ceil(const posix_time::time_duration &dur,
	const posix_time::time_duration &prec);

/* Округление времени с точностью до prec.
	Возвращает значение, равное или ближайшее заданному:
		round(00:00:03.500, 00:00:01) = 00:00:04
		round(00:00:03.499, 00:00:01) = 00:00:03 */
posix_time::ptime round(const posix_time::ptime &time,
	const posix_time::time_duration &prec);
posix_time::time_duration round(const posix_time::time_duration &dur,
	const posix_time::time_duration &prec);


template<class Time>
void throw_if_fail(const Time &time,
	const std::wstring &text = std::wstring(L"Invalid time"))
{
	if (time.is_special())
		throw my::exception(text)
			<< my::param(L"time", time);
}


/*
	Функции преобразования даты/времени в строку
*/

template<class Time>
date_time::special_values as_special(const Time &t)
{
	if (t.is_not_a_date_time())
		return date_time::not_a_date_time;
	else if (t.is_neg_infinity())
		return date_time::neg_infin;
	else if (t.is_pos_infinity())
		return date_time::pos_infin;

	return date_time::not_special;
}

template<class Char>
std::size_t put(Char *buf, std::size_t buf_sz,
	const date_time::special_values &sv)
{
	static const Char neg_infinity[]
		= { '-','i','n','f','i','n','i','t','y', 0 };
	static const Char pos_infinity[]
		= { '+','i','n','f','i','n','i','t','y', 0 };
	static const Char not_a_date_time[]
		= { 'n','o','t','-','a','-','d','a','t','e',
			'-','t','i','m','e', 0 };
	static const Char not_special[]
		= { 'n','o','t','-','s','p','e','c','i','a','l', 0 };

	Char *ptr = buf;
	Char *end = buf + buf_sz;

	switch (sv)
	{
		case date_time::not_a_date_time:
			ptr += my::str::put(ptr, end - ptr, not_a_date_time);
			break;

		case date_time::neg_infin:
			ptr += my::str::put(ptr, end - ptr, neg_infinity);
			break;
		
		case date_time::pos_infin:
			ptr += my::str::put(ptr, end - ptr, pos_infinity);
			break;
		
		default:
			ptr += my::str::put(ptr, end - ptr, not_special);
	}

	return ptr - buf;
}

/* Преобразование даты (boost::gregorian::date) в строку */
template<class Char>
std::size_t put(Char *buf, std::size_t buf_sz,
	const gregorian::date &date, const Char *format = 0)
{
	if (date.is_special())
		return put(buf, buf_sz, date.as_special());

	Char *ptr = buf;
	Char *end = buf + buf_sz;

	gregorian::date::ymd_type ymd = date.year_month_day();

	if (!format)
	{
		ptr += my::num::put(ptr, end - ptr, ymd.year, 4);
		ptr += my::str::put(ptr, end - ptr, Char('-'));
		ptr += my::num::put(ptr, end - ptr, ymd.month, 2);
		ptr += my::str::put(ptr, end - ptr, Char('-'));
		ptr += my::num::put(ptr, end - ptr, ymd.day, 2);
	}
	else
	{
		while (ptr < end)
		{
			const Char *format_ptr = format;

			while (*format_ptr && *format_ptr != '%')
				format_ptr++;

			ptr += my::str::put(ptr, end - ptr,
				format, format_ptr - format);

			if (*format_ptr == 0)
				break;

			Char ch = format_ptr[1];
			format = format_ptr + 2;

			switch (ch)
			{
				case 0:
				case '%':
					ptr += my::str::put(ptr, end - ptr, Char('%'));
					break;

				case 'Y':
					ptr += my::num::put(ptr, end - ptr, ymd.year, 4);
					break;

				case 'm':
					ptr += my::num::put(ptr, end - ptr, ymd.month, 2);
					break;

				case 'd':
					ptr += my::num::put(ptr, end - ptr, ymd.day, 2);
					break;

				default:
					ptr += my::str::put(ptr, end - ptr, Char('%'));
					ptr += my::str::put(ptr, end - ptr, ch);
					break;
			}

			if (ch == 0)
				break;

		} /* while (*format) */
	}

	return ptr - buf;
}

/* Преобразование даты/времени (boost::posix_time::ptime) в строку */
template<class Char>
std::size_t put(Char *buf, std::size_t buf_sz,
	const posix_time::ptime &time, const Char *format = 0)
{
	if (time.is_special())
		return put(buf, buf_sz, my::time::as_special(time));

	Char *ptr = buf;
	Char *end = buf + buf_sz;

	if (!format)
	{
		ptr += put(ptr, end - ptr, time.date());
		ptr += my::str::put(ptr, end - ptr, Char(' '));
		ptr += put(ptr, end - ptr, time.time_of_day());
	}
	else if (buf_sz)
	{
		/* Сложный момент, как из двух собрать одно с помощью
			одного формата? */

		/* Используя формат, выводим дату. Формат для времени
			сохраняется в выходном буфере */
		ptr += put(ptr, end - ptr, time.date(), format);

        /* Делаем копию полученной строки */
        std::basic_string<Char> new_format(buf);

		/* Используем скопированную строку как формат */
		ptr = buf;
		ptr += put(ptr, end - ptr, time.time_of_day(), new_format.c_str());
	}

	return ptr - buf;
}

/* Преобразование длительности (boost::posix_time::time_duration) в строку */
template<class Char>
std::size_t put(Char *buf, std::size_t buf_sz,
	const posix_time::time_duration &dur, const Char *format = 0)
{
	if (dur.is_special())
		return put(buf, buf_sz, my::time::as_special(dur));
	
	Char *ptr = buf;
	Char *end = buf + buf_sz;

	long long ticks = dur.ticks();
	bool negative = false;

	if (ticks < 0)
	{
		negative = true;
		ticks = -ticks;
	}
		
	long long total_seconds = ticks / dur.ticks_per_second();
	long long hours = total_seconds / 3600;

	long fseconds = static_cast<long>(ticks
		- total_seconds * dur.ticks_per_second());
	long seconds = static_cast<long>(total_seconds - hours * 3600);
	long minutes = seconds / 60;
	seconds -= minutes * 60;
		
	if (!format)
	{
		if (negative)
			ptr += my::str::put(ptr, end - ptr, Char('-'));

		ptr += my::num::put(ptr, end - ptr, hours, 2);
		ptr += my::str::put(ptr, end - ptr, Char(':'));
		ptr += my::num::put(ptr, end - ptr, minutes, 2);
		ptr += my::str::put(ptr, end - ptr, Char(':'));
		ptr += my::num::put(ptr, end - ptr, seconds, 2);

		if (fseconds)
		{
			ptr += my::str::put(ptr, end - ptr, Char('.'));
			ptr += my::num::put(ptr, end - ptr, fseconds,
				posix_time::time_duration::num_fractional_digits());
		}
	}
	else
	{
		while (ptr < end)
		{
			const Char *format_ptr = format;

			while (*format_ptr && *format_ptr != '%')
				format_ptr++;

			ptr += my::str::put(ptr, end - ptr,
				format, format_ptr - format);

			if (*format_ptr == 0)
				break;

			Char ch = format_ptr[1];
			format = format_ptr + 2;

			switch (ch)
			{
				case 0:
				case '%':
					ptr += my::str::put(ptr, end - ptr, Char('%'));
					break;

				case '-':
					if (negative)
						ptr += my::str::put(ptr, end - ptr, Char('-'));
					break;

				case '+':
					ptr += my::str::put(ptr, end - ptr,
						negative ? Char('-') : Char('+'));
					break;

				case 'H':
					ptr += my::num::put(ptr, end - ptr, hours, 2);
					break;

				case 'M':
					ptr += my::num::put(ptr, end - ptr, minutes, 2);
					break;

				case 'S':
					ptr += my::num::put(ptr, end - ptr, seconds, 2);
					break;

				case 'f':
					ptr += my::str::put(ptr, end - ptr, Char('.'));
					ptr += my::num::put(ptr, end - ptr, fseconds, 6);
					break;

				case 'F':
					if (fseconds)
					{
						ptr += my::str::put(ptr, end - ptr, Char('.'));
						ptr += my::num::put(ptr, end - ptr, fseconds, 6);
					}
					break;

				default:
					ptr += my::str::put(ptr, end - ptr, Char('%'));
					ptr += my::str::put(ptr, end - ptr, ch);
					break;
			}

			if (ch == 0)
				break;

		} /* while (*format) */
	}

	return ptr - buf;
}


template<class Char,class Time>
inline std::basic_string<Char> to_str(const Time &t,
	const Char *format = 0)
{
	Char buf[64];
	std::size_t n;
	
	n = my::time::put(buf, sizeof(buf) / sizeof(*buf), t, format);
	
	if (n < sizeof(buf) / sizeof(*buf) - 1)
		return std::basic_string<Char>(buf);

    /* Если статического буфера не хватило (из-за формата)
    	- выделяем динамический. И повторно запускаем put().
    	Это не есть хорошо, но пока сойдёт */
	
	if (format)
		//n = std::max(n, my::str::length(format));
	{
		std::size_t len = my::str::length(format);
		if (len > n)
			n = len;
	}

	std::basic_string<Char> out(n * 2, ' ');

	while (true)
	{
		n = my::time::put(&*out.begin(), out.size() + 1, t, format);

		if (n != out.size())
			break;

		out.resize(n * 2);
	}

	out.resize(n);

	return out;
}

template<class Time>
inline std::string to_string(const Time &time,
	const char *format = 0)
	{ return my::time::to_str<char>(time, format); }

template<class Time>
inline std::wstring to_wstring(const Time &time,
	const wchar_t *format = 0)
	{ return my::time::to_str<wchar_t>(time, format); }


/*
	Функции преобразования строки в дату/время
*/

/* Преобразование строки в дату (boost::gregorian::date) */
template<class Char>
std::size_t get(const Char *str, std::size_t str_sz,
	gregorian::date &date)
{
	static const Char sep_list[] = { '-','.','/',' ',',', 0 };

	unsigned short y(0), m(0), d(0);
	const Char *ptr = str;
	const Char *end = my::str::end(str, str_sz);

	date = gregorian::date(gregorian::not_a_date_time);

	std::size_t n;

	/* Год */
	ptr += (n = my::num::get(ptr, end - ptr, y));
	if (n != 0 && ptr != end)
	{
		/* Разделитель частей */
		Char sep = *std::find(sep_list, sep_list
			+ sizeof(sep_list) / sizeof(*sep_list) - 1, *ptr);

		if (sep)
		{
			/* Месяц */
			ptr++;
			ptr += (n = my::num::get(ptr, end - ptr, m));
			if (n != 0 && ptr != end && *ptr == sep)
			{
				/* День */
				ptr++;
				ptr += (n = my::num::get(ptr, end - ptr, d));
				if (n != 0)
				{
					/* Если дата была в виде d-m-y, переворачиваем */
					if (y < d)
					{
						unsigned short tmp = d;
						d = y;
						y = tmp;
					}

					try { date = gregorian::date(y, m, d); }
					catch(...) {}
				}
			}
		}
	}

	return ptr - str;
}

template<class Char>
inline std::size_t get(const std::basic_string<Char> &str,
	gregorian::date &date)
	{ return my::time::get(str.c_str(), str.size(), date); }

template<class Char>
inline gregorian::date to_date(const Char *str, std::size_t str_sz)
{
	gregorian::date date;

	str_sz = my::str::end(str, str_sz) - str;

	if (str_sz != my::time::get(str, str_sz, date))
		date = gregorian::date(gregorian::not_a_date_time);

	return date;
}

template<class Char>
inline gregorian::date to_date(const std::basic_string<Char> &str)
	{ return my::time::to_date(str.c_str(), str.size()); }


/* Преобразование строки в дату/время (boost::posix_time::ptime) */
template<class Char>
std::size_t get(const Char *str, std::size_t str_sz,
	posix_time::ptime &time)
{
	const Char *ptr = str;
	const Char *end = my::str::end(str, str_sz);

	time = posix_time::not_a_date_time;

	gregorian::date date;
	ptr += my::time::get(str, end - ptr, date);

	if (!date.is_special() && ptr != end && *ptr == ' ')
	{
		ptr++;

		posix_time::time_duration dur;
		ptr += my::time::get(ptr, end - ptr, dur);

		if (!dur.is_special() && !dur.is_negative() && dur.hours() < 24)
			time = posix_time::ptime(date, dur);
	}

	return ptr - str;
}

template<class Char>
inline std::size_t get(const std::basic_string<Char> &str,
	posix_time::ptime &time)
	{ return my::time::get(str.c_str(), str.size(), time); }

template<class Char>
inline posix_time::ptime to_time(const Char *str, std::size_t str_sz)
{
	posix_time::ptime time;

	str_sz = my::str::end(str, str_sz) - str;

	if (str_sz != my::time::get(str, str_sz, time))
		time = posix_time::not_a_date_time;

	return time;
}

template<class Char>
inline posix_time::ptime to_time(const std::basic_string<Char> &str)
	{ return my::time::to_time(str.c_str(), str.size()); }


/* Преобразование строки в длительность (boost::posix_time::time_duration) */
template<class Char>
std::size_t get(const Char *str, std::size_t str_sz,
	posix_time::time_duration &dur)
{
	unsigned long h(0), m(0), s(0), fs(0);
	const Char *ptr = str;
	const Char *end = my::str::end(str, str_sz);
	bool negative = false;

	dur = gregorian::not_a_date_time;

	if (ptr != end && *ptr == '-')
	{
		ptr++;
		negative = true;
	}

	std::size_t n;
	bool ok = true;

	/* Часы */
	ptr += (n = my::num::get(ptr, end - ptr, h));

	if (n == 0)
		ok = false;

	/* Минуты (могут отсутствовать) */
	else if (ptr != end && *ptr == ':')
	{	
		ptr++;
		ptr += (n = my::num::get(ptr, end - ptr, m));

		/* Но если есть разделитель, дожно быть и число */
		if (n == 0 || m >= 60)
			ok = false;
		
		/* Секунды (могут отсутствовать) */
		else if (ptr != end && *ptr == ':')
		{
			ptr++;
			ptr += (n = my::num::get(ptr, end - ptr, s));

			/* Но если есть разделитель, дожно быть и число */
			if (n == 0 || s >= 60)
				ok = false;

			/* Доли секунд (могут отсутствовать) */
			else if (ptr != end && *ptr == '.')
			{
				ptr++;
				ptr += (n = my::num::get(ptr, end - ptr, fs));

				/* Но если есть точка, должно быть и число */
				if (n == 0)
					ok = false;

				else
				{
					/* Учитываем, что значение после точки - это
						ДОЛИ секунды! Т.е. 00:00:00.12 не должно
						превратиться в 00:00:00.000012 */
					
					/* a - кол-во недостающих нулей (a>0) или
						избыток цифр (a<0) */
					long a = dur.num_fractional_digits() - n;
					
					/* Округляем при необходимости:
						00:00:00.123456789 -> 00:00:00.123457 */
					if (a < 0)
					{
						double dfs = fs;
						while (a++)
							dfs /= 10.0;
						fs = (long)(dfs + 0.5);
					}
					else
					{
						/* Добавляем недостающее:
							00:00:00.12 -> 00:00:00.120000 */
						while (a--)
							fs = (fs<<3) + (fs<<1); /* Умножение на 10 */
					}
				}
			}
		}
	}

	if (ok)
	{
		try {

			dur = posix_time::time_duration(h, m, s, fs);

			if (negative)
				dur = -dur;
		}
		catch(...) {}
	}

	return ptr - str;
}

template<class Char>
inline std::size_t get(const std::basic_string<Char> &str,
	posix_time::time_duration &dur)
	{ return my::time::get(str.c_str(), str.size(), dur); }

template<class Char>
inline posix_time::time_duration to_duration(const Char *str,
	std::size_t str_sz)
{
	posix_time::time_duration dur;

	str_sz = my::str::end(str, str_sz) - str;

	if (str_sz != my::time::get(str, str_sz, dur))
		dur = posix_time::not_a_date_time;

	return dur;
}

template<class Char>
inline posix_time::time_duration to_duration(const std::basic_string<Char> &str)
	{ return my::time::to_duration(str.c_str(), str.size()); }


}}

#endif
