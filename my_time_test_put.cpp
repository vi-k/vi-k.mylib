#include "stdafx.h"

#include "my_time.h"

#include <boost/io/ios_state.hpp>

#include <iostream>
#include <sstream>
#include <string>
using namespace std;


int main()
{

#define TEST_SPECIAL(TN,T,COUT,STR) {\
	T t;\
	COUT << "type=" << TN << endl;\
	t = T(date_time::neg_infin);\
	COUT << "[neg_infin] " << t << " = " << flush;\
	COUT << my::time::to_##STR(t) << endl;\
	t = T(date_time::pos_infin);\
	COUT << "[pos_infin] " << t << " = " << flush;\
	COUT << my::time::to_##STR(t) << endl;\
	t = T(date_time::not_a_date_time);\
	COUT << "[not_a_date_time] " << t << " = " << flush;\
	COUT << my::time::to_##STR(t) << endl;\
	t = T(date_time::min_date_time);\
	COUT << "[min_date_time] " << t << " = " << flush;\
	COUT << my::time::to_##STR(t) << endl;\
	t = T(date_time::max_date_time);\
	COUT << "[max_date_time] " << t << " = " << flush;\
	COUT << my::time::to_##STR(t) << endl;\
	COUT << endl; }

	cout << "*** special values ***\n" << endl;
	TEST_SPECIAL("gregorian::date",gregorian::date,cout,string)
	TEST_SPECIAL("posix_time::time_duration",posix_time::time_duration,cout,string)
	TEST_SPECIAL("posix_time::ptime",posix_time::ptime,cout,string)


#define TEST(t,fmt)\
	if (*fmt) {\
		cout << "fmt  = " << fmt << endl;\
		cout << "str  = " << my::time::to_string(t,fmt) << flush << endl;\
		wcout << "wstr = " << my::time::to_wstring(t,L##fmt) << flush << endl;\
	} else {\
		cout << "str  = " << my::time::to_string(t) << flush << endl;\
		wcout << "wstr = " << my::time::to_wstring(t) << flush << endl;\
	}


#define TEST_D(y,m,d) TEST_D_F(y,m,d,"")

#define TEST_D_F(y,m,d,fmt) {\
	gregorian::date t(y,m,d);\
	cout << "       " << t << " (" << y << "," << m << "," << d << ")" << endl << flush;\
	TEST(t,fmt)\
	cout << endl;}


#define TEST_TD(h,m,s,f) TEST_TD_F(h,m,s,f,"")

#define TEST_TD_F(h,m,s,f,fmt) {\
	posix_time::time_duration t(h,m,s,f);\
	cout << "       " << t << " (" << h << "," << m << "," << s\
		<< "," << f << ")" << endl << flush;\
	TEST(t,fmt)\
	cout << endl;}


#define TEST_PT(y,mo,d,h,mi,s,f) TEST_PT_F(y,mo,d,h,mi,s,f,"")

