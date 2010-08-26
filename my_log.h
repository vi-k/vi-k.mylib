#ifndef MY_LOG_H
#define MY_LOG_H

#include "my_time.h"
#include "my_str.h"
#include "my_utf8.h"
#include "my_thread.h"

#include <string>
#include <sstream>
#include <fstream>

#include <boost/function.hpp>
#include <boost/format.hpp>
#include <boost/format/format_fwd.hpp>

namespace my
{

class log
{
public:
	enum {clean = 1, multiline = 2};

private:
	std::wostream &out_;
	std::wofstream fs_;
	int flags_;
	std::wstring time_format_;
	bool begin_;
	boost::recursive_mutex rmutex_;

public:
	/* Лог в std::wcout, std::wcerr и т.п. */
	log(std::wostream &out, int flags = 0,
		const std::wstring &time_format = L"%Y-%m-%d %H:%M:%S%f")
		: out_(out)
		, flags_(flags)
		, time_format_(time_format)
		, begin_(true)
	{
	}

	/* Лог в файл в utf8 */
	log(const std::wstring &filename, int flags = 0,
		const std::wstring &time_format = L"%Y-%m-%d %H:%M:%S%f")
		: out_(fs_)
		, flags_(flags)
		, time_format_(time_format)
		, begin_(true)
	{
		#if defined(_MSC_VER)
		const std::wstring &fname = filename;
		#else
		std::string fname = my::str::to_string(filename);
		#endif

		bool exists = (flags & clean) ? false : fs::exists(fname);

		if (!exists)
		{
			std::ofstream fs(fname.c_str());
			fs << "\xEF\xBB\xBF"; /* BOM */
			fs.close();
		}

		fs_.open(fname.c_str(), std::ios::app);
		fs_.imbue( std::locale( fs_.getloc(),
			new boost::archive::detail::utf8_codecvt_facet) );

		if (exists)
			fs_ << std::endl;
	}

	void operator <<(const log& x)
	{
		if (flags_ & multiline)
			out_ << L"\n\n";
		else
			out_ << L" [thread="
				<< my::str::to_wstring( my::get_thread_name() )
				<< L"]\n";

		out_ << std::flush;

		begin_ = true;

		rmutex_.unlock();
	}

	template<class T>
	log& operator <<(const T& x)
	{
		unique_lock<boost::recursive_mutex> l(rmutex_);

		if (begin_)
		{
			rmutex_.lock();
			begin_ = false;

			out_ << L'[' << my::time::to_wstring(
				my::time::local_now(), time_format_.c_str())
				<< L" thread=" << boost::this_thread::get_id();

			if ( !(flags_ & multiline) )
				out_ << L"] ";
			else
				out_ << L" ("
					<< my::str::to_wstring( my::get_thread_name() )
					<< L")]\n";
		}

		out_ << x;
		return *this;
	}
};

class custom_log
{
public:
	typedef boost::function<void (const std::wstring &text)> on_log_proc;

private:
	on_log_proc on_log_;
	std::wostringstream out_;

public:
	custom_log(on_log_proc on_log)
		: on_log_(on_log) {}

	void operator <<(const custom_log& x)
	{
		flush();
	}

	template<class T>
	custom_log& operator <<(const T& x)
	{
		out_ << x;
		return *this;
	}

	void flush()
	{
		std::wstring text = out_.str();
		if (on_log_ && !text.empty())
			on_log_(text);
		out_.str(L"");
	}
};

}

#endif
