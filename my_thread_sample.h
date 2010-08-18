#ifndef MY_THREAD_H
#define MY_THREAD_H

#include <boost/thread.hpp>

#include <boost/utility.hpp> /* boost::noncopyable */

#ifndef MY_LOCK_DEBUG

using boost::mutex;
using boost::shared_mutex;
using boost::recursive_mutex;
typedef boost::condition_variable_any condition_variable;

using boost::unique_lock;
using boost::shared_lock;
using boost::upgrade_lock;
using boost::unique_lock;
using boost::upgrade_to_unique_lock;

#else

template<typename Mutex>
class my_mutex : boost::noncopyable
{
private:
	Mutex m_;

public:
	my_mutex() : m_() {}
	~my_mutex() {}

	void lock()
	{
		m_.lock();
	}

    bool try_lock()
    {
    	return m_.try_lock();
    }

    void unlock()
    {
    	m_.unlock();
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

public:
	my_shared_mutex() : m_() {}
	~my_shared_mutex() {}

	void lock_shared()
	{
		m_.lock_shared();
	}

	bool try_lock_shared()
	{
		return m_.try_lock_shared();
	}

	template<typename TimeDuration>
	bool timed_lock_shared(TimeDuration const& relative_time)
	{
		return m_.timed_lock_shared(relative_time);
	}

	bool timed_lock_shared(boost::system_time const& wait_until)
	{
		return m_.timed_lock_shared(wait_until);
	}

	void unlock_shared()
	{
		m_.unlock_shared();
	}

	void lock()
	{
		m_.lock();
	}

    bool try_lock()
    {
    	return m_.try_lock();
    }

	template<typename TimeDuration>
	bool timed_lock(TimeDuration const& relative_time)
	{
		return m_.timed_lock(relative_time);
	}

    void unlock()
    {
    	m_.unlock();
    }

    typedef boost::unique_lock< my_mutex<Mutex> > scoped_lock;
    typedef boost::detail::try_lock_wrapper< my_mutex<Mutex> > scoped_try_lock;
};

typedef my_shared_mutex<boost::shared_mutex> shared_mutex;

#endif

using boost::shared_lock;
using boost::unique_lock;

typedef boost::condition_variable_any condition_variable;
using boost::condition_variable_any;

#endif
