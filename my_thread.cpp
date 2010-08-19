#include "my_thread.h"

#include <stdio.h>
#include <string>
#include <sstream>


namespace my
{

typedef boost::unordered_map<unsigned int, std::string> threads_map;

threads_map g_threads;

unsigned int get_thread_id()
{
	unsigned int id = 0;
	std::stringstream ss;
	ss << boost::this_thread::get_id();
	sscanf(ss.str().c_str(), "0x%x", &id);
	return id;
}

void register_thread(const std::string &name)
{
	g_threads[ get_thread_id() ] = name;
}

std::string& get_thread_name()
{
	std::string &str = g_threads[ get_thread_id() ];

	if (str.empty())
	{
		std::stringstream ss;
		ss << boost::this_thread::get_id();
		str = ss.str();
	}

	return str;
}

}
