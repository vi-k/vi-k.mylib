#ifndef MY_THREAD_H
#define MY_THREAD_H

#include <boost/thread.hpp>


/* Регистрация потоков по имени */
namespace my
{
    unsigned int get_thread_id();
    void register_thread(const std::wstring &name);
    std::wstring& get_thread_name();
}


/* Блокировки */
using boost::unique_lock;
using boost::shared_lock;
using boost::upgrade_lock;
using boost::unique_lock;
using boost::upgrade_to_unique_lock;
typedef boost::condition_variable_any condition_variable;


/* Мьютексы */

#ifndef MY_LOCK_DEBUG

#define MY_MUTEX_DEF(var,log) var()
#define MY_MUTEX_DEFN(var,name,log) var()
#define MY_REGISTER_THREAD(name)

using boost::mutex;
using boost::shared_mutex;
using boost::recursive_mutex;

#else

#include "my_log.h"
#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp> /* boost::noncopyable */

#define MY_MUTEX_DEF(var,log) var(L ## #var,(log))
#define MY_MUTEX_DEFN(var,name,log) var((name),(log))
#define MY_REGISTER_THREAD(name) my::register_thread(name)

template<typename T>
class mutex_type
{
public:
	static const wchar_t* type()
	{
		static const wchar_t type[] = L"unknown mutex";
		return type;
	}
};

template<>
class mutex_type<boost::mutex>
{
public:
	static const wchar_t* type()
	{
		static const wchar_t type[] = L"mutex";
		return type;
	}
};

template<>
class mutex_type<boost::recursive_mutex>
{
public:
	static const wchar_t* type()
	{
		static const wchar_t type[] = L"recursive_mutex";
		return type;
	}
};

template<>
class mutex_type<boost::shared_mutex>
{
public:
	static const wchar_t* type()
	{
		static const wchar_t type[] = L"shared_mutex";
		return type;
	}
};

namespace my
{
	extern my::log locks_log;
}

template<typename Mutex>
class my_mutex : boost::noncopyable
{
private:
	Mutex m_;
	std::wstring name_;
	bool log_;

	void lock_debug(const wchar_t *func)
	{
		if (log_)
		{
			my::locks_log.to_header(L" class=");
			my::locks_log.to_header(mutex_type<Mutex>::type());

			if (name_.empty())
				my::locks_log << L"0x" << std::hex << (int)this;
			else
				my::locks_log << name_;

			my::locks_log << L'.' << func << my::locks_log;
		}
	}

public:
	my_mutex() : m_() {}
	my_mutex(const std::wstring &name, bool log)
		: name_(name), log_(log) {}

	~my_mutex() {}


	void lock()
	{
		lock_debug(L"lock()");
		m_.lock();
		lock_debug(L"lock()=ok");
	}

	bool try_lock()
	{
		lock_debug(L"try_lock()");
		bool res = m_.try_lock();
		lock_debug( res ? L"try_lock()=true" : L"try_lock()=false" );
		return res;
	}

	void unlock()
	{
		lock_debug(L"unlock()");
		m_.unlock();

		/* Если мьютексом владеет другой поток, то сразу после unlock()
			он может перестать существовать, поэтому здесь лучше
			больше ничего не делать. Собственно, и необходимости
			такой нет, т.к. unlock() должен срабатывать сразу  */
	}

	typedef boost::unique_lock< my_mutex<Mutex> > scoped_lock;
	typedef boost::detail::try_lock_wrapper< my_mutex<Mutex> > scoped_try_lock;
};

template<typename Mutex>
class my_shared_mutex : boost::noncopyable
{
private:
	Mutex m_;
	std::wstring name_;
	bool log_;

	void lock_debug(const wchar_t *func)
	{
		if (log_)
		{
			my::locks_log.to_header(L" class=");
			my::locks_log.to_header(mutex_type<Mutex>::type());

			if (name_.empty())
				my::locks_log << L"0x" << std::hex << (int)this;
			else
				my::locks_log << name_;

			my::locks_log << L'.' << func << my::locks_log;
		}
	}

public:
	my_shared_mutex() : m_() {}
	my_shared_mutex(const std::wstring &name, bool log)
		: name_(name), log_(log) {}

	~my_shared_mutex() {}

	void lock()
	{
		lock_debug(L"lock()");
		m_.lock();
		lock_debug(L"lock()=ok");
	}

	bool try_lock()
	{
		lock_debug(L"try_lock()");
		bool res = m_.try_lock();
		lock_debug( res ? L"try_lock()=true" : L"try_lock()=false" );
		return res;
	}

	void unlock()
	{
		lock_debug(L"unlock()");
		m_.unlock();

		/* Если мьютексом владеет другой поток, то сразу после unlock()
			он может перестать существовать, поэтому здесь лучше
			больше ничего не делать. Собственно, и необходимости
			такой нет, т.к. unlock() должен срабатывать сразу  */
	}

	void lock_shared()
	{
		lock_debug(L"lock_shared()");
		m_.lock_shared();
		lock_debug(L"lock_shared()=ok");
	}

	bool try_lock_shared()
	{
		lock_debug(L"try_lock_shared()");
		bool res = m_.try_lock_shared();
		lock_debug( res ? L"try_lock_shared()=true" : L"try_lock_shared()=false" );
		return res;
	}

	template<typename TimeDuration>
	bool timed_lock_shared(TimeDuration const& relative_time)
	{
		lock_debug(L"timed_lock_shared()");
		bool res = m_.timed_lock_shared(relative_time);
		lock_debug( res ? L"timed_lock_shared()=true" : L"timed_lock_shared()=false" );
		return res;
	}

	bool timed_lock_shared(boost::system_time const& wait_until)
	{
		lock_debug(L"timed_lock_shared()");
		bool res = m_.timed_lock_shared(wait_until);
		lock_debug( res ? L"timed_lock_shared()=true" : L"timed_lock_shared()=false" );
		return res;
	}

	void unlock_shared()
	{
		lock_debug(L"unlock_shared()");
		m_.unlock_shared();

		/* Если мьютексом владеет другой поток, то сразу после unlock()
			он может перестать существовать, поэтому здесь лучше
			больше ничего не делать. Собственно, и необходимости
			такой нет, т.к. unlock() должен срабатывать сразу  */
	}

	template<typename TimeDuration>
	bool timed_lock(TimeDuration const& relative_time)
	{
		lock_debug(L"timed_lock()");
		bool res = m_.timed_lock(relative_time);
		lock_debug( res ? L"timed_lock()=true" : L"timed_lock()=false" );
		return res;
	}

	typedef boost::unique_lock< my_shared_mutex<Mutex> > scoped_lock;
	typedef boost::detail::try_lock_wrapper< my_shared_mutex<Mutex> > scoped_try_lock;
};

typedef my_mutex<boost::mutex> mutex;
typedef my_mutex<boost::recursive_mutex> recursive_mutex;
typedef my_shared_mutex<boost::shared_mutex> shared_mutex;

#endif /* MY_LOCK_DEBUG */

#endif
