/**********************************************************\

  momo/stdish/map.h

  namespace momo::stdish:
    class map

\**********************************************************/

#pragma once

#include "../TreeMap.h"

namespace momo
{

namespace stdish
{

template<typename TKey, typename TMapped,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>,
	typename TTreeMap = TreeMap<TKey, TMapped, TreeTraitsStd<TKey, TLessFunc>,
		MemManagerStd<TAllocator>>>
class map
{
private:
	typedef TTreeMap TreeMap;
	typedef typename TreeMap::TreeTraits TreeTraits;
	typedef typename TreeMap::MemManager MemManager;

public:
	typedef TKey key_type;
	typedef TMapped mapped_type;
	typedef TLessFunc key_compare;
	typedef TAllocator allocator_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef std::pair<const key_type, mapped_type> value_type;
	
	class value_compare : public std::binary_function<value_type, value_type, bool>
	{
	public:
		explicit value_compare(const key_compare& keyComp)
			: mKeyComp(keyComp)
		{
		}

		bool operator()(const value_type& value1, const value_type& value2) const
		{
			return mKeyComp(value1.first, value2.first);
		}

	private:
		key_compare mKeyComp;
	};

	typedef internal::MapReferenceStd<key_type, mapped_type,
		typename TreeMap::Iterator::Reference> reference;
	typedef typename reference::ConstReference const_reference;

	typedef internal::TreeDerivedIterator<typename TreeMap::Iterator, reference> iterator;
	typedef typename iterator::ConstIterator const_iterator;

	typedef typename iterator::Pointer pointer;
	typedef typename const_iterator::Pointer const_pointer;

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

private:
	typedef internal::ObjectBuffer<key_type, TreeMap::KeyValueTraits::keyAlignment> KeyBuffer;

public:
	map()
	{
	}

	explicit map(const allocator_type& alloc)
		: mTreeMap(TreeTraits(), MemManager(alloc))
	{
	}

	explicit map(const key_compare& lessFunc)
		: mTreeMap(TreeTraits(lessFunc))
	{
	}

	map(const key_compare& lessFunc, const allocator_type& alloc)
		: mTreeMap(TreeTraits(lessFunc), MemManager(alloc))
	{
	}

	template<typename Iterator>
	map(Iterator first, Iterator last)
	{
		insert(first, last);
	}

	template<typename Iterator>
	map(Iterator first, Iterator last, const key_compare& lessFunc)
		: mTreeMap(TreeTraits(lessFunc))
	{
		insert(first, last);
	}

	template<typename Iterator>
	map(Iterator first, Iterator last, const key_compare& lessFunc, const allocator_type& alloc)
		: mTreeMap(TreeTraits(lessFunc), MemManager(alloc))
	{
		insert(first, last);
	}

	map(std::initializer_list<value_type> values)
		: mTreeMap(values)
	{
	}

	map(std::initializer_list<value_type> values, const key_compare& lessFunc)
		: mTreeMap(values, TreeTraits(lessFunc))
	{
	}

	map(std::initializer_list<value_type> values, const key_compare& lessFunc,
		const allocator_type& alloc)
		: mTreeMap(values, TreeTraits(lessFunc), MemManager(alloc))
	{
	}

	map(map&& right) MOMO_NOEXCEPT
		: mTreeMap(std::move(right.mTreeMap))
	{
	}

	map(map&& right, const allocator_type& alloc)
		: mTreeMap(right.mTreeMap.GetTreeTraits(), MemManager(alloc))
	{
		if (right.get_allocator() == alloc)
			*this = std::move(right);
		else
			insert(right.begin(), right.end());
		right.clear();
	}

	map(const map& right)
		: mTreeMap(right.mTreeMap)
	{
	}

	map(const map& right, const allocator_type& alloc)
		: mTreeMap(right.mTreeMap.GetTreeTraits(), MemManager(alloc))
	{
		insert(right.begin(), right.end());
	}

	~map() MOMO_NOEXCEPT
	{
	}

	map& operator=(map&& right) MOMO_NOEXCEPT
	{
		mTreeMap = std::move(right.mTreeMap);
		return *this;
	}

	map& operator=(const map& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_copy_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			map(right, alloc).swap(*this);
		}
		return *this;
	}

	map& operator=(std::initializer_list<value_type> values)
	{
		clear();	//?
		insert(values);
		return *this;
	}

	void swap(map& right) MOMO_NOEXCEPT
	{
		mTreeMap.Swap(right.mTreeMap);
	}

