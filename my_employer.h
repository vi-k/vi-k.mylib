/*
	Класс employer - "работодатель".

	Цель: обеспечение контроля и безопасного завершения
		деятельности "работников" (worker'ов) класса.

	Описание:

	Под "работниками" подразумеваются потоки, асинхронные операции, таймеры
	и т.п., т.е. самостоятельно живущие программные конструкции. При
	проектировании класса с такими "работниками" есть определённая трудность
	обеспечить нормальное завершение работы и дождаться, когда они
	действительно прекратят свою работу.

	Далее по тексту	"работников" буду называть worker'ами.


	* Уведомление о завершении работы *

	Обычно уведомление worker'ов происходит через установку флага, который,
	затем, worker проверит и завершит свою работу (выйдет из функции
	потока, остановит (или не перезапустит) таймер, прервёт (или не
	возобновит) асинхронную операцию). В employer установка флага
	обеспечивается функцией lets_finish ("пора заканчивать"). Проверка флага
	- функцией finish (if (finish()) .., while (!finish()) ..).


	* Ожидание завершения работы *

	Пока worker получит уведомление и завершит работу, объект нашего
	класса не должен быть разрушен, поэтому ни в коем случае нельзя выйти
	из деструктора, пока не убедимся, что все worker'ы закончили работу
	и больше не используют переменные/структуры/объекты нашего класса.
	В employer для этого существует функция wait_for_finish(). Её суть
	- каждый worker, при своей инициализации создаёт блокировку
	shared_lock общего мьютекса. employer же при завершении просто пытается
	установить unique_lock на этот мьютекс. И, следовательно, пока все
	worker'ы не перестанут существовать (и, тем самым, не освободят
	мьютекс), employer будет спать.


	* Создание worker'а *

	Создаётся worker функцией new_worker(). Функция возвращает "умный"
	указатель (shared_ptr) на worker'а (worker::ptr), и это означает, что
	для employer worker жив пока жив хотя бы один указатель на него
	(employer тоже является владельцем такого указателя, поэтому
	в wait_for_finish() перед блокировкой мьютекса он удаляет свои копии
	указателей). Целью разработчика является сохранить этот указатель
	на протяжении жизни потока или асинхронной операции. Лучшим способом,
	IMHO, является передача указателя в функцию потока или обработчик
	асинхронной операции через boost::bind:

	Поток:

		Создание потока:

		boost::thread( boost::bind( &my_class::thread_proc,
			this, new_worker("my_thread")) );

		Функция потока:

		void my_class::thread_proc(my::worker::ptr this_worker) {}

	Асинхронная операция:

		Запуск операции:

		boost::asio::async_read_until( socket, buf, "\r\n",
			boost::bind(&my_class::handle_read, this, new_worker("async_read")) );

		Обработчик операции:

		void my_class::handle_read(my::worker::ptr this_worker);

		Перезапуск операции внутри обработчика:

		boost::asio::async_read_until( socket, buf, "\r\n",
			boost::bind(&my_class::handle_read, this, this_worker) );


	Но, в принципе, все эти вещи достаточно легко реализуются простыми
	средствами, и если бы только это было нужно, то создавать employer
	было бы нецелесообразно. Но есть более интересные вещи.


	* Интересные вещи *


	* Спящие потоки *

	Некоторым потокам нет необходимости постоянно работать. Т.е. они
	действуют по принципу - сделал дело и уснул, пока не разбудят.
	В employer для этого есть функции sleep() и wake_up().

	sleep(this_worker) - усыпляет текущий (!) поток. this_worker, в данном
	случае, обеспечиват лишь хранение condition и обеспечивает уверенность,
	что поток не заснёт, когда установлен флаг finish().

	wake_up(worker) - будит поток, если он спит. Особенность функции в том,
	что для её выполнения необходимо где-то в нашем классе хранить копию
	указателя на worker'а. Но раз будет сохранён указатель, то
	wait_for_finish() никогда не завершится, пока вы самостоятельно
	не освободите указатель или, в терминах employer, пока не уволите
	worker'а функцией dismiss() (освободится только хранимый вами указатель,
	сам worker пока ещё будет жить!).

	Функция lets_finish() помимо установки флага завершения, автоматически
	разбудит все уснувшие потоки.

	Таким образом для спящих потоков деструктор класса должен выглядеть
	следующим образом:

		lets_finish(); // Оповестить о завершении, разбудить спящих
		dismiss(some_sleeping_thread); // Особождаем хранимые нами указатели
		dismiss(another_sleeping_thread);
		...
		wait_for_finish(); // Ждём завершения


	* Windows Messages и dead_lock'и *

	Особенностью при разработке является завершение работы потока, так или
	иначе, связанного с отправкой оконных сообщений.

	Примерный вариант развития событий: завершение работы нашего класса
	происходит из основного потока приложения (к примеру, через закрытие
	пользователем окна). В классе работает поток, отсылающий сообщения
	через SendMessage (к примеру, устанавливающий текст в некое текстовое
	поле или меняющий заголовок окна - в конечном итоге, в WinAPI и то,
	и другое выполняется через функцию SetWindowText(), которая, в свою
	очередь вызывает SendMessage). Особенность SendMessage в том, что она
	дожидается, когда сообщение будет получено. Но сообщения обрабатываются
	основным потоком, который в это время спит в ожидании завершения нашего
	потока (к примеру, через thread.join() или нашу wait_for_finish()). Таким
	образом, возникает взаимная блокировка - dead_lock.

	Для обхода этого в employer есть функция check_for_finish(), которая
	проверяет завершены ли все потоки. Дальнейшее зависит от того, что нужно
	делать пока check_for_finish не вернёт true. Для wxWidgets это будет
	выглядеть так:

		while (!check_for_finish() && wxTheApp->Pending())
			wxTheApp->Dispatch();

	Но разработчик сам должен убедиться, что после этого цикла ни один
	worker не будет создан! У меня, к примеру, извне вызывалась функция
	создания асинхронного таймера - создавался он только при необходимости,
	поэтому иметь постоянно работающий таймер было жестоко. В этом случае
	простого if (!finish()) timer.async_wait(..) будет недостаточно, т.к.
	есть вероятность, что проверка if (!finish()) выполнится до lets_finish(),
	а timer.async_wait() уже после check_for_finish().

	В любом случае, в конце деструктора всё же стоит поставить
	wait_for_finish(). В этом случае, если обработчик таймера "правильно"
	сделан и проверяет finish(), то мы зависнем до первой сработки таймера.
	А это лучше, чем получить undefined behavior из-за обращения
	к разрушенному объекту.

	Деструктор employer вызывает wait_for_finish(), но где уверенность,
	что ваш "недобитый" worker не попытается обратиться к уже разрушенным
	в деструкторе переменным/структурам/объектам вашего класса?


	* Принудительное прерывание деятельности worker'а *

	Установка флага finish() вещь хорошая, но недостаточная. Может пройти
	много времени, пока worker не увидит, что пора заканчивать (пока не
	сработает таймер или пока не поступят данные на вход в случае
	синхронного/асинхронного чтения). Необходимы дополнительные действия.
	Для таймера - вызов timer.cancel(), для сокетов - socket.close().

	При создании worker'а в функции new_worker() можно через boost::bind
	задать такой "завершитель".

	Сокет:

	new_worker( "async_read",
		boost::bind( &socket_class::close, &socket) );

	Таймер:

	new_worker( "timer",
		boost::bind( &timer_class::cancel, &timer) );

	Важное замечание при использовании Boost.Asio (и, скорее всего, любых
	стандартных асинхронных функций) в Windows: socket.cancel() не работает!
	А после socket.close() функция-обработчик вызовется однозначно и может
	не раз (собственно, это и было самой главной проблемой, из-за которой
	был создан employer). Поэтому, в обработчике надо обязательно проверять
	finish().


	* Отладка *

	Для проверки, кто из worker'ов не закончил свою работу, и как много
	существует на него указателей, есть функция workers_state(), которая
	сохраняет в переданном vector'е строки вида:
		worker_name_1 - finished (1)
		worker_name_2 - works (4)

	И пусть число 4 вас не смущает. Это boost::bind создаёт кучу копий
	(я не знаю, зачем). И не забывайте, что одна копия хранится у employer'а.
*/

