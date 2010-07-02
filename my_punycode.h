#ifndef MY_PUNYCODE_H
#define MY_PUNYCODE_H

#include <cstddef> /* size_t */
#include <string>

namespace my {

std::string punycode_encode(const wchar_t *str, std::size_t length);
std::wstring punycode_decode(const char *str, std::size_t length);

}

#endif
