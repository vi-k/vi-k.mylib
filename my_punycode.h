#ifndef MY_PUNYCODE_H
#define MY_PUNYCODE_H

#include <cstddef> /* std::size_t */
#include <string>

namespace my {

/* Преобразование в/из Punycode */
std::string punycode_encode(const wchar_t *str, std::size_t length);
std::wstring punycode_decode(const char *str, std::size_t length);

}

#endif
