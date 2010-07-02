#ifndef MY_MRU_H
#define MY_MRU_H

/*
	MRU-list собственного производства.

	Создан на основе:
		boost::unordered_map (далее map) - для быстрого доступа к значению
		по уникальному ключу (key). Соответственно, для класса Key
		должна быть определена функция hash_value (см. boost::hash).
		
		std::list (далее list) - для хранения значений в порядке последнего
		использования.

	Суть:
		Map строится по ключу (key) и хранит итераторы list'а (в теории,
		они не меняются в результате добавления/удаления элементов)
		и больше ничего.

		Элементы list'а хранят указатели на mru::list (об этом далее),
		ключи (key) и значения (value).

		Mru::list, соответственно, хранит и map, и list, и предоставляет
		интерфейс для работы с последним. Все функции, возвращающие
		итератор, возвращают итератор именно list'а. Map для пользователя
		не доступен. Mru::list, также, хранит ограничитель на кол-во
		элементов в списке (max_items), но сам его никак не использует.
		
		За добавление/удаление/ограничение элементов в списке отвечает (!)
		элемент list'а - его конструктор и деструктор. Это, с одной стороны,
		вынуждает хранить в каждом из них указатель на mru::list. Но,
		с другой стороны, это значительно упрощает удаление/добавление
		/перемещение элементов.
*/

#define MRU_DEBUG 0

#if MRU_DEBUG
#include <sstream>
#include <iostream>
#include <exception>
#define SS_EXCEPTION(s) throw std::exception(((stringstream*)&(s))->str().c_str())
#endif

#include <ostream>
#include <list>
#include <boost/unordered_map.hpp>

namespace my { namespace mru {

template <typename Key, typename Value>
class list
{
private:
	class item;
	
public:
	typedef typename item item_type;
	typedef typename Key key_type;
	typedef typename Value value_type;
	typedef std::list<item> list_type;
	typedef typename list_type::iterator iterator;
	typedef typename list_type::const_iterator const_iterator;
	typedef typename list_type::reverse_iterator reverse_iterator;
	typedef typename list_type::const_reverse_iterator const_reverse_iterator;
private:
	typedef boost::unordered_map<key_type, iterator> map_type;
	typedef typename map_type::iterator map_iterator;

private:

	class item
	{
	friend class list;
	private:
		list &mru_;
		key_type key_;
		value_type value_;
		bool mapped_;

		item(list &mru, const key_type &key, const value_type &value)
			: mru_(mru)
			, key_(key)
			, value_(value)
			, mapped_(false) {}

	public:
		item(const item &other)
			: mru_(other.mru_)
			, key_(other.key_)
			, value_(other.value_)
			, mapped_(false)
		{
			assert( !other.mapped_ );
		}

		~item()
		{
			if (mapped_)
				mru_.map_.erase(key_);
		}

		template<class Char>
		friend std::basic_ostream<Char>& operator <<(
			std::basic_ostream<Char>& out, item &i)
		{
			out	<< "{" << i.key() << "=" << i.value() << "}";
			return out;
		}

		inline const key_type& key() const
			{ return key_; }

		inline value_type& value()
			{ return value_; }

		inline const value_type& value() const
			{ return value_; }

		inline bool operator <(const item &item) const
		{
			return value_ < item.value_;
		}
	};

	map_type map_;
	list_type list_;
	list_type tmp_list_; /* временный list для перемещений
		- так работает гораздо быстрее */
	size_t max_items_;

	inline void move__(iterator where, iterator *p_list_iter)
	{
		if (where != *p_list_iter)
		{
			tmp_list_.splice(tmp_list_.begin(), list_, *p_list_iter);
			list_.splice(where, tmp_list_);

			/* После splice все ранее определённые итераторы 
				и ссылки на элементы становятся invalid'ными 
				(Only iterators or references that designate
				spliced elements become invalid) */
			*p_list_iter = --where;
		}
	}


public:
	list(size_t max_items)
		: max_items_(max_items) {}

