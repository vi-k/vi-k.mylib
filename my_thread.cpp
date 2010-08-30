#include "my_thread.h"

#include <stdio.h>
#include <string>
#include <sstream>
#include <iostream>


namespace my
{

my::log locks_log(L"locks.log", my::log::clean);

typedef boost::unordered_map<unsigned int, std::wstring> threads_map;

threads_map g_threads;

unsigned int get_thread_id()
{
	unsigned int id = 0;
	std::wstringstream ss;
	ss << boost::this_thread::get_id();
	swscanf(ss.str().c_str(), L"0x%x", &id);
	return id;
}

void register_thread(const std::wstring &name)
{
	g_threads[ get_thread_id() ] = name;
}

std::wstring& get_thread_name()
{
	std::wstring &str = g_threads[ get_thread_id() ];

	if (str.empty())
	{
		std::wstringstream ss;
		ss << boost::this_thread::get_id();
		str = ss.str();
	}

	return str;
}

}
