/*
	Быстрые функции преобразования чисел в строки и из строки
	
	В строку - свой алгоритм (первый придуманный алгоритм
		оказался быстрее itoa и, даже (!), Boost.Spirit.Karma (после
		создания его безопасной версии стал таким как последний).

	Из строки (to_signed, to_unsigned, to_real)- использую
		Boost.Spirit2 (ужасно медленно компилируется, поэтому шаблоны
		вынес в *.cpp (и чтоб они компилировались, а не игнорировались,
		включил функцию my_num_dummy, которая задействует все возможные
		варианты).

	----------------------------------------

	Функции преобразования числа в строку.
	
	Поддерживаемые типы: char, short, int, long, long long
		и их беззнаковые версии.
		
		----------

		template<class Char>
		size_t put(
				Char *buf,
				size_t buf_sz,
				type value,
				size_t decimals = 0);

			Преобразование числа value в буфер buf размером buf_sz.
			decimals - минимальное кол-во цифр.
			
			Возврат: размер полученной строки (без учёта
				завершающего нуля).

		----------

		template<class Char>
		std::basic_string<Char> to_str(
				type value,
				size_t decimals = 0);

		std::string to_string(
				type value,
				size_t decimals = 0);

		std::wstring to_wstring(
				type value,
				size_t decimals = 0);

			Преобразование числа value в строку.


	----------------------------------------

	Функции преобразования строки в число.

	Поддерживаемые типы: char, short, int, long, long long,
	их беззнаковые версии, float, double, long double.
		
		Используемые типы и их представление в названиях функций:
			char      -> char
			uchar     -> unsigned char
			short     -> short
			ushort    -> unsigned short
			int       -> int
			uint      -> unsigned int
			long      -> long
			ulong     -> unsigned long
			longlong  -> long long
			ulonglong -> unsigned long long

		----------

		template<class Char>
		size_t get(
				Char *str,
				size_t str_sz,
				type &res);

		template<class Char>
		size_t get(
				const std::basic_string<Char> &str,
				type &res);

			Преобразование строки в число. В первом варианте,
			если размер строки не задан (== -1), он рассчитывается
			автоматически.

			Возврат: кол-во символов, считанных из буфера или строки;
				в res - полученное значение.

		----------

		template<class Char>
		type to_type_def(
				Char *str,
				size_t str_sz,
				type def);

		template<class Char>
		type to_type_def(
				const std::basic_string<Char> &str,
				type def);

			Преобразование строки в число. В случае ошибки разбора,
			возвращается def. В первом варианте, если размер строки
			не задан (== -1), он рассчитывается автоматически.

		----------

		template<class Char>
		bool try_to_type(
				Char *str,
				size_t str_sz,
				type &res);

		template<class Char>
		bool try_to_type(
				const std::basic_string<Char> &str,
				type &res);

			Попытка преобразования строки в число. При ошибке
			преобразования число в res не меняется. В первом варианте,
			если размер строки не задан (== -1), он рассчитывается
			автоматически.
			
			Возврат: успех преобразования, в res - полученное значение.
*/

#ifndef MY_NUM_H
#define MY_NUM_H

#include <cstddef> /* std::size_t */
#include <string>

