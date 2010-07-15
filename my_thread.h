#ifndef MY_THREAD_H
#define MY_THREAD_H

#include <boost/thread/thread.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>

#include <boost/current_function.hpp>

#ifdef MY_LOCK_INSPECTOR
#include <string>
#include <map>
#include <boost/unordered_map.hpp>
#include "my_time.h"
#include "my_exception.h"
#endif

using boost::mutex;
using boost::shared_mutex;
using boost::recursive_mutex;

typedef boost::condition_variable_any condition_variable;

namespace my {

template<typename Mutex, typename Lock>
class locker_templ
{
private:
	Lock lock_;

    #ifdef MY_LOCK_INSPECTOR
	int id_;
	#endif

public:

    #ifndef MY_LOCK_INSPECTOR

    #define CURLINE()
    #define MYLOCKER_PARAMS(m,c,i) (m)

	locker_templ(Mutex &m)
		: lock_(m) {}

	~locker_templ() {}

	void lock()
	{
		lock_.lock();
	}

	void unlock()
	{
		lock_.unlock();
	}

	#else

	#define CURLINE_(F,L,Fu) "File: " F ", line: " #L ", func: " Fu
	#define CURLINE CURLINE_(__FILE__,__LINE__,BOOST_CURRENT_FUNCTION)
	#define MYLOCKER_PARAMS(m,c,i) (m,c,i)

	locker_templ(Mutex &m, int max_count, const std::string &info)
		: lock_(m, boost::defer_lock)
		, id_( my::lock_inspector::get_unique_id() )
	{
		g_lock_inspector.add(id_, max_count, info);
		lock();
	}

	~locker_templ()
	{
		unlock();
	}

	void lock()
	{
		g_lock_inspector.set_state(id_, my::lock_info::locking);
		lock_.lock();
		g_lock_inspector.set_state(id_, my::lock_info::locked);
	}

	void unlock()
	{
		g_lock_inspector.set_state(id_, my::lock_info::unlocking);
		lock_.unlock();
		g_lock_inspector.remove(id_);
	}
	#endif
};

typedef locker_templ<mutex, boost::unique_lock<mutex> > locker;
typedef locker_templ<recursive_mutex, boost::unique_lock<recursive_mutex> > recursive_locker;
typedef locker_templ<shared_mutex, boost::unique_lock<shared_mutex> > not_shared_locker;
typedef locker_templ<shared_mutex, boost::shared_lock<shared_mutex> > shared_locker;

#ifdef MY_LOCK_INSPECTOR

struct lock_info
{
    enum lock_state { locking, locked, unlocking };

	lock_state state;
	int count; /* Число проверок */
	int max_count;
	std::string info; /* __FILE__ , __LINE__, __FUNC__ */

	lock_info()
		: state(locking)
		, count(0)
		, max_count(0) {}

	lock_info(int max_count, const std::string &info)
		: state(locking)
		, count(0)
		, max_count(max_count)
		, info(info) {}
};

class lock_inspector
{
private:
	typedef std::map<int, lock_info> thread_locks;
	typedef std::map<boost::thread::id, thread_locks> locks;

	locks locks_;
	mutex mutex_;
	condition_variable sleep_cond_;
	boost::thread thread_;
	bool finish_;

public:
	lock_inspector()
        : finish_(false)
	{
		thread_ = boost::thread( boost::bind(
			&lock_inspector::inspector_proc, this) );
	}

	~lock_inspector()
	{
		finish_ = true;
		thread_.join();
	}

	void inspector_proc()
	{
		mutex::scoped_lock lock(mutex_);

		while (!finish_)
		{
			for (locks::iterator locks_iter = locks_.begin();
				locks_iter != locks_.end(); ++locks_iter)
			{
				thread_locks tl = locks_iter->second;

				for (thread_locks::iterator iter = tl.begin();
					iter != tl.end(); ++iter)
				{
					if (++iter->second.count >= iter->second.max_count)
				    	throw my::exception(L"Вышло время для блокировки")
				    		<< my::param(L"locker_id", iter->first)
				    		<< my::param(L"state", state_str(iter->second.state))
	    					<< my::param(L"info", iter->second.info.c_str())
	    					<< my::param(L"checked_count", iter->second.count)
	    					<< my::param(L"max_count", iter->second.max_count);
				}
			}

			sleep_cond_.timed_wait(lock, posix_time::seconds(1));
		}
	}

	static int get_unique_id()
	{
		static int id = 0;
		return ++id;
	}

	static std::wstring state_str(lock_info::lock_state state)
	{
		switch (state)
		{
			case lock_info::locking:
				return L"locking";

			case lock_info::locked:
				return L"locked";

			case lock_info::unlocking:
				return L"unlocking";

			default:
				return L"unknown";
		}
	}

	void add(int locker_id, int max_count, const std::string &info)
	{
	    mutex::scoped_lock l(mutex_);

	    thread_locks &tl = locks_[ boost::this_thread::get_id() ];

	    if (tl.find(locker_id) != tl.end())
	    	throw my::exception(L"Сбой в lock_inspector::add()"
	    		L" - такая блокировка уже существует")
	    		<< my::param(L"locker_id", locker_id)
	    		<< my::param(L"info", info.c_str());

		tl[locker_id] = lock_info(max_count, info);
	}

	void set_state(int locker_id, lock_info::lock_state state)
	{
	    mutex::scoped_lock l(mutex_);

	    thread_locks &tl = locks_[ boost::this_thread::get_id() ];

	    thread_locks::iterator iter = tl.find(locker_id);

	    if (iter == tl.end())
	    	throw my::exception(L"Сбой в lock_inspector::set_state()"
	    		L" - такой блокировки нет")
	    		<< my::param(L"locker_id", locker_id)
	    		<< my::param(L"state", state_str(state));

		iter->second.state = state;
	}

	void remove(int locker_id)
	{
	    mutex::scoped_lock l(mutex_);

	    thread_locks &tl = locks_[ boost::this_thread::get_id() ];

	    if (tl.find(locker_id) == tl.end())
	    	throw my::exception(L"Сбой в lock_inspector::remove()"
	    		L" - такой блокировки нет")
	    		<< my::param(L"locker_id", locker_id);

		tl.erase(locker_id);
	}
};

#endif

}

#ifdef MY_LOCK_INSPECTOR
extern my::lock_inspector g_lock_inspector;
#endif

#endif
