#include "my_num.h"

#include <string>
#include <iostream>
using namespace std;

int main(void)
{

#define TEST_S(TN,N,T,COUT,STR) TEST_S_CAST(TN,N,T,T,COUT,STR)

#define TEST_S_CAST(TN,N,T,CAST,COUT,STR) {\
	COUT << "type=" << TN << endl;\
	STR str;\
	T n;\
	n = 0;\
	str = my::num::to_##STR(n);\
	COUT << "[0000..] " << (CAST)n << " = " << str << endl;\
	n = ~0;\
	str = my::num::to_##STR(n);\
	COUT << "[1111..] " << (CAST)n << " = " << str << endl;\
	n = (unsigned T)~0 >> 1;\
	str = my::num::to_##STR(n);\
	COUT << "[0111..] " << (CAST)n << " = " << str << endl;\
	n = ~((unsigned T)~0 >> 1);\
	str = my::num::to_##STR(n);\
	COUT << "[1000..] " << (CAST)n << " = " << str << endl;\
	n = 123;\
	str = my::num::to_##STR(n);\
	COUT << (CAST)n << " = " << str << endl;\
	n = 123;\
	str = my::num::to_##STR(n, 5);\
	COUT << (CAST)n << "[dec=5] = " << str << endl;\
	n = -123;\
	str = my::num::to_##STR(n, 5);\
	COUT << (CAST)n << "[dec=5] = " << str << endl;\
	STR::value_type buf[64];\
	size_t sz;\
	n = 123;\
	sz = my::num::put(buf, 4, n);\
	COUT << (CAST)n << "[sz=4] = " << buf << " (sz=" << sz << ")" << endl;\
	n = 123;\
	sz = my::num::put(buf, 3, n);\
	COUT << (CAST)n << "[sz=3] = " << buf << " (sz=" << sz << ")" << endl;\
	n = -123;\
	sz = my::num::put(buf, 4, n);\
	COUT << (CAST)n << "[sz=4] = " << buf << " (sz=" << sz << ")" << endl;\
	n = -123;\
	sz = my::num::put(buf, 5, n);\
	COUT << (CAST)n << "[sz=5] = " << buf << " (sz=" << sz << ")" << endl;\
	n = 123;\
	sz = my::num::put(buf, 5, n, 5);\
	COUT << (CAST)n << "[sz=5,dec=5] = " << buf << " (sz=" << sz << ")" << endl;\
	n = 123;\
	sz = my::num::put(buf, 6, n, 5);\
	COUT << (CAST)n << "[sz=6,dec=5] = " << buf << " (sz=" << sz << ")" << endl;\
	n = -123;\
	sz = my::num::put(buf, 6, n, 5);\
	COUT << (CAST)n << "[sz=6,dec=5] = " << buf << " (sz=" << sz << ")" << endl;\
	n = -123;\
	sz = my::num::put(buf, 7, n, 5);\
	COUT << (CAST)n << "[sz=7,dec=5] = " << buf << " (sz=" << sz << ")" << endl;\
	COUT << endl;}

#define TEST_U(TN,N,T,COUT,STR) TEST_U_CAST(TN,N,T,T,COUT,STR)

#define TEST_U_CAST(TN,N,T,CAST,COUT,STR) {\
	COUT << "type=" << TN << endl;\
	STR str;\
	T n;\
	n = 0;\
	str = my::num::to_##STR(n);\
	COUT << "[0000..] " << (CAST)n << " = " << str << endl;\
	n = ~0;\
	str = my::num::to_##STR(n);\
	COUT << "[1111..] " << (CAST)n << " = " << str << endl;\
	n = (T)~0 >> 1;\
	str = my::num::to_##STR(n);\
	COUT << "[0111..] " << (CAST)n << " = " << str << endl;\
	n = ~((T)~0 >> 1);\
	str = my::num::to_##STR(n);\
	COUT << "[1000..] " << (CAST)n << " = " << str << endl;\
	n = 123;\
	str = my::num::to_##STR(n);\
	COUT << (CAST)n << " = " << str << endl;\
	n = 123;\
	str = my::num::to_##STR(n, 5);\
	COUT << (CAST)n << "[dec=5] = " << str << endl;\
	STR::value_type buf[64];\
	size_t sz;\
	n = 123;\
	sz = my::num::put(buf, 4, n);\
	COUT << (CAST)n << "[sz=4] = " << buf << " (sz=" << sz << ")" << endl;\
	n = 123;\
	sz = my::num::put(buf, 3, n);\
	COUT << (CAST)n << "[sz=3] = " << buf << " (sz=" << sz << ")" << endl;\
	n = 123;\
	sz = my::num::put(buf, 5, n, 5);\
	COUT << (CAST)n << "[sz=5,dec=5] = " << buf << " (sz=" << sz << ")" << endl;\
	n = 123;\
	sz = my::num::put(buf, 6, n, 5);\
	COUT << (CAST)n << "[sz=6,dec=5] = " << buf << " (sz=" << sz << ")" << endl;\
	COUT << endl;}

	cout << "*** signed integers ***\n" << endl;
	TEST_S_CAST("char",char,char,int,cout,string)
	TEST_S("short",short,short,cout,string)
	TEST_S("int",int,int,cout,string)
	TEST_S("long",long,long,cout,string)
	TEST_S("longlong",longlong,long long,cout,string)

	cout << "*** signed integers to wstring ***\n" << endl;
	TEST_S_CAST("char",char,char,int,wcout,wstring)
	TEST_S("int",int,int,wcout,wstring)

	cout << "*** unsigned integers ***\n" << endl;
	TEST_U_CAST("uchar",uchar,unsigned char,unsigned int,cout,string)
	TEST_U("ushort",ushort,unsigned short,cout,string)
	TEST_U("uint",uint,unsigned int,cout,string)
	TEST_U("ulong",ulong,unsigned long,cout,string)
	TEST_U("ulonglong",ulonglong,unsigned long long,cout,string)

	cout << "*** unsigned integers to wstring ***\n" << endl;
	TEST_U_CAST("uchar",uchar,unsigned char,unsigned int,wcout,wstring)
	TEST_U("uint",uint,unsigned int,wcout,wstring)

	return 0;
}
