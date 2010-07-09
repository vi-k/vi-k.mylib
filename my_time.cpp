#include "my_time.h"
#include "my_exception.h"

using namespace std;

#include <boost/date_time/time_parsing.hpp>
#include <boost/date_time/c_local_time_adjustor.hpp>

namespace boost {
std::size_t hash_value(const posix_time::ptime &t)
{
	int size = sizeof(t) / sizeof(size_t);

	size_t seed = 0;
	size_t *ptr = (size_t*)&t;

	while (size--)
		boost::hash_combine(seed, *ptr);

	return seed;
}
}

namespace my { namespace time {

posix_time::ptime utc_now()
{
	return posix_time::microsec_clock::universal_time();
}

posix_time::ptime local_now()
{
	return posix_time::microsec_clock::local_time();
}

double div(
	const posix_time::time_duration &time1,
	const posix_time::time_duration &time2)
{
	return (double)time1.ticks() / (double)time2.ticks();
}

posix_time::time_duration div(
	const posix_time::time_duration &time,
	double divisor)
{
	return posix_time::time_duration(0, 0, 0,
		posix_time::time_duration::fractional_seconds_type(
			(double)time.ticks() / divisor + 0.5));
}

posix_time::time_duration mul(
	const posix_time::time_duration &time,
	double rhs)
{
	return posix_time::time_duration(0, 0, 0,
		posix_time::time_duration::fractional_seconds_type(
			(double)time.ticks() * rhs + 0.5));
}

posix_time::ptime utc_to_local(const posix_time::ptime &utc_time)
{
	return boost::date_time::c_local_adjustor<posix_time::ptime>
		::utc_to_local(utc_time);
}

posix_time::ptime local_to_utc(const posix_time::ptime &local_time)
{
	/* Прямой функции для c_local_adjustor почему-то нет */
	posix_time::time_duration td = utc_to_local(local_time) - local_time;
	return local_time - td;
}

posix_time::ptime floor(const posix_time::ptime &time,
	const posix_time::time_duration &prec)
{
	return posix_time::ptime(time.date(),
		my::time::floor(time.time_of_day(), prec));
}

posix_time::time_duration floor(const posix_time::time_duration &dur,
	const posix_time::time_duration &prec)
{
	return prec * (int)( my::time::div(dur, prec) );
}

posix_time::ptime ceil(const posix_time::ptime &time,
	const posix_time::time_duration &prec)
{
	return posix_time::ptime(time.date(),
		my::time::ceil(time.time_of_day(), prec));
}

posix_time::time_duration ceil(const posix_time::time_duration &dur,
	const posix_time::time_duration &prec)
{
	posix_time::time_duration fl = my::time::floor(dur, prec);
	return fl == dur ? fl : fl + prec;
}

posix_time::ptime round(const posix_time::ptime &time,
	const posix_time::time_duration &prec)
{
	return posix_time::ptime(time.date(),
		my::time::round(time.time_of_day(), prec));
}

posix_time::time_duration round(const posix_time::time_duration &dur,
	const posix_time::time_duration &prec)
{
	return my::time::floor(dur + prec / 2, prec);
}

}}