namespace my { namespace num {


/*
	Функции преобразования целых чисел в строку
*/


/* Размер временного буфера для преобразований,
	хватит на 128 бит (39 сиволов без знака) */
#define NUM_TO_STR_BUF_SIZE 64

/*
	signed_to

	Алгоритм преобразования целого числа со знаком в строку.
	Ради производительности имеет ограничение - используется
	временный буфер размером NUM_TO_STR_BUF_SIZE байт
	(128 бит требует 39 байт).
	
	Напрямую использовать не рекомендуется, т.к. шаблон принимает
	любые типы и результат такого действия не предсказуем.
*/
template<class T, class Char>
std::size_t put_signed(Char *buf, std::size_t buf_sz,
	T n, std::size_t decimals = 0)
{
	static const Char sym[]
		= { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

	/* Результат сохраняем во временный буфер в обратном порядке */
	Char tmp[NUM_TO_STR_BUF_SIZE];
	Char *tmp_ptr = tmp;
	bool neg;

	/* Сохраняем знак. Операции проводим только с отрицательными
		числами. Логичнее было бы проводить с положительными,
		но не все отрицательные числа можно перевести
		в положительные - например, (char)-128 */
	if ( (neg = n < 0) == false)
		n = -n;

	do
	{
		T nn = n / 10;
		*tmp_ptr++ = sym[ (nn<<3) + (nn<<1) - n ];
		n = nn;
	} while (n);

	/* На данный момент во временном буфере хранится число
		в обратном порядке, без знака и без завершающего нуля */

	/* Заполняем выходную строку. Отдельно, если не входим
		в ограничение размера (###), отдельно - если входим */
	Char *ptr = buf;
	Char *end = buf + buf_sz;
	
	std::size_t tmp_sz = tmp_ptr - tmp;
	std::size_t zero_count = (decimals > tmp_sz ? decimals - tmp_sz : 0);

	if (tmp_sz + zero_count + neg >= buf_sz)
	{
		if (buf_sz != 0)
		{
			Char *last = buf + buf_sz - 1;
			while (ptr != last)
   				*ptr++ = '#';

   			*ptr = 0;
		}	
   	}
   	else
   	{
		if (neg)
			*ptr++ = '-';

		Char *zero_end = ptr + zero_count;
		while (ptr < zero_end)
			*ptr++ = '0';

		while (tmp_ptr != tmp)
   			*ptr++ = *--tmp_ptr;

		*ptr = 0;   	
   	}

	return ptr - buf;
}

/*
	unsigned_to

	Алгоритм преобразования целого числа без знака в строку.
	Ради производительности имеет ограничение - используется
	временный буфер размером NUM_TO_STR_BUF_SIZE байт
	(128 бит требует 39 байт).
	
	Напрямую использовать не рекомендуется, т.к. шаблон принимает
	любые типы и результат такого действия не предсказуем.
*/

template<class T, class Char>
std::size_t put_unsigned(Char *buf, std::size_t buf_sz,
	T n, std::size_t decimals = 0)
{
	static const Char sym[]
		= { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' };

	/* Результат сохраняем во временный буфер в обратном порядке */
	Char tmp[NUM_TO_STR_BUF_SIZE];
	Char *tmp_ptr = tmp;

	do
	{
		T nn = n / 10;
		*tmp_ptr++ = sym[ n - (nn<<3) - (nn<<1) ];
		n = nn;
	} while (n);

	/* На данный момент в буфере хранится число в обратном
		порядке, без завершающего нуля */

	/* Заполняем выходную строку. Отдельно, если не входим
		в ограничение размера (###), отдельно - если входим */
	Char *ptr = buf;
	Char *end = buf + buf_sz;
	
	std::size_t tmp_sz = tmp_ptr - tmp;
	std::size_t zero_count = (decimals > tmp_sz ? decimals - tmp_sz : 0);

	if (tmp_sz + zero_count >= buf_sz)
	{
		if (buf_sz != 0)
		{
			Char *last = buf + buf_sz - 1;
			while (ptr != last)
   				*ptr++ = '#';

   			*ptr = 0;
		}	
   	}
   	else
   	{
		Char *zero_end = ptr + zero_count;
		while (ptr < zero_end)
			*ptr++ = '0';

		while (tmp_ptr != tmp)
   			*ptr++ = *--tmp_ptr;

		*ptr = 0;   	
   	}

	return ptr - buf;
}

/*
	signed_to_str и unsigned_to_str
	Напрямую использовать не рекомендуется.
*/

template<class T, class Char>
inline std::basic_string<Char> signed_to_str(T n, std::size_t decimals = 0)
{
	Char buf[NUM_TO_STR_BUF_SIZE];
	std::size_t sz
		= put_signed<T,Char>(buf, sizeof(buf) / sizeof(*buf), n, decimals);
	return std::basic_string<Char>(buf);
}

template<class T, class Char>
inline std::basic_string<Char> unsigned_to_str(T n, std::size_t decimals = 0)
{
	Char buf[NUM_TO_STR_BUF_SIZE];
	std::size_t sz
		= put_unsigned<T,Char>(buf, sizeof(buf) / sizeof(*buf), n, decimals);
	return std::basic_string<Char>(buf);
}

#define DEF_NUM_TO_FUNCS(S,T)\
template<class Char>\
inline std::size_t put(Char *str, std::size_t size,\
	T n, std::size_t decimals = 0)\
	{ return put_##S<T,Char>(str, size, n, decimals); }\
template<class Char>\
inline std::basic_string<Char> to_str(T n, std::size_t decimals = 0)\
	{ return S##_to_str<T,Char>(n, decimals); }\
inline std::string to_string(T n, std::size_t decimals = 0)\
	{ return to_str<char>(n, decimals); }\
inline std::wstring to_wstring(T n, std::size_t decimals = 0)\
	{ return to_str<wchar_t>(n, decimals); }

DEF_NUM_TO_FUNCS(signed,char)
DEF_NUM_TO_FUNCS(signed,short)
DEF_NUM_TO_FUNCS(signed,int)
DEF_NUM_TO_FUNCS(signed,long)
DEF_NUM_TO_FUNCS(signed,long long)

DEF_NUM_TO_FUNCS(unsigned,unsigned char)
DEF_NUM_TO_FUNCS(unsigned,unsigned short)
DEF_NUM_TO_FUNCS(unsigned,unsigned int)
DEF_NUM_TO_FUNCS(unsigned,unsigned long)
DEF_NUM_TO_FUNCS(unsigned,unsigned long long)


/*
	Функции преобразования строки в число
*/

template<class Char, class Type>
inline std::size_t get_signed(const Char *str, std::size_t str_sz, Type &res);

template<class Char, class Type>
inline Type to_signed_def(const Char *str, std::size_t str_sz, Type def);

template<class Char, class Type>
inline bool try_to_signed(const Char *str, std::size_t str_sz, Type &res);


template<class Char, class Type>
inline std::size_t get_unsigned(const Char *str, std::size_t str_sz, Type &res);

template<class Char, class Type>
inline Type to_unsigned_def(const Char *str, std::size_t str_sz, Type def);

template<class Char, class Type>
inline bool try_to_unsigned(const Char *str, std::size_t str_sz, Type &res);


template<class Char, class Type>
inline std::size_t get_real(const Char *str, std::size_t str_sz, Type &res);

template<class Char, class Type>
inline Type to_real_def(const Char *str, std::size_t str_sz, Type def);

template<class Char, class Type>
inline bool try_to_real(const Char *str, std::size_t str_sz, Type &res);


#define DEF_TO_NUM_FUNCS(S,N,T)\
template<class Char>\
inline std::size_t get(const Char *str, std::size_t str_sz, T &res)\
	{ return get_##S<Char,T>(str, str_sz, res); }\
template<class Char>\
inline T to_##N##_def(const Char *str, std::size_t str_sz, T def)\
	{ return to_##S##_def<Char,T>(str, str_sz, def); }\
template<class Char>\
inline bool try_to_##N(const Char *str, std::size_t str_sz, T &res)\
	{ return try_to_##S<Char,T>(str, str_sz, res); }\
template<class Char>\
inline std::size_t get(const std::basic_string<Char> &str, T &res)\
	{ return get_##S<Char,T>(str.c_str(), str.size(), res); }\
template<class Char>\
inline T to_##N##_def(const std::basic_string<Char> &str, T def)\
	{ return to_##S##_def<Char,T>(str.c_str(), str.size(), def); }\
template<class Char>\
inline bool try_to_##N(const std::basic_string<Char> &str, T &res)\
	{ return try_to_##S<Char,T>(str.c_str(), str.size(), res); }


DEF_TO_NUM_FUNCS(signed,char,char)
DEF_TO_NUM_FUNCS(signed,short,short)
DEF_TO_NUM_FUNCS(signed,int,int)
DEF_TO_NUM_FUNCS(signed,long,long)
DEF_TO_NUM_FUNCS(signed,longlong,long long)

DEF_TO_NUM_FUNCS(unsigned,uchar,unsigned char)
DEF_TO_NUM_FUNCS(unsigned,ushort,unsigned short)
DEF_TO_NUM_FUNCS(unsigned,uint,unsigned int)
DEF_TO_NUM_FUNCS(unsigned,ulong,unsigned long)
DEF_TO_NUM_FUNCS(unsigned,ulonglong,unsigned long long)

DEF_TO_NUM_FUNCS(real,float,float)
DEF_TO_NUM_FUNCS(real,double,double)
DEF_TO_NUM_FUNCS(real,long_double,long double)

}}

#endif
