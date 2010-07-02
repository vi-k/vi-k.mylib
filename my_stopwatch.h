/*
	Секундомер
*/

#ifndef MY_STOPWATCH_H
#define MY_STOPWATCH_H

#include <ostream>
#include <string>
#include <list>

#include "my_time.h"

#define MY_SW_COUNT my::stopwatch::show_count
#define MY_SW_TOTAL my::stopwatch::show_total
#define MY_SW_AVG   my::stopwatch::show_avg
#define MY_SW_MIN   my::stopwatch::show_min
#define MY_SW_MAX   my::stopwatch::show_max
#define MY_SW_ALL   my::stopwatch::show_all

namespace my {

class stopwatch
{
public:
	enum {show_count=1, show_total=2, show_avg=4,
		show_min=8, show_max=16, show_all=31};

private:
	posix_time::ptime start_;
	int show_;
	int count_;
	posix_time::time_duration total_;
	posix_time::time_duration min_;
	posix_time::time_duration max_;

	struct period
	{
		posix_time::time_duration total;
		int count;
	};

	typedef std::vector<period> periods_list;
	periods_list periods_;

public:

	stopwatch(int show = show_all)
		: count_(0)
		, show_(show)
	{
		start();
	}

	/* Сбросить секундомер */
	inline void reset(bool keep_periods = false)
	{
		count_ = 0;
		total_ = max_ = min_ = posix_time::time_duration();

		if (!keep_periods)
			periods_.clear();

		start();
	}

    /* Старт (после паузы) */
	inline void start()
	{
		start_ = posix_time::microsec_clock::universal_time();
	}

    /* Остановка (пауза) */
	inline void finish()
	{
		posix_time::time_duration time
			= posix_time::microsec_clock::universal_time() - start_;

		if (time < min_ || count_ == 0)
			min_ = time;
		if (time > max_ || count_ == 0)
			max_ = time;

		total_ += time;
		++count_;
	}

	/* Сохранить состояние в очереди */
	void push()
	{
		period p;
		p.total = total_;
		p.count = count_;

		periods_.insert(periods_.begin(), p);

		reset(true);
	}

    /* Удалить самое раннее состояние из очереди */
	void pop_back()
	{
		periods_.pop_back();
	}


    /* Итоговое время */
	inline posix_time::time_duration total() const
		{ return total_; }

    /* Итоговое время с учётом сохранённых состояний */
	posix_time::time_duration full_total() const
	{
		posix_time::time_duration full_total = total_;

		for (periods_list::const_iterator iter = periods_.begin();
			iter != periods_.end(); ++iter)
		{
			full_total += iter->total;
		}

		return full_total;
	}

	
    /* Количество измеренных периодов */
	inline int count() const
		{ return count_; }

    /* Итоговое время с учётом сохранённых состояний */
	int full_count() const
	{
		int full_count = count_;

		for (periods_list::const_iterator iter = periods_.begin();
			iter != periods_.end(); ++iter)
		{
			full_count += iter->count;
		}

		return full_count;
	}


	/* Максимальное время между start() и stop() */
	inline posix_time::time_duration max() const
		{ return max_; }

	/* Минимальное время между start() и stop() */
	inline posix_time::time_duration min() const
		{ return min_; }


	/* Среднее время */
	inline posix_time::time_duration avg() const
	{
		return count_ ? (total_ / count_)
			: posix_time::time_duration();
	}

	/* Среднее время с учётом сохранённых состояний */
	inline posix_time::time_duration full_avg() const
	{
		int n = full_count();
		return n ? (full_total() / n)
			: posix_time::time_duration();
	}

	template<class Char>
	friend std::basic_ostream<Char>& operator <<(std::basic_ostream<Char>& out,
		const stopwatch &sw)
	{
		if (sw.count_ == 0)
			out << "null";
		else if (sw.count_ == 1 && (sw.show_ & show_total))
			out << "total=" << sw.total_;
		else
		{
			int index = 0;
			if (sw.show_ & show_count)
				out	<< (index++ ? " " : "") << "count=" << sw.count_;
			if (sw.show_ & show_total)
				out << (index++ ? " " : "") << "total=" << sw.total_;
			if (sw.show_ & show_avg)
				out << (index++ ? " " : "") << "avg=" << sw.avg();
			if (sw.show_ & show_min)
				out << (index++ ? " " : "") << "min=" << sw.min_;
			if (sw.show_ & show_max)
				out << (index++ ? " " : "") << "max=" << sw.max_;
		}

		return out;
	}	

	template<class Char>
	std::basic_string<Char> to_str()
	{
		basic_ostringstream<Char> out;
		out << *this;
		return out.str();
	}

	inline std::string to_string()
		{ return to_str<char>(); }

	inline std::wstring to_wstring()
		{ return to_str<wchar_t>(); }

};

}

#endif
