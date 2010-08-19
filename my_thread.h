#ifndef MY_THREAD_H
#define MY_THREAD_H

#include <boost/thread.hpp>


/* Регистрация потоков по имени */
namespace my
{
    unsigned int get_thread_id();
    void register_thread(const std::string &name);
    std::string& get_thread_name();
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

#include "my_time.h"
#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp> /* boost::noncopyable */

#define MY_MUTEX_DEF(var,log) var(#var,(log))
#define MY_MUTEX_DEFN(var,name,log) var((name),(log))
#define MY_REGISTER_THREAD(name) my::register_thread(name)

template<typename T>
class mutex_type
{
public:
	static const char * type()
	{
		static const char type[] = "unknown mutex";
		return type;
	}
};

template<>
class mutex_type<boost::mutex>
{
public:
	static const char * type()
	{
		static const char type[] = "mutex";
		return type;
	}
};

template<>
class mutex_type<boost::recursive_mutex>
{
public:
	static const char * type()
	{
		static const char type[] = "recursive_mutex";
		return type;
	}
};

template<>
class mutex_type<boost::shared_mutex>
{
public:
	inline static const char * type()
	{
		static const char type[] = "shared_mutex";
		return type;
	}
};

template<typename Mutex>
class my_mutex : boost::noncopyable
{
private:
	Mutex m_;
	std::string name_;
	bool log_;

	void lock_debug(const char *func)
	{
		if (log_)
		{
			std::stringstream buf;

			buf << my::time::to_string( my::time::local_now(), "[%Y-%m-%d %H:%M:%S%f] " );

			if (name_.empty())
				buf << std::hex << (int)this;
			else
				buf << name_;

			buf << '.' << func
				<< " [" << mutex_type<Mutex>::type()
				<< " thread=" << my::get_thread_name()
				<< "]\n";

			std::cout << buf.str() << std::flush;
		}
	}

public:
	my_mutex() : m_() {}
	my_mutex(const std::string &name, bool log)
		: name_(name), log_(log) {}

	~my_mutex() {}


	void lock()
	{
		lock_debug("lock()");
		m_.lock();
		lock_debug("lock()=ok");
	}

	bool try_lock()
	{
		lock_debug("try_lock()");
		bool res = m_.try_lock();
		lock_debug( res ? "try_lock()=true" : "try_lock()=false" );
		return res;
	}

	void unlock()
	{
		lock_debug("unlock()");
		m_.unlock();
		lock_debug("unlock()=ok");
	}

	typedef boost::unique_lock< my_mutex<Mutex> > scoped_lock;
	typedef boost::detail::try_lock_wrapper< my_mutex<Mutex> > scoped_try_lock;
};

template<typename Mutex>
class my_shared_mutex : boost::noncopyable
{
private:
	Mutex m_;
	std::string name_;
	bool log_;

	void lock_debug(const char *func)
	{
		if (log_)
		{
			std::stringstream buf;

			buf << my::time::to_string( my::time::local_now(), "[%Y-%m-%d %H:%M:%S%f] " );

			if (name_.empty())
				buf << std::hex << (int)this;
			else
				buf << name_;

			buf << '.' << func
				<< " [" << mutex_type<Mutex>::type()
				<< " thread=" << my::get_thread_name()
				<< "]\n";

			std::cout << buf.str() << std::flush;
		}
	}

public:
	my_shared_mutex() : m_() {}
	my_shared_mutex(const std::string &name, bool log)
		: name_(name), log_(log) {}

	~my_shared_mutex() {}

	void lock()
	{
		lock_debug("lock()");
		m_.lock();
		lock_debug("lock()=ok");
	}

	bool try_lock()
	{
		lock_debug("try_lock()");
		bool res = m_.try_lock();
		lock_debug( res ? "try_lock()=true" : "try_lock()=false" );
		return res;
	}

	void unlock()
	{
		lock_debug("unlock()");
		m_.unlock();
		lock_debug("unlock()=ok");
	}

	void lock_shared()
	{
		lock_debug("lock_shared()");
		m_.lock_shared();
		lock_debug("lock_shared()=ok");
	}

	bool try_lock_shared()
	{
		lock_debug("try_lock_shared()");
		bool res = m_.try_lock_shared();
		lock_debug( res ? "try_lock_shared()=true" : "try_lock_shared()=false" );
		return res;
	}

	template<typename TimeDuration>
	bool timed_lock_shared(TimeDuration const& relative_time)
	{
		lock_debug("timed_lock_shared()");
		bool res = m_.timed_lock_shared(relative_time);
		lock_debug( res ? "timed_lock_shared()=true" : "timed_lock_shared()=false" );
		return res;
	}

	bool timed_lock_shared(boost::system_time const& wait_until)
	{
		lock_debug("timed_lock_shared()");
		bool res = m_.timed_lock_shared(wait_until);
		lock_debug( res ? "timed_lock_shared()=true" : "timed_lock_shared()=false" );
		return res;
	}

	void unlock_shared()
	{
		lock_debug("unlock_shared()");
		m_.unlock_shared();
		lock_debug("unlock_shared()=ok");
	}

	template<typename TimeDuration>
	bool timed_lock(TimeDuration const& relative_time)
	{
		lock_debug("timed_lock()");
		bool res = m_.timed_lock(relative_time);
		lock_debug( res ? "timed_lock()=true" : "timed_lock()=false" );
		return res;
	}

	typedef boost::unique_lock< my_shared_mutex<Mutex> > scoped_lock;
	typedef boost::detail::try_lock_wrapper< my_shared_mutex<Mutex> > scoped_try_lock;
};

typedef my_mutex<boost::mutex> mutex;
typedef my_mutex<boost::recursive_mutex> recursive_mutex;
typedef my_shared_mutex<boost::shared_mutex> shared_mutex;

#endif

#endif
