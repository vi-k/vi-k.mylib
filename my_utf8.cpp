#include "my_utf8.h"

#include <boost/config.hpp>

#if defined(BOOST_WINDOWS)
#include <libs/serialization/src/utf8_codecvt_facet.cpp>
#else
#include </usr/local/include/boost/libs/serialization/src/utf8_codecvt_facet.cpp>
#endif
