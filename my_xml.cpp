#include "my_xml.h"
#include "my_utf8.h"
#include "my_str.h"
#include "my_exception.h"

#include <locale>
#include <sstream>

#include <boost/config.hpp>
#include <boost/optional.hpp>
#include <boost/foreach.hpp>

#include <boost/archive/codecvt_null.hpp>

/* Прописать путь к boost! */
#include <libs/serialization/src/codecvt_null.cpp>

std::wstring xmlattr_to_str(const xml::wptree::value_type &v)
{
	std::wostringstream s;

	s << L"<" << v.first;

	boost::optional<const xml::wptree&> opt
		= v.second.get_child_optional(L"<xmlattr>");

	if (opt)
	{
		BOOST_FOREACH(const xml::wptree::value_type &v, *opt)
			/*TODO: не учитываю необходимость преобразования кавычек */
			s << L" " << v.first << L"=\"" << v.second.data() << L"\"";
	}

	if (v.second.size() == 0)
		s << L" />";
	else
		s << L"...</" << v.first << ">";

	return s.str();
}

std::wstring xmlattr_to_str(const xml::wptree &pt)
{
	std::wostringstream s;
	bool first = true;

	boost::optional<const xml::wptree&> opt
		= pt.get_child_optional(L"<xmlattr>");

	if (opt)
	{
		BOOST_FOREACH(const xml::wptree::value_type &v, *opt)
		{
			if (first)
				first = false;
			else
				s << L" ";

			/*TODO: не учитываю необходимость преобразования кавычек */
			s << v.first << L"=\"" << v.second.data() << L"\"";
		}
	}

	if ( pt.size() > (size_t)(opt ? 1 : 0) )
		s << L" ...";

	return s.str();
}

void my::xml::load(const std::wstring &filename, ::xml::wptree &pt)
{
	int charset;

	/* Чтение BOM - лучше способа не нашёл, чтобы считать BOM в char,
		а дальше заново открыть файл в wchar_t */
	{
		fs::ifstream fs(filename, ios_base::in | ios_base::binary);

		if (!fs)
			throw my::exception(L"Не удалось открыть файл")
				<< my::param(L"file", filename);

		unsigned char ch = fs.get();

		/* utf8 */
		if (ch == 0xEF && fs.get() == 0xBB && fs.get() == 0xBF)
			charset = 0;
		/* unicode */
		else if (ch == 0xFF && fs.get() == 0xFE)
			charset = 1;
		/* ansi */
		else
			charset = 2;
	}

	/* Открываем заново, пропускаем BOM */
	fs::wifstream fs(filename);

	switch (charset)
	{
		case 0:
			fs.seekg(3);
			fs.imbue( locale( fs.getloc(),
				new boost::archive::detail::utf8_codecvt_facet) );
			break;

		case 1:
			fs.seekg(2);
			fs.imbue( locale( fs.getloc(),
				new boost::archive::codecvt_null<wchar_t>) );
			break;

		case 2:
			fs.imbue( locale("") );
			break;
	}

	try
	{
		my::xml::parse(fs, pt);
	}
	catch(my::exception &e)
	{
		e.message(L"Ошибка разбора xml-файла");
		throw e << my::param(L"file", filename);
	}
}