#ifndef MY_EMPLOYER_H
#define MY_EMPLOYER_H

#include <cstddef> /* std::size_t */
#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "my_ptr.h"
#include "my_time.h"
#include "my_thread.h"
#include "my_exception.h"

#ifdef MY_LOCK_DEBUG
#include "my_str.h"
#endif

namespace my
{

/* Класс "работника" */
class worker
{
public:
	friend class employer;
	typedef shared_ptr<class worker> ptr;

private:
	shared_lock<shared_mutex> lock_;
	std::wstring name_;
	bool finish_;
	mutex mutex_;
	condition_variable sleep_cond_;
	boost::function<void ()> on_finish_;

	void do_finish()
	{
		if (on_finish_)
			on_finish_();
	}

public:
	worker(shared_mutex &mutex, const std::wstring &name,
		boost::function<void ()> on_finish)
		: lock_(mutex)
		, name_(name)
		, finish_(false)
		, MY_MUTEX_DEFN(mutex_, my::str::to_string(name), true)
		, on_finish_(on_finish)
	{
	}

	inline mutex& get_mutex()
		{ return mutex_; }
};

/* Класс "работодателя" */
class employer
{
private:
	typedef std::list<worker::ptr> workers_list;
	bool employer_finish_;
	shared_mutex employer_mutex_;
	workers_list employer_workers_;

public:
	employer()
		: employer_finish_(false)
		, MY_MUTEX_DEF(employer_mutex_, true) {}