	/* Добавление нового элемента - всегда наверх */
	iterator insert(key_type const& key, value_type const& value)
	{
		/* Ищем ключ */
		map_iterator map_iter = map_.find(key);

		/* Если уже есть - удаляем */
		if (map_iter != map_.end())
			list_.erase(map_iter->second);

		/* Добавляем в list */
		iterator list_iter = list_.insert( list_.begin(),
			item(*this, key, value) );

		/* Добавляем в map */
		std::pair<map_iterator, bool> p
			= map_.insert(map_type::value_type(key, list_iter));

		list_iter->mapped_ = true;

		/* Удаляем лишние */
		while (list_.size() > max_items_)
			list_.pop_back();

		return list_iter;
	}

	/* Поиск по ключу */
	inline iterator find(key_type const& key)
	{
		map_iterator map_iter = map_.find(key);
		return (map_iter == map_.end() ? list_.end() : map_iter->second);
	}

	/* Удаление по ключу */
	void remove(key_type const& key)
	{
		map_iterator map_iter = map_.find(key);
		if (map_iter != map_.end())
			list_.erase(map_iter->second);
	}

	/* Поднять наверх */
	iterator up(key_type const& key)
	{
		map_iterator map_iter = map_.find(key);
		if (map_iter == map_.end())
			return list_.end();

		move__(list_.begin(), &map_iter->second);

		return map_iter->second;
	}

    /* Передвинуть в любое место */
	iterator move(iterator where, key_type const& key)
	{
		map_iterator map_iter = map_.find(key);
		if (map_iter == map_.end())
			return list_.end();

		move__(where, &map_iter->second);

		return map_iter->second;
	}

	/* Доступ по ключу */
	value_type& operator[](key_type const& key)
	{
		map_iterator map_iter = map_.find(key);
		iterator list_iter;

        /*
			Если ключ отсутствует, элемент будет создан - у класса
			Value должен быть определён конструктор по умолчанию.
		*/
		if (map_iter == map_.end())
			list_iter = insert(key, value_type());
		else
			/* Неявное поднятие элемента наверх упраздняем,
				т.к. если вызывающей программой был сохранён
				итератор на этот элемент, он станет invalid */
			list_iter = map_iter->second;

		return list_iter->value();
	}

	/* Восстановление итераторов, хранящихся в map'е, после операций
		с list'ом, их нарушающих (->splice) */
	/*- Убираю за ненадобностью
	void remap()
	{
		for (iterator iter = list_.begin();
			iter != list_.end(); iter++)
		{
			map_.find(iter->key())->second = iter;
		}
	}
	-*/

	inline void sort()
		{ list_.sort(); }

	template<class Pr>
	inline void sort(Pr pred)
		{ list_.sort(pred); }

	template<class Pr>
	inline void remove_if(Pr pred)
		{ list_.remove_if(pred); }


	inline iterator erase(iterator list_iter)
		{ return list_.erase(list_iter); }

	inline iterator erase(iterator first, iterator last)
		{ list_.erase(first, last); }


	inline void clear()
		{ list_.clear(); }

	
	inline iterator begin()
		{ return list_.begin(); }

	inline const_iterator begin() const
		{ return list_.begin(); }
	
	inline const_iterator cbegin() const
		{ return list_.cbegin(); }


	inline iterator end()
		{ return list_.end(); }
	
	inline const_iterator end() const
		{ return list_.end(); }

	inline const_iterator cend() const
		{ return list_.cend(); }


	inline reverse_iterator rbegin()
		{ return list_.rbegin(); }

	inline const_reverse_iterator rbegin() const
		{ return list_.rbegin(); }

	inline const_reverse_iterator crbegin() const
		{ return list_.crbegin(); }

	
	inline reverse_iterator rend()
		{ return list_.rend(); }

	inline const_reverse_iterator rend() const
		{ return list_.rend(); }

	inline const_reverse_iterator crend() const
		{ return list_.crend(); }


	inline item& front()
		{ return list_.front(); }

	inline const item& front() const
		{ return list_.front(); }


	inline item& back()
		{ return list_.back(); }

	inline const item& back() const
		{ return list_.back(); }


	inline size_t size()
		{ return list_.size(); }

	inline bool empty()
		{ return list_.empty(); }

	/*- Прямой доступ к списку - убираем
	inline list_type* operator ->()
		{ return &list_; }

	inline list_type& get()
		{ return list_; }
	-*/
};

} }

#endif
