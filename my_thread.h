#ifndef MY_THREAD_H
#define MY_THREAD_H

#include <boost/thread.hpp>

using boost::unique_lock;
using boost::shared_lock;
using boost::upgrade_lock;
using boost::unique_lock;
using boost::upgrade_to_unique_lock;

typedef boost::condition_variable_any condition_variable;


#ifndef MY_LOCK_DEBUG

using boost::mutex;
using boost::shared_mutex;
using boost::recursive_mutex;

#else

#include "my_time.h"
#include <iostream>
#include <boost/utility.hpp> /* boost::noncopyable */

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
		static const char type[] = "boost::mutex";
		return type;
	}
};

template<>
class mutex_type<boost::recursive_mutex>
{
public:
	static const char * type()
	{
		static const char type[] = "boost::recursive_mutex";
		return type;
	}
};

template<>
class mutex_type<boost::shared_mutex>
{
public:
	inline static const char * type()
	{
		static const char type[] = "boost::shared_mutex";
		return type;
	}
};


template<typename Mutex>
class my_mutex : boost::noncopyable
{
private:
	Mutex m_;

	void lock_debug(const char *func)
	{
		std::stringstream out;

		out << my::time::to_string( my::time::local_now(), "[%Y-%m-%d %H:%M:%S%f] " )
			<< std::hex << (int)this
			<< ' ' << func
			<< " type: " << mutex_type<Mutex>::type()
			<< " thread: " << boost::this_thread::get_id()
			<< std::endl;

		std::cout << out.str() << std::flush;
	}

public:
	my_mutex() : m_() {}
	~my_mutex() {}


	void lock()
	{
		lock_debug("lock()");
		m_.lock();
		lock_debug("  lock()=ok");
	}

	bool try_lock()
	{
		lock_debug("try_lock()");
		bool res = m_.try_lock();
		lock_debug( res ? "  try_lock()=true" : "  try_lock()=false" );
		return res;
	}

	void unlock()
	{
		lock_debug("unlock()");
		m_.unlock();
		lock_debug("  unlock()=ok");
	}

	typedef boost::unique_lock< my_mutex<Mutex> > scoped_lock;
	typedef boost::detail::try_lock_wrapper< my_mutex<Mutex> > scoped_try_lock;
};

typedef my_mutex<boost::mutex> mutex;
typedef my_mutex<boost::recursive_mutex> recursive_mutex;

template<typename Mutex>
class my_shared_mutex : boost::noncopyable
{
private:
	Mutex m_;

	void lock_debug(const char *func)
	{
		std::stringstream out;

		out << my::time::to_string( my::time::local_now(), "[%Y-%m-%d %H:%M:%S%f] " )
			<< std::hex << (int)this
			<< ' ' << func
			<< " type: " << mutex_type<Mutex>::type()
			<< " thread: " << boost::this_thread::get_id()
			<< std::endl;

		std::cout << out.str() << std::flush;
	}

public:
	my_shared_mutex() : m_() {}
	~my_shared_mutex() {}

	void lock_shared()
	{
		lock_debug("lock_shared()");
		m_.lock_shared();
		lock_debug("  lock_shared()=ok");
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
		lock_debug("  unlock_shared()=ok");
	}

	void lock()
	{
		lock_debug("lock()");
		m_.lock();
		lock_debug("  lock()=ok");
	}

	bool try_lock()
	{
		lock_debug("try_lock()");
		bool res = m_.try_lock();
		lock_debug( res ? "  try_lock()=true" : "  try_lock()=false" );
		return res;
	}

	template<typename TimeDuration>
	bool timed_lock(TimeDuration const& relative_time)
	{
		lock_debug("timed_lock()");
		bool res = m_.timed_lock(relative_time);
		lock_debug( res ? "timed_lock()=true" : "timed_lock()=false" );
		return res;
	}

	void unlock()
	{
		lock_debug("unlock()");
		m_.unlock();
		lock_debug("  unlock()=ok");
	}

    typedef boost::unique_lock< my_mutex<Mutex> > scoped_lock;
    typedef boost::detail::try_lock_wrapper< my_mutex<Mutex> > scoped_try_lock;
};

typedef my_shared_mutex<boost::shared_mutex> shared_mutex;

#endif

#endif
