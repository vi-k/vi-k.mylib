#include "my_num.h"
#include "my_qi.h"
#include "my_str.h"

using namespace std;

namespace my { namespace num {

/* Шаблоны для внутреннего использования - для преобразования
	используют Boost.Spirit (хоть я и разочарован в скорости его компиляции) */
template<class Char, class Type, class Rule>
std::size_t get_num(const Char *str, std::size_t str_sz, Rule rule, Type &res)
{
	const Char *ptr = str;
	const Char *end = my::str::end(str, str_sz);

	qi::parse(ptr, end, rule, res);

	return ptr - str;
}

template<class Char, class Type, class Rule>
Type to_num_def(const Char *str, std::size_t str_sz, Rule rule, Type def)
{
	const Char *ptr = str;
	const Char *end = my::str::end(str, str_sz);

	Type res;
	bool ok = qi::parse(ptr, end, rule, res);
	
	return ok && ptr == end ? res : def;
}

template<class Char, class Type, class Rule>
bool try_to_num(const Char *str, std::size_t str_sz, Rule rule, Type &res)
{
	const Char *ptr = str;
	const Char *end = my::str::end(str, str_sz);
	
	Type tmp;
	bool ok = qi::parse(ptr, end, rule, tmp);
	
	if (!ok || ptr != end)
		return false;

	res = tmp;
	return true;
}


template<class Char, class Type>
inline std::size_t get_signed(const Char *str, std::size_t str_sz, Type &res)
	{ return get_num(str, str_sz, qi::int_parser<Type, 10, 1, -1>(), res); }

template<class Char, class Type>
inline Type to_signed_def(const Char *str, std::size_t str_sz, Type def)
	{ return to_num_def(str, str_sz, qi::int_parser<Type, 10, 1, -1>(), def); }

template<class Char, class Type>
inline bool try_to_signed(const Char *str, std::size_t str_sz, Type &res)
	{ return try_to_num(str, str_sz, qi::int_parser<Type, 10, 1, -1>(), res); }


template<class Char, class Type>
inline std::size_t get_unsigned(const Char *str, std::size_t str_sz, Type &res)
	{ return get_num(str, str_sz, qi::uint_parser<Type, 10, 1, -1>(), res); }

template<class Char, class Type>
inline Type to_unsigned_def(const Char *str, std::size_t str_sz, Type def)
	{ return to_num_def(str, str_sz, qi::uint_parser<Type, 10, 1, -1>(), def); }

template<class Char, class Type>
inline bool try_to_unsigned(const Char *str, std::size_t str_sz, Type &res)
	{ return try_to_num(str, str_sz, qi::uint_parser<Type, 10, 1, -1>(), res); }


template<class Char, class Type>
inline std::size_t get_real(const Char *str, std::size_t str_sz, Type &res)
	{ return get_num(str, str_sz, qi::real_parser<Type>(), res); }

template<class Char, class Type>
inline Type to_real_def(const Char *str, std::size_t str_sz, Type def)
	{ return to_num_def(str, str_sz, qi::real_parser<Type>(), def); }

template<class Char, class Type>
inline bool try_to_real(const Char *str, std::size_t str_sz, Type &res)
	{ return try_to_num(str, str_sz, qi::real_parser<Type>(), res); }


}}

int my_num_dummy()
//int main()
{

#define DUMMY(N,T) {\
	T n;\
	my::num::get<char>(0,0,n);\
	my::num::to_##N##_def<char,T>(0,0,n);\
	my::num::try_to_##N<char>(0,0,n);\
	my::num::get<wchar_t>(0,0,n);\
	my::num::to_##N##_def<wchar_t>(0,0,n);\
	my::num::try_to_##N<wchar_t,T>(0,0,n);}

	DUMMY(signed,char)
	DUMMY(signed,short)
	DUMMY(signed,int)
	DUMMY(signed,long)
	DUMMY(signed,long long)

	DUMMY(unsigned, unsigned char)
	DUMMY(unsigned, unsigned short)
	DUMMY(unsigned, unsigned int)
	DUMMY(unsigned, unsigned long)
	DUMMY(unsigned, unsigned long long)

	DUMMY(real, float)
	DUMMY(real, double)
	DUMMY(real, long double)

	return 0;
}