	friend void swap(map& left, map& right) MOMO_NOEXCEPT
	{
		left.swap(right);
	}

	iterator begin() MOMO_NOEXCEPT
	{
		return iterator(mTreeMap.GetBegin());
	}

	const_iterator begin() const MOMO_NOEXCEPT
	{
		return const_iterator(mTreeMap.GetBegin());
	}

	iterator end() MOMO_NOEXCEPT
	{
		return iterator(mTreeMap.GetEnd());
	}

	const_iterator end() const MOMO_NOEXCEPT
	{
		return const_iterator(mTreeMap.GetEnd());
	}

	reverse_iterator rbegin() MOMO_NOEXCEPT
	{
		return reverse_iterator(end());
	}

	const_reverse_iterator rbegin() const MOMO_NOEXCEPT
	{
		return reverse_iterator(end());
	}

	reverse_iterator rend() MOMO_NOEXCEPT
	{
		return reverse_iterator(begin());
	}

	const_reverse_iterator rend() const MOMO_NOEXCEPT
	{
		return reverse_iterator(begin());
	}

	const_iterator cbegin() const MOMO_NOEXCEPT
	{
		return begin();
	}

	const_iterator cend() const MOMO_NOEXCEPT
	{
		return end();
	}

	const_reverse_iterator crbegin() const MOMO_NOEXCEPT
	{
		return rbegin();
	}

	const_reverse_iterator crend() const MOMO_NOEXCEPT
	{
		return rend();
	}

	key_compare key_comp() const
	{
		return mTreeMap.GetTreeTraits().GetLessFunc();
	}

	value_compare value_comp() const
	{
		return value_compare(key_comp());
	}

	allocator_type get_allocator() const //MOMO_NOEXCEPT
	{
		return mTreeMap.GetMemManager().GetAllocator();
	}

	size_type max_size() const //MOMO_NOEXCEPT
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const MOMO_NOEXCEPT
	{
		return mTreeMap.GetCount();
	}

	bool empty() const MOMO_NOEXCEPT
	{
		return mTreeMap.IsEmpty();
	}

	void clear() MOMO_NOEXCEPT
	{
		mTreeMap.Clear();
	}

	const_iterator find(const key_type& key) const
	{
		return const_iterator(mTreeMap.Find(key));
	}

	iterator find(const key_type& key)
	{
		return iterator(mTreeMap.Find(key));
	}

	size_type count(const key_type& key) const
	{
		return mTreeMap.HasKey(key) ? 1 : 0;
	}

	const_iterator lower_bound(const key_type& key) const
	{
		return const_iterator(mTreeMap.LowerBound(key));
	}

	iterator lower_bound(const key_type& key)
	{
		return iterator(mTreeMap.LowerBound(key));
	}

	const_iterator upper_bound(const key_type& key) const
	{
		return const_iterator(mTreeMap.UpperBound(key));
	}

	iterator upper_bound(const key_type& key)
	{
		return iterator(mTreeMap.UpperBound(key));
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		const_iterator iter = lower_bound(key);
		if (iter == end() || mTreeMap.GetTreeTraits().Less(key, *iter))
			return std::pair<const_iterator, const_iterator>(iter, iter);
		return std::pair<const_iterator, const_iterator>(iter, std::next(iter));
	}

	std::pair<iterator, iterator> equal_range(const key_type& key)
	{
		iterator iter = lower_bound(key);
		if (iter == end() || mTreeMap.GetTreeTraits().Less(key, *iter))
			return std::pair<iterator, iterator>(iter, iter);
		return std::pair<iterator, iterator>(iter, std::next(iter));
	}

	//template<typename Value>
	//typename std::enable_if<std::is_convertible<Value, value_type>::value,
	//	std::pair<iterator, bool>>::type
	//insert(Value&& value)

	//template<typename Value>
	//typename std::enable_if<std::is_convertible<Value, value_type>::value,
	//	iterator>::type
	//insert(const_iterator, Value&& value)

	//std::pair<iterator, bool> insert(const value_type& value)