#define TEST_PT_F(y,mo,d,h,mi,s,f,fmt) {\
	posix_time::ptime t(gregorian::date(y,mo,d),\
		posix_time::time_duration(h,mi,s,f));\
	cout << "       " << t << " (" << y << "," << mo << "," << d << ","\
		<< h << "," << mi << "," << s << "," << f << ")" << endl << flush;\
	TEST(t,fmt)\
	cout << endl;}


	cout << "\n*** date/time ***\n" << endl;
	
	TEST_D(1945,5,9)
	TEST_D(2010,1,1)
	TEST_D(2010,12,31)
	TEST_D_F(1400,1,1,"%Y.%m.%d")
	TEST_D_F(1777,2,23,"%d-%m-%Y")
	TEST_D_F(1992,6,12,"%Y/%m/%d %H:%M:%S%F")
	TEST_D_F(1999,12,31,"symbols test %% %Q%* %")

	
	cout << "\n*** time_duration ***\n" << endl;

	TEST_TD(0,0,0,0)
	TEST_TD(0,0,0,1)
	TEST_TD(0,0,1,1)
	TEST_TD(0,1,0,1)
	TEST_TD(1,0,0,1)
	TEST_TD(0,0,0,-1)
	TEST_TD(0,0,-1,1)
	TEST_TD(0,-1,0,1)
	TEST_TD(-1,0,0,1)
	TEST_TD(-11,-11,-11,-11)

	TEST_TD(2147483647,0,0,0)
	TEST_TD(2147483648,0,0,0)
	cout << "!!! time_duration(2147483648,0,0,0).is_negative()="
		<< posix_time::time_duration(2147483648,0,0,0).is_negative()
		<< endl;
	cout << "!!! time_duration(2147483648,0,0,0) > time_duration(2147483647,0,0,0)="
		<< (posix_time::time_duration(2147483648,0,0,0)
			> posix_time::time_duration(2147483647,0,0,0))
		<< endl;
	cout << "!!! Boost.DateTime cast problem\n\n";
		
	TEST_TD(22,27,9,0)

	TEST_TD_F(12,34,45,123456,"%-%H:%M:%S%F")
	TEST_TD_F(12,34,45,123456,"%+%H:%M:%S%F")
	TEST_TD_F(-1,2,3,456,"%- %Hh %Mm %S%Fs")
	TEST_TD_F(-1,2,3,456,"%+ %Hh %Mm %S%Fs")
	TEST_TD_F(-1234,0,0,0,"%H.%M.%S%F")
	TEST_TD_F(-1234,0,0,0,"%H.%M.%S%f")
	TEST_TD_F(12,30,0,0,"%Y/%m/%d %H:%M:%S%F")
	TEST_TD_F(0,0,0,0,"symbols test %% %Q%* %")

	
	cout << "\n*** ptime ***\n" << endl;

	TEST_PT( 2010,6,10, 23,19,0,0)
	TEST_PT_F(1992,6,12, 12,30,0,0, "%Y/%m/%d %H:%M:%S%F")
	TEST_PT_F(1992,6,12, 12,30,0,0, "%H:%M:%S%F %d.%m.%Y")
	TEST_TD_F(0,0,0,0,"symbols test %% %Q%* %")
	TEST_TD_F(0,0,0,123456,"big format <%f%f%f%f%f%f%f%f%f%f>")

	cout << my::time::floor(posix_time::time_duration(0,0,3,500000),
		posix_time::time_duration(0,0,1,000000)) << endl;
	cout << my::time::ceil(posix_time::time_duration(0,0,3,500000),
		posix_time::time_duration(0,0,1,000000)) << endl;
	cout << my::time::round(posix_time::time_duration(0,0,3,500000),
		posix_time::time_duration(0,0,1,000000)) << endl;
	cout << my::time::round(posix_time::time_duration(0,0,3,499999),
		posix_time::time_duration(0,0,1,000000)) << endl;

	cout << my::time::floor(posix_time::ptime(
			gregorian::date(2009,12,31),
			posix_time::time_duration(23,59,59,500000)),
		posix_time::time_duration(0,0,1,000000)) << endl;
	cout << my::time::ceil(posix_time::ptime(
			gregorian::date(2009,12,31),
			posix_time::time_duration(23,59,59,500000)),
		posix_time::time_duration(0,0,1,000000)) << endl;
	cout << my::time::round(posix_time::ptime(
			gregorian::date(2009,12,31),
			posix_time::time_duration(23,59,59,500000)),
		posix_time::time_duration(0,0,1,000000)) << endl;
	cout << my::time::round(posix_time::ptime(
			gregorian::date(2009,12,31),
			posix_time::time_duration(23,59,59,499999)),
		posix_time::time_duration(0,0,1,000000)) << endl;

	cout << my::time::div( posix_time::time_duration(24,0,0), 48.0) << endl;

	return 0;
}