	employer(const std::string &name)
		: employer_finish_(false)
		, MY_MUTEX_DEFN(employer_mutex_, name, true) {}

	~employer() {}

	/* Создание нового "работника". Возвращается указатель на работника.
		Напрямую к работнику стучаться не надо */
	worker::ptr new_worker(const std::wstring &name = std::wstring(),
		boost::function<void ()> on_finish = boost::function<void ()>())
	{
		worker::ptr ptr( new worker(employer_mutex_, name, on_finish) );

		employer_workers_.push_back(ptr);

		return ptr;
	}

	/* Усыпить поток (но только, если не было команды завершить работу) */
	bool sleep(worker::ptr ptr)
	{
		if (ptr && !employer_finish_)
		{
			/* Блокировкой гарантируем атомарность операций:
				сравнения и засыпания */
			unique_lock<mutex> lock(ptr->mutex_);
			ptr->sleep_cond_.wait(lock);
			return true;
		}

		return false;
	}

	/* Усыпить поток (но только, если не было команды завершить работу).
		Блокировка обеспечена вызывающей стороной */
	template<typename Lock>
	bool sleep(worker::ptr ptr, Lock &lock)
	{
		if (ptr && !employer_finish_)
		{
			ptr->sleep_cond_.wait(lock);
			return true;
		}

		return false;
	}

	/* Усыпляем на время */
	template<typename DurationType>
	bool timed_sleep(worker::ptr ptr, DurationType rel_time)
	{
		/* Блокировкой гарантируем атомарность операций:
			сравнения и засыпания */
		if (ptr)
		{
			unique_lock<mutex> lock(ptr->mutex_);

			if (!employer_finish_)
			{
				ptr->sleep_cond_.timed_wait(lock, rel_time);
				return true;
			}
		}

		return false;
	}

	/* Усыпляем на время */
	template<typename Lock, typename DurationType>
	bool timed_sleep(worker::ptr ptr, Lock &lock, DurationType rel_time)
	{
		if (ptr && !employer_finish_)
		{
			ptr->sleep_cond_.timed_wait(lock, rel_time);
			return true;
		}

		return false;
	}

	/* Разбудить поток */
	void wake_up(worker::ptr ptr)
	{
		/* Блокировкой гарантируем, что не окажемся между
			if (!finish()) и wait(). Иначе мы "разбудим" ещё
			не спящий поток, но который тут же заснёт  */
		if (ptr)
		{
			unique_lock<mutex> lock(ptr->mutex_);
			ptr->sleep_cond_.notify_all();
		}
	}

	/* Разбудить поток. Блокировка обеспечена вызывающей стороной */
	template<typename Lock>
	void wake_up(worker::ptr ptr, Lock &)
	{
		/* Переданная блокировка не используется, параметр лишь
			напоминает, что она должна быть создана самостоятельно */
		if (ptr)
			ptr->sleep_cond_.notify_all();
	}

