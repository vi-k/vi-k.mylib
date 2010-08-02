#ifndef MY_THREAD_H
#define MY_THREAD_H

#include <boost/thread.hpp>

using boost::mutex;
using boost::shared_mutex;
using boost::recursive_mutex;
typedef boost::condition_variable_any condition_variable;

using boost::unique_lock;
using boost::shared_lock;
using boost::upgrade_lock;
using boost::unique_lock;
using boost::upgrade_to_unique_lock;

#if 0

#ifdef MY_LOCK_INSPECTOR
#include <string>
#include <map>
#include <boost/unordered_map.hpp>
#include "my_time.h"
#include "my_exception.h"
#endif

namespace my {

#ifndef MY_LOCK_INSPECTOR

#define MYLOCKERPARAMS(m,c,i) m
#define MYLOCKINSPECTOR_INIT()

typedef mutex::scoped_lock locker;
typedef recursive_mutex::scoped_lock recursive_locker;
typedef boost::unique_lock<shared_mutex> not_shared_locker;
typedef boost::shared_lock<shared_mutex> shared_locker;
typedef boost::upgrade_lock<shared_mutex> upgrade_locker;

/*-
template<typename Mutex, typename Lock>
class locker_templ
{
private:
	Lock lock_;

public:

	locker_templ(Mutex &m)
		: lock_(m) {}

	~locker_templ() {}

	inline void lock()
		{ lock_.lock(); }

	inline void unlock()
		{ lock_.unlock(); }
};
-*/

#else /* #ifndef MY_LOCK_INSPECTOR */

#define MYLOCKERPARAMS(m,c,i) m,c,i
#define MYLOCKINSPECTOR_INIT() my::lock_inspector my::g_lock_inspector;

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

	std::ofstream log_;

	static int get_unique_id()
	{
		static int id = 0;
		return ++id;
	}

public:
	lock_inspector()
        : finish_(false)
        , log_("lock.log")
	{
		thread_ = boost::thread( boost::bind(
			&lock_inspector::inspector_proc, this) );
	}

	~lock_inspector()
	{
		finish_ = true;
		thread_.join();
	}

	inline std::ofstream& log()
		{ return log_; }

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

	int add(int max_count, const std::string &info)
	{
	    mutex::scoped_lock l(mutex_);

	    int locker_id = get_unique_id();

	    thread_locks &tl = locks_[ boost::this_thread::get_id() ];

	    if (tl.find(locker_id) != tl.end())
	    	throw my::exception(L"Сбой в lock_inspector::add()"
	    		L" - такая блокировка уже существует")
	    		<< my::param(L"locker_id", locker_id)
	    		<< my::param(L"info", info.c_str());

		tl[locker_id] = lock_info(max_count, info);

		return locker_id;
	}

	void set_state(int locker_id, lock_info::lock_state state)
	{
	    /*-
	    mutex::scoped_lock l(mutex_);

	    thread_locks &tl = locks_[ boost::this_thread::get_id() ];

	    thread_locks::iterator iter = tl.find(locker_id);

	    if (iter == tl.end())
	    	throw my::exception(L"Сбой в lock_inspector::set_state()"
	    		L" - такой блокировки нет")
	    		<< my::param(L"locker_id", locker_id)
	    		<< my::param(L"state", state_str(state));

		iter->second.state = state;
        -*/
	}

	void remove(int locker_id)
	{
	    /*-
	    mutex::scoped_lock l(mutex_);

	    thread_locks &tl = locks_[ boost::this_thread::get_id() ];

	    if (tl.find(locker_id) == tl.end())
	    	throw my::exception(L"Сбой в lock_inspector::remove()"
	    		L" - такой блокировки нет")
	    		<< my::param(L"locker_id", locker_id);

		tl.erase(locker_id);
        -*/
	}
};

extern lock_inspector g_lock_inspector;

template<typename Mutex, typename Lock>
class locker_templ
{
private:
	Lock lock_;
	int id_;

public:

	locker_templ(Mutex &m, int max_count, const std::string &info)
		: lock_(m)
		//: lock_(m, boost::defer_lock)
	{
		id_ = g_lock_inspector.add(max_count, info);
		g_lock_inspector.log() << "#" << id_ << " locked " << info << "\n" << std::flush;
		//lock();
	}

	~locker_templ()
	{
		//unlock();
		g_lock_inspector.log() << "#" << id_ << " unlocking\n" << std::flush;
	}

	void lock()
	{
		//g_lock_inspector.set_state(id_, my::lock_info::locking);
		g_lock_inspector.log() << "#" << id_ << " locking\n" << std::flush;
		lock_.lock();
		g_lock_inspector.log() << "#" << id_ << " locked\n" << std::flush;
		//g_lock_inspector.set_state(id_, my::lock_info::locked);
	}

	void unlock()
	{
		//g_lock_inspector.set_state(id_, my::lock_info::unlocking);
		g_lock_inspector.log() << "#" << id_ << " unlocking\n" << std::flush;
		lock_.unlock();
		g_lock_inspector.log() << "#" << id_ << " unlocked\n" << std::flush;
		//g_lock_inspector.remove(id_);
	}
};

typedef locker_templ<mutex, boost::unique_lock<mutex> > locker;
typedef locker_templ<recursive_mutex, boost::unique_lock<recursive_mutex> > recursive_locker;
typedef locker_templ<shared_mutex, boost::unique_lock<shared_mutex> > not_shared_locker;
typedef locker_templ<shared_mutex, boost::shared_lock<shared_mutex> > shared_locker;

#endif /* #else | #ifndef MY_LOCK_INSPECTOR */

}

#endif

#endif