	//iterator insert(const_iterator, const value_type& value)

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<const First&, key_type>::value
		&& std::is_convertible<const Second&, mapped_type>::value,
		std::pair<iterator, bool>>::type
	insert(const std::pair<First, Second>& value)
	{
		typedef typename TreeMap::KeyValueTraits::template ValueCreator<const Second&> MappedCreator;
		return _insert(nullptr, value.first, MappedCreator(value.second));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<const First&, key_type>::value
		&& std::is_convertible<const Second&, mapped_type>::value, iterator>::type
	insert(const_iterator hint, const std::pair<First, Second>& value)
	{
		typedef typename TreeMap::KeyValueTraits::template ValueCreator<const Second&> MappedCreator;
		return _insert(hint, value.first, MappedCreator(value.second)).first;
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<First, key_type>::value
		&& std::is_convertible<Second, mapped_type>::value,
		std::pair<iterator, bool>>::type
	insert(std::pair<First, Second>&& value)
	{
		typedef typename TreeMap::KeyValueTraits::template ValueCreator<Second> MappedCreator;
		return _insert(nullptr, std::forward<First>(value.first),
			MappedCreator(std::forward<Second>(value.second)));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<First, key_type>::value
		&& std::is_convertible<Second, mapped_type>::value, iterator>::type
	insert(const_iterator hint, std::pair<First, Second>&& value)
	{
		typedef typename TreeMap::KeyValueTraits::template ValueCreator<Second> MappedCreator;
		return _insert(hint, std::forward<First>(value.first),
			MappedCreator(std::forward<Second>(value.second))).first;
	}

	template<typename Iterator>
	void insert(Iterator first, Iterator last)
	{
		for (Iterator iter = first; iter != last; ++iter)
			insert(*iter);
	}

	void insert(std::initializer_list<value_type> values)
	{
		insert(values.begin(), values.end());
	}

	std::pair<iterator, bool> emplace()
	{
		return _emplace(nullptr, std::tuple<>(), std::tuple<>());
	}

	iterator emplace_hint(const_iterator hint)
	{
		return _emplace(hint, std::tuple<>(), std::tuple<>()).first;
	}

	template<typename Arg>
	std::pair<iterator, bool> emplace(Arg&& arg)
	{
		return insert(std::forward<Arg>(arg));
	}

	template<typename Arg>
	iterator emplace_hint(const_iterator hint, Arg&& arg)
	{
		return insert(hint, std::forward<Arg>(arg));
	}

	template<typename Arg1, typename Arg2>
	std::pair<iterator, bool> emplace(Arg1&& arg1, Arg2&& arg2)
	{
		typedef typename TreeMap::KeyValueTraits::template ValueCreator<Arg2> MappedCreator;
		return _insert(nullptr, std::forward<Arg1>(arg1), MappedCreator(std::forward<Arg2>(arg2)));
	}

	template<typename Arg1, typename Arg2>
	iterator emplace_hint(const_iterator hint, Arg1&& arg1, Arg2&& arg2)
	{
		typedef typename TreeMap::KeyValueTraits::template ValueCreator<Arg2> MappedCreator;
		return _insert(hint, std::forward<Arg1>(arg1),
			MappedCreator(std::forward<Arg2>(arg2))).first;
	}

	template<typename... Args1, typename... Args2>
	std::pair<iterator, bool> emplace(std::piecewise_construct_t,
		std::tuple<Args1...> args1, std::tuple<Args2...> args2)
	{
		return _emplace(nullptr, std::move(args1), std::move(args2));
	}

	template<typename... Args1, typename... Args2>
	iterator emplace_hint(const_iterator hint, std::piecewise_construct_t,
		std::tuple<Args1...> args1, std::tuple<Args2...> args2)
	{
		return _emplace(hint, std::move(args1), std::move(args2)).first;
	}

	iterator erase(const_iterator where)
	{
		return iterator(mTreeMap.Remove(where.GetBaseIterator()));
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		if (first == begin() && last == end())
		{
			clear();
			return end();
		}
		if (first == last)
			return (first != end()) ? find(first->key) : end();
		size_t count = std::distance(first, last);
		iterator iter = erase(first);
		for (size_t i = 1; i < count; ++i)
			iter = erase(iter);
		return iter;
	}

	size_type erase(const key_type& key)
	{
		return mTreeMap.Remove(key) ? 1 : 0;
	}

	typename TreeMap::ValueReferenceRKey operator[](key_type&& key)
	{
		return mTreeMap[std::move(key)];
	}

	typename TreeMap::ValueReferenceCKey operator[](const key_type& key)
	{
		return mTreeMap[key];
	}

	const mapped_type& at(const key_type& key) const
	{
		const_iterator iter = find(key);
		if (iter == end())
			throw std::out_of_range("invalid map key");
		return iter->second;
	}

	mapped_type& at(const key_type& key)
	{
		iterator iter = find(key);
		if (iter == end())
			throw std::out_of_range("invalid map key");
		return iter->second;
	}

	bool operator==(const map& right) const
	{
		return size() == right.size() && std::equal(begin(), end(), right.begin());
	}

	bool operator!=(const map& right) const
	{
		return !(*this == right);
	}

	bool operator<(const map& right) const
	{
		return std::lexicographical_compare(begin(), end(), right.begin(), right.end());
	}

	bool operator>(const map& right) const
	{
		return right < *this;
	}

	bool operator<=(const map& right) const
	{
		return !(right < *this);
	}

	bool operator>=(const map& right) const
	{
		return right <= *this;
	}

private:
	bool _check_hint(const_iterator hint, const key_type& key) const
	{
		const TreeTraits& treeTraits = mTreeMap.GetTreeTraits();
		return (hint == begin() || treeTraits.Less(*std::prev(hint), key))
			&& (hint == end() || !treeTraits.Less(*hint, key));
	}

	template<typename Hint, typename Key, typename MappedCreator>
	std::pair<iterator, bool> _insert(Hint hint, Key&& key, const MappedCreator& mappedCreator)
	{
		typedef internal::ObjectManager<key_type> KeyManager;
		typedef typename KeyManager::template Creator<Key> KeyCreator;
		KeyBuffer keyBuffer;
		KeyCreator(std::forward<Key>(key))(&keyBuffer);
		std::pair<iterator, bool> res;
		try
		{
			res = _insert(hint, std::move(*&keyBuffer), mappedCreator);
		}
		catch (...)
		{
			KeyManager::Destroy(*&keyBuffer);
			throw;
		}
		KeyManager::Destroy(*&keyBuffer);
		return res;
	}

	template<typename MappedCreator>
	std::pair<iterator, bool> _insert(std::nullptr_t, key_type&& key,
		const MappedCreator& mappedCreator)
	{
		typename TreeMap::InsertResult res = mTreeMap.InsertCrt(std::move(key), mappedCreator);
		return std::pair<iterator, bool>(iterator(res.iterator), res.inserted);
	}

	template<typename MappedCreator>
	std::pair<iterator, bool> _insert(const_iterator hint, key_type&& key,
		const MappedCreator& mappedCreator)
	{
		if (!_check_hint(hint, (const key_type&)key))
			return _insert(nullptr, std::move(key), mappedCreator);
		typename TreeMap::Iterator resIter = mTreeMap.AddCrt(hint.GetBaseIterator(),
			std::move(key), mappedCreator);
		return std::pair<iterator, bool>(iterator(resIter), true);
	}

	template<typename MappedCreator>
	std::pair<iterator, bool> _insert(std::nullptr_t, const key_type& key,
		const MappedCreator& mappedCreator)
	{
		typename TreeMap::InsertResult res = mTreeMap.InsertCrt(key, mappedCreator);
		return std::pair<iterator, bool>(iterator(res.iterator), res.inserted);
	}

	template<typename MappedCreator>
	std::pair<iterator, bool> _insert(const_iterator hint, const key_type& key,
		const MappedCreator& mappedCreator)
	{
		if (!_check_hint(hint, key))
			return _insert(nullptr, key, mappedCreator);
		typename TreeMap::Iterator resIter = mTreeMap.AddCrt(hint.GetBaseIterator(),
			key, mappedCreator);
		return std::pair<iterator, bool>(iterator(resIter), true);
	}

	template<typename Hint, typename... Args1, typename... Args2>
	std::pair<iterator, bool> _emplace(Hint hint, std::tuple<Args1...>&& args1,
		std::tuple<Args2...>&& args2)
	{
		typedef internal::ObjectManager<key_type> KeyManager;
		typedef typename KeyManager::template Creator<Args1...> KeyCreator;
		typedef typename TreeMap::KeyValueTraits::template ValueCreator<Args2...> MappedCreator;
		KeyBuffer keyBuffer;
		KeyCreator(std::move(args1))(&keyBuffer);
		std::pair<iterator, bool> res;
		try
		{
			res = _insert(hint, std::move(*&keyBuffer), MappedCreator(std::move(args2)));
		}
		catch (...)
		{
			KeyManager::Destroy(*&keyBuffer);
			throw;
		}
		KeyManager::Destroy(*&keyBuffer);
		return res;
	}

private:
	TreeMap mTreeMap;
};

} // namespace stdish

} // namespace momo
