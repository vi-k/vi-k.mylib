#include "stdafx.h"

#include "my_time.h"
//#include "my_num.cpp"

#include <boost/io/ios_state.hpp>

#include <iostream>
#include <sstream>
#include <string>
using namespace std;

template<class Time, class Char>
void check(const Char *cstr)
{
    basic_string<Char> str(cstr);

	Time my_t;
	my::time::get(str, my_t);

	basic_istringstream<Char> is(str);
	Time boost_t;
	is >> boost_t;
	if (my_t != boost_t)
		cout << "FAILED: " << my_t << " != " << boost_t << endl << endl;
}

int main()
{
	#if 0
	posix_time::ptime pt1( gregorian::date(2010,06,02),
		posix_time::time_duration(20,41,35,653) );

	posix_time::time_duration td1(0,20,35,500000);

	cout << "set_output_format\n";
	{
		boost::io::ios_all_saver ios_saver(cout);

		my::time::set_output_format(cout, "[%Y.%m.%d %H:%M]");
		cout << pt1 << endl;
		cout << posix_time::ptime() << endl;

		my::time::set_output_format(cout, "{%Hh %Mm %Ss}");
		cout << td1 << endl;
		cout << posix_time::time_duration() << endl;

		my::time::set_output_format(cout, "[%Y.%m.%d %H:%M]", "{%Hh %Mm %Ss}");
		cout << "user fmt: " << pt1 << ' ' << td1 << endl;

		my::time::set_mydef_output_format(cout);
		cout << "my fmt: " << pt1 << ' ' << td1 << endl;
	}

	cout << "sys fmt: " << pt1 << ' ' << td1 << endl;

	cout << "\nset_input_format\n";
	{
		posix_time::ptime pt2;

		stringstream ss;
		boost::io::ios_all_saver ios_saver(ss);
		
		my::time::set_output_format(ss, "[%Y.%m.%d %H:%M]");

		ss << pt1;
		cout << ss.str() << endl;
		my::time::set_input_format(ss, "%Y-%m-%d %H:%M");
		pt2 = posix_time::ptime();
		ss >> pt2;
		cout << pt2 << endl;
		if (!ss)
			cout << "failed\n";

		ss.str("");
		ss.clear();

		ss << pt1;
		cout << ss.str() << endl;
		my::time::set_input_format(ss, "[%Y.%m.%d %H:%M]");
		ss >> pt2;
		cout << pt2 << endl;
		if (!ss)
			cout << "failed\n";

		ss.str("");
		ss.clear();

		my::time::set_mydef_output_format(ss);
		ss << pt1;
		cout << ss.str() << endl;
		my::time::set_mydef_input_format(ss);
		ss >> pt2;
		cout << pt2 << endl;
		if (!ss)
			cout << "failed\n";
	}

	cout << "\nformat\n";
	cout << my::time::format("%Y-%b-%d (%A) %H:%M:%S", pt1) << endl;
	cout << my::time::format("%Y-%b-%d (%A) %H:%M:%S", posix_time::ptime()) << endl;
	cout << my::time::format("%+%H:%M:%S", td1) << endl;

	cout << "\nto_str\n";
	cout << my::time::to_str<char>(pt1) << endl;
	cout << my::time::to_str<char>(posix_time::ptime()) << endl;
	cout << my::time::to_str<char>(td1) << endl;

	cout << "\nformat_to\n";
	string str("1978-Jun-29 00:05:30");
	cout << str << " -> ";
	cout << my::time::format_to_time("%Y-%b-%d %H:%M:%S", str) << endl;

	str = "1976-03-05 14:00:00";
	cout << str << " -> ";
	cout << my::time::format_to_time("%Y-%b-%d %H:%M:%S", str) << endl;

	str = "{23h 45m 23s}";
	cout << str << " -> ";
	cout << my::time::format_to_duration("{%Hh %Mm %Ss}", str) << endl;

	str = "12:00:00";
	cout << str << " -> ";
	cout << my::time::format_to_duration("{%Hh %Mm %Ss}", str) << endl;

	/*-
	cout << "\nstr_to\n";
	str = "2010-01-02 12:30:00";
	cout << str << " -> ";
	cout << my::time::to_time(str) << endl;

	str = "2010-13-02 12:30:00";
	cout << str << " -> ";
	cout << my::time::to_time(str) << endl;
    -*/

    cout << endl;

    #endif

#define TEST(NN,N,T,STR,SZ) {\
	const char cstr[] = STR;\
	string str(STR);\
	const wchar_t wcstr[] = L##STR;\
	wstring wstr(L##STR);\
	cout << "type=" << NN << endl;\
	cout << "string=\"" << STR << "\"" << endl;\
	\
	T t;\
	std::size_t n;\
	n = my::time::get(cstr, SZ, t);\
	cout << "get(cstr," << SZ << ")=" << n << " "##NN##"=" << t << endl;\
	if (SZ==-1) {\
		n = my::time::get(str, t);\
		cout << "get(str)=" << n << " "##NN##"=" << t << endl;}\
	n = my::time::get(wcstr, SZ, t);\
	cout << "get(wcstr," << SZ << ")=" << n << " "##NN##"=" << t << endl;\
	if (SZ==-1) {\
		n = my::time::get(wstr, t);\
		cout << "get(wstr)=" << n << " "##NN##"=" << t << endl;}\
	t = my::time::to_##N(cstr, SZ);\
	cout << "to_"##NN##"(cstr," << SZ << ")=" << t << endl;\
	if (SZ==-1) {\
		t = my::time::to_##N(str);\
		cout << "to_"##NN##"(str)=" << t << endl;}\
	t = my::time::to_##N(wcstr, SZ);\
	cout << "to_"##NN##"(wcstr," << SZ << ")=" << t << endl;\
	if (SZ==-1) {\
		t = my::time::to_##N(wstr);\
		cout << "to_"##NN##"(wstr)=" << t << endl;}\
	cout << endl; }

	TEST("date",date,gregorian::date,"2010-12-02",-1)
	TEST("date",date,gregorian::date,"2010/13/02",-1)
	TEST("date",date,gregorian::date,"2010/01/01",-1)
	TEST("date",date,gregorian::date,"2010/01",-1)
	TEST("date",date,gregorian::date,"25.01.2002",-1)
	TEST("date",date,gregorian::date,"25.01-2002",-1)
	TEST("date",date,gregorian::date,"25.01.2002 ",-1)
	TEST("date",date,gregorian::date,"29-02-2010",-1)
	TEST("date",date,gregorian::date,"29 06 1978",-1)
	TEST("date",date,gregorian::date,"2010-06-12",10)
	TEST("date",date,gregorian::date,"2010-06-12",9)
	TEST("date",date,gregorian::date,"2010-06-12",8)
	TEST("date",date,gregorian::date,"2010-06-12",0)

	
	TEST("duration",duration,posix_time::time_duration,"00:00:00",-1)
	TEST("duration",duration,posix_time::time_duration,"12:34:56.123456789",-1)
	TEST("duration",duration,posix_time::time_duration,"12:34:56.123456",-1)
	TEST("duration",duration,posix_time::time_duration,"12:34:56.123",-1)
	TEST("duration",duration,posix_time::time_duration,"12:34:56",-1)
	TEST("duration",duration,posix_time::time_duration,"12:34",-1)
	TEST("duration",duration,posix_time::time_duration,"12",-1)

	TEST("duration",duration,posix_time::time_duration,"-12:34:56.123456",-1)
	TEST("duration",duration,posix_time::time_duration,"-12:34",-1)

	TEST("duration",duration,posix_time::time_duration,"00:00:60",-1)
	TEST("duration",duration,posix_time::time_duration,"00:60:00",-1)
	TEST("duration",duration,posix_time::time_duration,"24:00:00",-1)

	TEST("duration",duration,posix_time::time_duration,"12:34:56.123456",10)
	TEST("duration",duration,posix_time::time_duration,"12:34:56.12",9)
	TEST("duration",duration,posix_time::time_duration,"12:34:56.12",8)
	TEST("duration",duration,posix_time::time_duration,"12:34:56.12",1)
	TEST("duration",duration,posix_time::time_duration,"12:34:56.12",0)


	TEST("time",time,posix_time::ptime,"2010-06-09 23:59:34",-1)
	TEST("time",time,posix_time::ptime,"2010-06-09 24:00:00",-1)
	TEST("time",time,posix_time::ptime,"2010-06-09 -00:00:01",-1)
	TEST("time",time,posix_time::ptime,"2010/06/09 12:00:00.123456",-1)
	TEST("time",time,posix_time::ptime,"2010.06.09 12:00:00.123456 ",-1)
	TEST("time",time,posix_time::ptime,"29 06 2010 09:30:00 ",-1)

	return 0;
}