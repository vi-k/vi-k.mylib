#include "my_thread.h" /* Обязательно здесь! */

#ifndef MY_DEBUG_H
#define MY_DEBUG_H

#include "my_stopwatch.h"

#include <string>

namespace my
{

/* my::stopwatch */
#ifdef MY_STOPWATCH_NDEBUG
	#define MY_STOPWATCH(t)
	#define MY_STOPWATCH_START(t)
	#define MY_STOPWATCH_FINISH(t)
	#define MY_STOPWATCH_OUT(out,t)
	#define IF_MY_STOPWATCH(c)
#else
	#define MY_STOPWATCH(t) my::stopwatch t;
	#define MY_STOPWATCH_START(t) (t).start();
	#define MY_STOPWATCH_FINISH(t) (t).finish();
	#define MY_STOPWATCH_OUT(out,t) out << t;
	#define IF_MY_STOPWATCH(c) c
#endif

/* Регистрация потоков */
unsigned int get_thread_id();
void register_thread(const std::wstring &name);
std::wstring& get_thread_name();

#ifdef MY_THREAD_NDEBUG
	#define MY_REGISTER_THREAD(name)
#else
	#define MY_REGISTER_THREAD(name) my::register_thread(name)
#endif

} /* namespace my*/

#ifndef MY_SCOPE_NDEBUG

#include "my_log.h" /* Обязательно после функций регистрации потоков! */
extern my::log main_log;

namespace my
{

/* Регистрация scope-блоков */
class scope
{
private:
	std::wstring text_;

public:
	scope() {}

	scope(const std::wstring &scope)
	{
		main_log << scope << main_log;
		text_ = L'~' + scope;
	}

	scope(const std::wstring &scope, const std::wstring &unit)
	{
		main_log << unit << L' ' << scope << main_log;
		text_ = unit + L" ~" + scope;
	}

	scope(bool log, const std::wstring &scope, const std::wstring &unit)
	{
		if (log)
		{
			main_log << unit << L' ' << scope << main_log;
			text_ = unit + L" ~" + scope;
		}
	}

	~scope()
	{
		if (!text_.empty())
			main_log << text_ << main_log;
	}

	void add(const std::wstring &str)
	{
		if (!text_.empty())
			text_ += str;
	}
};

#endif /* #ifndef MY_SCOPE_NDEBUG */

} /* namespace my*/

#endif
