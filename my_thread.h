#ifndef MY_THREAD_H
#define MY_THREAD_H

#include <boost/thread/thread.hpp>

#include <boost/thread/mutex.hpp>
#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/recursive_mutex.hpp>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/locks.hpp>

using boost::mutex;
using boost::shared_mutex;
using boost::recursive_mutex;

using boost::shared_lock;
using boost::unique_lock;

using boost::condition_variable;
using boost::condition_variable_any;

/* Остальное включено для редких случаев, в основном для тестирования */

#endif
