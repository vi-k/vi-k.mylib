#include <boost/config/warning_disable.hpp> /* против unsafe */

#include "my_http.h"
#include "my_utf8.h"
#include "my_str.h"
#include "my_qi.h"
#include "my_fs.h"
#include "my_exception.h"

#include <cstddef> /* std::size_t */
#include <errno.h>
#include <iterator>

#include <boost/regex.hpp>

namespace my { namespace http {

void parse_request(const std::string &line,
	std::string &url, params_type &params)
{
	qi::rule<std::string::const_iterator, std::string()> url_r
		= +(qi::char_ - ' ' - '?' - '&');
	qi::rule<std::string::const_iterator, std::string()> key_r
		= +(qi::char_ - ' ' - '&' - '=');
	qi::rule<std::string::const_iterator, std::string()> value_r
		= +(qi::char_ - ' ' - '&');
	qi::rule<std::string::const_iterator, param_type()> pair_r
		= key_r >> -('=' >> value_r);
	qi::rule<std::string::const_iterator, params_type()> params_r
		= qi::lit('?') >> pair_r >> *(qi::lit('&') >> pair_r);

	std::string::const_iterator iter = line.begin();

	bool res = qi::parse( iter, line.end(),
		qi::lit("GET ")
		>> url_r[phoenix::ref(url) = qi::_1]
		>> -params_r[phoenix::ref(params) = qi::_1]
		>> " HTTP/1.1\r\n"
		>> qi::eoi);

	if (!res)
		throw my::exception(L"Не удалось разобрать HTTP-запрос")
			<< my::param(L"http-request", my::str::to_wstring(line))
			<< my::param(L"position", iter - line.begin());
}

unsigned int parse_reply(const std::string &line,
	std::string &status_message)
{
	unsigned int status_code;

	std::string::const_iterator iter = line.begin();

	bool res = qi::parse( iter, line.end(),
		qi::lit("HTTP/1.1 ")
		>> qi::uint_
		>> ' '
		>> +(qi::char_ - '\r' - '\n')
		>> "\r\n"
		>> qi::eoi,
		status_code, status_message);

	if (!res)
		throw my::exception(L"Не удалось разобрать HTTP-ответ")
			<< my::param(L"http-reply", my::str::to_wstring(line))
			<< my::param(L"position", iter - line.begin());

	return status_code;
}

void parse_header(const std::string &lines, params_type &params)
{
	qi::rule<std::string::const_iterator, std::string()> key_r
		= +(qi::char_ - ' ' - ':' - '\r' - '\n');
	qi::rule<std::string::const_iterator, std::string()> value_r
		= *(qi::char_ - '\r' - '\n');
	qi::rule<std::string::const_iterator, param_type()> pair_r
		= key_r >> ": " >> value_r >> "\r\n";
	qi::rule<std::string::const_iterator, params_type()> params_r
		= *pair_r;

	std::string::const_iterator iter = lines.begin();

	bool res = qi::parse( iter, lines.end(),
		params_r >> "\r\n" >> qi::eoi,
		params);

	if (!res)
	{
		/* Считаем, в какой строке ошибка */
		int line = 1;
		int pos = iter - lines.begin();
		int bytes_in_line = 0;

		for (std::string::const_iterator it = lines.begin();
			it != iter; it++)
		{
			if (*it == '\n')
			{
				pos -= bytes_in_line;
				line++;
				bytes_in_line = 0;
			}
		}

		throw my::exception(L"Не удалось разобрать HTTP-заголовок")
			<< my::param(L"http-header", my::str::to_wstring(lines))
			<< my::param(L"line", line)
			<< my::param(L"position", pos);
	}
}

std::string percent_decode(const char *str, int len)
{
	std::string out;

	/* Для нормального преобразования символов с кодами выше 127
		необходимо использовать unsigned char, поэтому оформляем
		это дело отдельным правилом */
	qi::rule<const char *, unsigned char()> char_r
		= (qi::char_ - '%')
			| ('%' >> qi::uint_parser<unsigned char, 16, 2, 2>());

	if (len < 0)
		len = my::str::length(str);

	qi::parse(str, str + len,
		*char_r >> qi::eoi, out); /* eoi - указатель конца строки */

	return out;
}

std::string percent_encode(const char *str,
	const char *escape_symbols, int len)
{
	static const char hex[16] =
	{
		'0', '1', '2', '3', '4', '5', '6', '7',
		'8', '9', 'a', 'b', 'c', 'd', 'e', 'f'
	};

	if (len < 0)
		len = my::str::length(str);

	const char *ptr_in = str;
	/* Строка может содержать нулевой символ, поэтому
		пользуемся размером исходной строки */
	const char *end_in = ptr_in + len;

	/* Сразу готовим буфер */
	std::string out(len * 3, ' ');
	char *begin_out = (char*)out.c_str();
	char *ptr_out = begin_out;

	while (ptr_in != end_in)
	{
		char ch = *ptr_in++;
        /* Кодируются все специальные символы, все не ascii-символы (>127),
        	пробел и заказанные пользователем */
		if ( ch <= 32 || ch > 127 ||
			(escape_symbols && strchr(escape_symbols, ch)) )
		{
			*ptr_out++ = '%';
			*ptr_out++ = hex[ ((unsigned char)ch) >> 4];
			*ptr_out++ = hex[ ch & 0xf ];
		}
		else
			*ptr_out++ = ch;
	}

	out.resize(ptr_out - begin_out);

	return out;
}


void message::read_header(tcp::socket &socket)
{
	std::size_t n = asio::read_until(socket, buf_, boost::regex("^\r\n"));

	header_.resize(n);
	buf_.sgetn((char*)header_.c_str(), n);

	params_type header_s;
	parse_header(header_, header_s);

	for (params_type::iterator iter = header_s.begin();
		iter != header_s.end(); iter++)
	{
		header[ my::utf8::decode( percent_decode(iter->first) ) ]
			= my::utf8::decode( percent_decode(iter->second) );
	}
}

void message::read_body(tcp::socket &socket)
{
	/* Чтение здесь всегда заканчивается ошибкой! */
	boost::system::error_code ec;
	asio::read(socket, buf_, boost::asio::transfer_all(), ec);

	std::size_t n = buf_.size();
	body.resize(n);
	buf_.sgetn((char*)body.c_str(), n);
}

std::wstring message::content_type()
{
	std::wstring value = header[L"Content-Type"];

	value = value.substr(0, value.find_first_of(L';'));

	if (value.empty())
		value = L"text/plain"; /* Так требует RFC! */

	return value;
}

void message::to_xml(::xml::ptree &pt)
{
	std::istringstream ss(body);
	my::xml::parse(ss, pt);
}

void message::to_xml(::xml::wptree &pt)
{
	std::wistringstream ss( my::utf8::decode(body) );
	my::xml::parse(ss, pt);
}

void message::save(const std::wstring &filename)
{
	fs::create_directories( fs::wpath(filename).parent_path() );

	fs::ofstream fs(filename, std::ios::binary);
	if (fs)
		fs << body << std::flush;

	if (!fs)
		throw my::exception(L"Не удалось сохранить данные в файл")
			<< my::param(L"file", filename)
			<< my::param(L"error", strerror(errno));
}

void reply::read_reply(tcp::socket &socket)
{
	std::size_t n = asio::read_until(socket, buf_, "\r\n");

	reply_.resize(n);
	buf_.sgetn((char*)reply_.c_str(), n);

	std::string status_message_s;
	status_code = parse_reply(reply_, status_message_s);

	status_message = my::utf8::decode( percent_decode(status_message_s) );
}

void reply::get(tcp::socket &socket,
	const std::string &request, bool do_read_body)
{
	asio::write(socket, asio::buffer(request), asio::transfer_all());

	read_reply(socket);
	read_header(socket);

	if (do_read_body)
		read_body(socket);
}

void request::read_request(tcp::socket &socket)
{
	std::size_t n = asio::read_until(socket, buf_, "\r\n");

	request_.resize(n);
	buf_.sgetn((char*)request_.c_str(), n);

	std::string url_s;
	params_type params_s;

	parse_request(request_, url_s, params_s);

	url = my::utf8::decode( percent_decode(url_s) );

	for (params_type::iterator iter = params_s.begin();
		iter != params_s.end(); iter++)
	{
		params[ my::utf8::decode( percent_decode(iter->first) ) ]
			= my::utf8::decode( percent_decode(iter->second) );
	}
}

} }
