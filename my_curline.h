#ifndef MY_CURLINE_H
#define MY_CURLINE_H

#include <boost/current_function.hpp>

#define MYCURLINE_(F,L,Fu) ((std::string)"File: " + F + ", line: " + #L + ", func: " + Fu)
#define MYCURLINE MYCURLINE_(__FILE__,__LINE__,BOOST_CURRENT_FUNCTION)

#endif