	/* "Увольняем" работника */
	inline void dismiss(worker::ptr &ptr)
	{
		if (ptr)
			ptr.reset();
	}

	/* Состояние работников */
	void workers_state(std::vector<std::wstring> &v)
	{
		v.clear();

		for (workers_list::iterator iter = employer_workers_.begin();
			iter != employer_workers_.end(); ++iter)
		{
			std::wostringstream out;
			long count = iter->use_count();

			out << (*iter)->name_ << L" - "
				<< (count == 0 ? L"???"
					: count == 1 ? L"finished"
					: L"works")
				<< L" (use: " << iter->use_count() << L')';

			v.push_back(out.str());
		}
	}

	/* Проверка флага завершения работы */
	inline bool finish()
		{ return employer_finish_; }

	/* Проверка флага завершения работы для работника */
	inline bool finish(worker::ptr ptr)
		{ return employer_finish_ || ptr->finish_; }

	/* Завершить работу всем работникам */
	void lets_finish()
	{
		/* Устанавливаем флаг завершения */
		employer_finish_ = true;

		/* Будим все потоки */
		void (employer::*fn)(worker::ptr) = &employer::wake_up;
		for_each(employer_workers_.begin(), employer_workers_.end(),
			boost::bind(fn, this, _1));

		/* Вызываем обработчики завершения */
		for_each(employer_workers_.begin(), employer_workers_.end(),
			boost::bind(&worker::do_finish, _1));
	}

	/* Остановить работника */
	void lets_finish(worker::ptr ptr)
	{
		/* Устанавливаем флаг завершения */
		if (ptr)
		{
			unique_lock<mutex> lock(ptr->mutex_);
			ptr->finish_ = true;
			wake_up(ptr, lock);
			ptr->do_finish();
		}
	}

	/* Отменить остановку работника */
	void cancel_finish(worker::ptr ptr)
	{
		/* Сбрасываем флаг завершения */
		if (ptr)
		{
			unique_lock<mutex> lock(ptr->mutex_);
			ptr->finish_ = false;
		}
	}

	/* Проверка - завершили ли "работники" работу */
	bool check_for_finish()
	{
		std::size_t count = 0;

		for (workers_list::iterator iter = employer_workers_.begin();
			iter != employer_workers_.end(); ++iter)
		{
			if ( !iter->unique() )
				++count;
		}

		return count == 0;
	}

	/* Проверка - работает ли работник */
	bool worked(worker::ptr &ptr)
	{
		/* Работает, если указателем владеет ещё кто-то помимо
			самого employer'а и того, кто вызвал эту функцию */
		if (!ptr)
			return false;
		else
		{
			unique_lock<mutex> lock(ptr->mutex_);
			return ptr.use_count() > 2;
		}
	}

	/* Отладка - поиск зависших */
	void debug_wait_for_finish(const std::wstring &info,
		const posix_time::time_duration &timeout)
	{
		posix_time::ptime start_finish = my::time::utc_now();
		while (!check_for_finish())
		{
			posix_time::ptime now = my::time::utc_now();

			if (now - start_finish >= timeout)
			{
				my::exception e(L"Timeout in my::employer (" + info + L")");

				e << my::param(L"start_finish", my::time::to_wstring(start_finish))
					<< my::param(L"timeout", my::time::to_wstring(timeout))
					<< my::param(L"now", my::time::to_wstring(now));

				for (workers_list::iterator iter = employer_workers_.begin();
					iter != employer_workers_.end(); ++iter)
				{
					std::wostringstream out;
					long use_count = iter->use_count();

					out << (use_count == 0 ? L"???"
							: use_count == 1 ? L"finished" : L"works")
						<< L" (use_count: " << use_count << L')';

					e << my::param(
						L"worker(\"" + (*iter)->name_ + L"\")", out.str() );
				}

				throw e;
			}
		}
	}

	/* Ожидаем, когда все "работники" завершат работу */
	void wait_for_finish()
	{
		employer_workers_.clear();
		unique_lock<shared_mutex> lock(employer_mutex_);
	}
};

}

#endif
