#ifndef MY_THREAD_H
#define MY_THREAD_H

#include <boost/thread.hpp>

/* Блокировки */
using boost::unique_lock;
using boost::shared_lock;
using boost::upgrade_lock;
using boost::unique_lock;
using boost::upgrade_to_unique_lock;
typedef boost::condition_variable_any condition_variable;


/* Мьютексы */

#ifdef MY_LOCK_NDEBUG

#define MY_MUTEX_DEF(var,log) var()
#define MY_MUTEX_DEFN(var,name,log) var()

using boost::mutex;
using boost::shared_mutex;
using boost::recursive_mutex;

#else

#include "my_debug.h"

#include <iostream>
#include <boost/unordered_map.hpp>
#include <boost/utility.hpp> /* boost::noncopyable */

#define MY_MUTEX_DEF(var,log) var(L ## #var,(log))
#define MY_MUTEX_DEFN(var,name,log) var((name),(log))

template<typename T>
class mutex_type
{
public:
	static const wchar_t* type()
	{
		static const wchar_t type[] = L"[unknown mutex]";
		return type;
	}
};

template<>
class mutex_type<boost::mutex>
{
public:
	static const wchar_t* type()
	{
		static const wchar_t type[] = L"[mutex]";
		return type;
	}
};

template<>
class mutex_type<boost::recursive_mutex>
{
public:
	static const wchar_t* type()
	{
		static const wchar_t type[] = L"[recursive_mutex]";
		return type;
	}
};

template<>
class mutex_type<boost::shared_mutex>
{
public:
	static const wchar_t* type()
	{
		static const wchar_t type[] = L"[shared_mutex]";
		return type;
	}
};


template<typename Mutex>
class my_mutex : boost::noncopyable
{
private:
	Mutex m_;
	std::wstring name_;
	bool log_;

public:
	my_mutex() : m_(), log_(false) {}

	my_mutex(const std::wstring &name, bool log)
		: name_(name), log_(log)
	{
		if (name_.empty())
		{
			std::wstringstream ss;
			ss << L"0x" << std::hex << (int)this;
			name_ = ss.str();
		}
	}

	~my_mutex() {}


	void lock()
	{
		my::scope sc(log_, name_ + L".lock()", mutex_type<Mutex>::type());
		m_.lock();
	}

	bool try_lock()
	{
		my::scope sc(log_, name_ + L".try_lock()", mutex_type<Mutex>::type());

		bool res = m_.try_lock();
		sc.add(res ? L"=true" : L"=false");

		return res;
	}

	void unlock()
	{
		my::scope sc(log_, name_ + L".unlock()", mutex_type<Mutex>::type());
		m_.unlock();
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

public:
	my_shared_mutex() : m_(), log_(false) {}

	my_shared_mutex(const std::wstring &name, bool log)
		: name_(name), log_(log)
	{
		if (name_.empty())
		{
			std::wstringstream ss;
			ss << L"0x" << std::hex << (int)this;
			name_ = ss.str();
		}
	}

	~my_shared_mutex() {}

	void lock()
	{
		my::scope sc(log_, name_ + L".lock()", mutex_type<Mutex>::type());
		m_.lock();
	}

	bool try_lock()
	{
		my::scope sc(log_, name_ + L".try_lock()", mutex_type<Mutex>::type());

		bool res = m_.try_lock();
		sc.add(res ? L"=true" : L"=false");

		return res;
	}

	void unlock()
	{
		my::scope sc(log_, name_ + L".UNlock()", mutex_type<Mutex>::type());
		m_.unlock();
	}

	void lock_shared()
	{
		my::scope sc(log_, name_ + L".lock_shared()", mutex_type<Mutex>::type());
		m_.lock_shared();
	}

	bool try_lock_shared()
	{
		my::scope sc(log_, name_ + L".try_lock_shared()", mutex_type<Mutex>::type());

		bool res = m_.try_lock_shared();
		sc.add(res ? L"=true" : L"=false");

		return res;
	}

	template<typename TimeDuration>
	bool timed_lock_shared(TimeDuration const& relative_time)
	{
		my::scope sc(log_, name_ + L".timed_lock_shared()", mutex_type<Mutex>::type());

		bool res = m_.timed_lock_shared(relative_time);
		sc.add(res ? L"=true" : L"=false");

		return res;
	}

	bool timed_lock_shared(boost::system_time const& wait_until)
	{
		my::scope sc(log_, name_ + L".timed_lock_shared()", mutex_type<Mutex>::type());

		bool res = m_.timed_lock_shared(wait_until);
		sc.add(res ? L"=true" : L"=false");

		return res;
	}

	void unlock_shared()
	{
		my::scope sc(log_, name_ + L".unlock_shared()", mutex_type<Mutex>::type());
		m_.unlock_shared();
	}

	template<typename TimeDuration>
	bool timed_lock(TimeDuration const& relative_time)
	{
		my::scope sc(log_, name_ + L".timed_lock()", mutex_type<Mutex>::type());

		bool res = m_.timed_lock(relative_time);
		sc.add(res ? L"=true" : L"=false");

		return res;
	}

	typedef boost::unique_lock< my_shared_mutex<Mutex> > scoped_lock;
	typedef boost::detail::try_lock_wrapper< my_shared_mutex<Mutex> > scoped_try_lock;
};

typedef my_mutex<boost::mutex> mutex;
typedef my_mutex<boost::recursive_mutex> recursive_mutex;
typedef my_shared_mutex<boost::shared_mutex> shared_mutex;

#endif /* #ifndef MY_LOCK_NDEBUG */

#endif
