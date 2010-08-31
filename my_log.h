#include "my_thread.h" /* Обязательно здесь! */
#include "my_debug.h"

#ifndef MY_LOG_H
#define MY_LOG_H

#include "my_time.h" /* my::time::to_wstring */
#include "my_str.h" /* my::str::to_string, my::str::escape */
#include "my_utf8.h" /* boost::...::utf8_codecvt_facet */
#include "my_fs.h" /* boost::filesystem */

#include <cstddef> /* std::size_t */
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
	enum {clean=1, full=0, nolineheader=2, simple=2, singleline=4, single=4,
		nothread=8, nothreadid=16};

private:
	std::wostream &out_;
	std::wofstream fs_;
	const int flags_;
	const std::wstring time_format_;
	boost::recursive_mutex rmutex_;
	std::wstringstream buf_;
	int state_;

	void print_header()
	{
		std::wstringstream ss;
		ss << L"Log opened: " << my::time::to_wstring(
			my::time::local_now(), time_format_.c_str());

		std::size_t sz = ss.str().size();
		buf_ << std::wstring(sz, L'=') << std::endl
			<< ss.str() << std::endl
			<< std::wstring(sz, L'-') << std::endl;

		if (flags_ & singleline)
			buf_ << std::endl;

		out_ << buf_.str() << std::flush;
		buf_.str(L"");
	}

	void print_footer()
	{
		std::wstringstream ss;
		ss << L"Log closed: " << my::time::to_wstring(
			my::time::local_now(), time_format_.c_str());

		std::size_t sz = ss.str().size();
		buf_ << std::endl << std::wstring(sz, L'-') << std::endl
			<< ss.str() << std::endl
			<< std::wstring(sz, L'=') << std::endl;

		out_ << buf_.str() << std::flush;
		buf_.str(L"");
	}

	void change_state(int st)
	{
		while (state_ != st)
		{
			switch (state_)
			{
				case 0:
					/* Выводим начало строки (дата и название потока),
						блокируем вывод в лог другим потокам */

					rmutex_.lock();

					if ( !(flags_ & nolineheader) )
					{
						if ( !(flags_ & singleline) )
							buf_ << std::endl;

						buf_ << my::time::to_wstring(
							my::time::local_now(), time_format_.c_str());

						if ( !(flags_ & nothread) )
						{
							buf_ << L" thread=" << my::get_thread_name();
							if ( !(flags_ & nothreadid) )
								buf_ << L" (" << boost::this_thread::get_id() << L')';
						}
					}

					state_ = 1;
					break;

				case 1:
					if ( !(flags_ & nolineheader) )
					{
						if (flags_ & singleline)
							buf_ << L": ";
						else
							buf_ << std::endl;
					}

					state_ = 2;
					break;

				case 2:
					buf_ << std::endl;
					out_ << buf_.str() << std::flush;
					buf_.str(L"");

					state_ = 0;
					rmutex_.unlock();
					break;

			} /* switch (state_) */

		} /* while (state_ != st) */
	}

public:
	/* Лог в std::wcout, std::wcerr и т.п. */
	log(std::wostream &out, int flags = 0,
		const std::wstring &time_format = L"%Y-%m-%d %H:%M:%S%f")
		: out_(out)
		, flags_(flags)
		, time_format_(time_format)
		, state_(0)
	{
		print_header();
	}

	/* Лог в файл в utf8 */
	log(const std::wstring &filename, int flags = 0,
		const std::wstring &time_format = L"%Y-%m-%d %H:%M:%S%f")
		: out_(fs_)
		, flags_(flags)
		, time_format_(time_format)
		, state_(0)
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
			fs_ << std::endl << std::endl;

		print_header();
	}

	~log()
	{
		print_footer();
	}

	void to_header(const std::wstring &text)
	{
		unique_lock<boost::recursive_mutex> l(rmutex_);

		change_state(1);

		if ( !(flags_ & nolineheader) )
			buf_ << text;
	}

	void operator <<(const log& x)
	{
		unique_lock<boost::recursive_mutex> l(rmutex_);
		change_state(0);
	}

	template<class T>
	log& operator <<(const T& x)
	{
		unique_lock<boost::recursive_mutex> l(rmutex_);

		change_state(2);

		if ( !(flags_ & singleline) )
			buf_ << x;
		else
		{
			std::wstringstream ss;
			ss << x;
			buf_ << my::str::escape(ss.str(), my::str::escape_cntrl_only);
		}

		return *this;
	}
};

class null_log
{
public:
	null_log() {}
	~null_log() {}

	void to_header(const std::wstring &text) {}

	void operator <<(const null_log& x) {}

	template<class T>
	null_log& operator <<(const T& x)
		{ return *this; }
};

class custom_log
{
public:
	typedef boost::function<void (const std::wstring &text)> on_log_proc;

private:
	on_log_proc on_log_;
	std::wostringstream out_;
	bool begin_;
	boost::recursive_mutex rmutex_;

public:
	custom_log(on_log_proc on_log)
		: on_log_(on_log)
		, begin_(true) {}

	void operator <<(const custom_log& x)
	{
		std::wstring text = out_.str();
		if (on_log_ && !text.empty())
			on_log_(text);
		out_.str(L"");

		if (!begin_)
		{
			begin_ = true;
			rmutex_.unlock();
		}

	}

	template<class T>
	custom_log& operator <<(const T& x)
	{
		unique_lock<boost::recursive_mutex> l(rmutex_);

		if (begin_)
		{
			rmutex_.lock();
			begin_ = false;
		}

		out_ << x;
		return *this;
	}
};

}

#endif
