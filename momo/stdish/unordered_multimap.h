/**********************************************************\

  momo/stdish/unordered_multimap.h

  namespace momo::stdish:
    class unordered_multimap
    class unordered_multimap_open

\**********************************************************/

#pragma once

#include "../HashMultiMap.h"

namespace momo
{

namespace stdish
{

template<typename TKey, typename TMapped,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>,
	typename THashMultiMap = HashMultiMap<TKey, TMapped, HashTraitsStd<TKey, THashFunc, TEqualFunc>,
		MemManagerStd<TAllocator>>>
class unordered_multimap
{
private:
	typedef THashMultiMap HashMultiMap;
	typedef typename HashMultiMap::HashTraits HashTraits;
	typedef typename HashMultiMap::MemManager MemManager;

public:
	typedef TKey key_type;
	typedef TMapped mapped_type;
	typedef THashFunc hasher;
	typedef TEqualFunc key_equal;
	typedef TAllocator allocator_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef std::pair<const key_type, mapped_type> value_type;

	typedef internal::UnorderedMapReference<key_type, mapped_type,
		typename HashMultiMap::Iterator::Reference> reference;
	typedef typename reference::ConstReference const_reference;

	typedef internal::HashDerivedIterator<typename HashMultiMap::Iterator, reference> iterator;
	typedef typename iterator::ConstIterator const_iterator;

	typedef typename iterator::Pointer pointer;
	typedef typename const_iterator::Pointer const_pointer;

	//local_iterator;
	//const_local_iterator;

private:
	typedef internal::ObjectBuffer<key_type, HashMultiMap::KeyValueTraits::keyAlignment> KeyBuffer;

public:
	unordered_multimap()
	{
	}

	explicit unordered_multimap(const allocator_type& alloc)
		: mHashMultiMap(HashTraits(), MemManager(alloc))
	{
	}

	explicit unordered_multimap(size_type bucketCount)
		: mHashMultiMap(HashTraits(bucketCount))
	{
	}

	unordered_multimap(size_type bucketCount, const hasher& hashFunc)
		: mHashMultiMap(HashTraits(bucketCount, hashFunc))
	{
	}

	unordered_multimap(size_type bucketCount, const hasher& hashFunc, const key_equal& equalFunc)
		: mHashMultiMap(HashTraits(bucketCount, hashFunc, equalFunc))
	{
	}

	unordered_multimap(size_type bucketCount, const hasher& hashFunc, const key_equal& equalFunc,
		const allocator_type& alloc)
		: mHashMultiMap(HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last, size_type bucketCount)
		: mHashMultiMap(HashTraits(bucketCount))
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc)
		: mHashMultiMap(HashTraits(bucketCount, hashFunc))
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalFunc)
		: mHashMultiMap(HashTraits(bucketCount, hashFunc, equalFunc))
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalFunc, const allocator_type& alloc)
		: mHashMultiMap(HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
		insert(first, last);
	}

#ifdef MOMO_USE_INIT_LISTS
	unordered_multimap(std::initializer_list<value_type> values)
	{
		insert(values);
	}

	unordered_multimap(std::initializer_list<value_type> values, size_type bucketCount)
		: mHashMultiMap(HashTraits(bucketCount))
	{
		insert(values);
	}

	unordered_multimap(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc)
		: mHashMultiMap(HashTraits(bucketCount, hashFunc))
	{
		insert(values);
	}

	unordered_multimap(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalFunc)
		: mHashMultiMap(HashTraits(bucketCount, hashFunc, equalFunc))
	{
		insert(values);
	}

	unordered_multimap(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalFunc, const allocator_type& alloc)
		: mHashMultiMap(HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
		insert(values);
	}
#endif

	unordered_multimap(unordered_multimap&& right) MOMO_NOEXCEPT
		: mHashMultiMap(std::move(right.mHashMultiMap))
	{
	}

	unordered_multimap(unordered_multimap&& right, const allocator_type& alloc)
		: mHashMultiMap(right.mHashMultiMap.GetHashTraits(), MemManager(alloc))
	{
		if (right.get_allocator() == alloc)
			*this = std::move(right);
		else
			insert(right.begin(), right.end());
		right.clear();
	}

	unordered_multimap(const unordered_multimap& right)
		: mHashMultiMap(right.mHashMultiMap)
	{
	}

	unordered_multimap(const unordered_multimap& right, const allocator_type& alloc)
		: mHashMultiMap(right.mHashMultiMap.GetHashTraits(), MemManager(alloc))
	{
		insert(right.begin(), right.end());
	}

	~unordered_multimap() MOMO_NOEXCEPT
	{
	}

	unordered_multimap& operator=(unordered_multimap&& right) MOMO_NOEXCEPT
	{
		mHashMultiMap = std::move(right.mHashMultiMap);
		return *this;
	}

	unordered_multimap& operator=(const unordered_multimap& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_copy_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			unordered_multimap(right, alloc).swap(*this);
		}
		return *this;
	}

#ifdef MOMO_USE_INIT_LISTS
	unordered_multimap& operator=(std::initializer_list<value_type> values)
	{
		clear();	//?
		insert(values);
		return *this;
	}
#endif

	void swap(unordered_multimap& right) MOMO_NOEXCEPT
	{
		mHashMultiMap.Swap(right.mHashMultiMap);
	}

	friend void swap(unordered_multimap& left, unordered_multimap& right) MOMO_NOEXCEPT
	{
		left.swap(right);
	}

	iterator begin() MOMO_NOEXCEPT
	{
		return iterator(mHashMultiMap.GetBegin());
	}

	const_iterator begin() const MOMO_NOEXCEPT
	{
		return const_iterator(mHashMultiMap.GetBegin());
	}

	iterator end() MOMO_NOEXCEPT
	{
		return iterator(mHashMultiMap.GetEnd());
	}

	const_iterator end() const MOMO_NOEXCEPT
	{
		return const_iterator(mHashMultiMap.GetEnd());
	}

	const_iterator cbegin() const MOMO_NOEXCEPT
	{
		return begin();
	}

	const_iterator cend() const MOMO_NOEXCEPT
	{
		return end();
	}

	//float max_load_factor() const MOMO_NOEXCEPT
	//void max_load_factor(float maxLoadFactor)

	hasher hash_function() const
	{
		return mHashMultiMap.GetHashTraits().GetHashFunc();
	}

	key_equal key_eq() const
	{
		return mHashMultiMap.GetHashTraits().GetEqualFunc();
	}

	allocator_type get_allocator() const //MOMO_NOEXCEPT
	{
		return mHashMultiMap.GetMemManager().GetAllocator();
	}

	size_type max_size() const //MOMO_NOEXCEPT
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const MOMO_NOEXCEPT
	{
		return mHashMultiMap.GetValueCount();
	}

	bool empty() const MOMO_NOEXCEPT
	{
		return size() == 0;
	}

	void clear() MOMO_NOEXCEPT
	{
		mHashMultiMap.Clear();
	}

	//void rehash(size_type bucketCount)
	//void reserve(size_type count)

	const_iterator find(const key_type& key) const
	{
		return equal_range(key).first;
	}

	iterator find(const key_type& key)
	{
		return equal_range(key).first;
	}

	size_type count(const key_type& key) const
	{
		typename HashMultiMap::ConstKeyIterator keyIter = mHashMultiMap.Find(key);
		return !!keyIter ? keyIter->values.GetCount() : 0;
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		typename HashMultiMap::ConstKeyIterator keyIter = mHashMultiMap.Find(key);
		if (!keyIter)
			return std::pair<const_iterator, const_iterator>(end(), end());
		size_t count = keyIter->values.GetCount();
		if (count == 0)	//?
			return std::pair<const_iterator, const_iterator>(end(), end());
		const_iterator first(mHashMultiMap.MakeIterator(keyIter, 0));
		const_iterator last(std::next(mHashMultiMap.MakeIterator(keyIter, count - 1)));
		return std::pair<const_iterator, const_iterator>(first, last);
	}

	std::pair<iterator, iterator> equal_range(const key_type& key)
	{
		typename HashMultiMap::KeyIterator keyIter = mHashMultiMap.Find(key);
		if (!keyIter)
			return std::pair<iterator, iterator>(end(), end());
		size_t count = keyIter->values.GetCount();
		if (count == 0)	//?
			return std::pair<iterator, iterator>(end(), end());
		iterator first(mHashMultiMap.MakeIterator(keyIter, 0));
		iterator last(std::next(mHashMultiMap.MakeIterator(keyIter, count - 1)));
		return std::pair<iterator, iterator>(first, last);
	}

	//template<typename Value>
	//typename std::enable_if<std::is_convertible<Value, value_type>::value,
	//	iterator>::type
	//insert(Value&& value)

	//template<typename Value>
	//typename std::enable_if<std::is_convertible<Value, value_type>::value,
	//	iterator>::type
	//insert(const_iterator, Value&& value)

	//iterator insert(const value_type& value)

	//iterator insert(const_iterator, const value_type& value)

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<const First&, key_type>::value
		&& std::is_convertible<const Second&, mapped_type>::value, iterator>::type
	insert(const std::pair<First, Second>& value)
	{
		typedef typename internal::ObjectManager<mapped_type>::template TemplCreator<const Second&> MappedCreator;
		return _insert(value.first, MappedCreator(value.second));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<const First&, key_type>::value
		&& std::is_convertible<const Second&, mapped_type>::value, iterator>::type
	insert(const_iterator, const std::pair<First, Second>& value)
	{
		return insert(value).first;
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<First, key_type>::value
		&& std::is_convertible<Second, mapped_type>::value, iterator>::type
	insert(std::pair<First, Second>&& value)
	{
		typedef typename internal::ObjectManager<mapped_type>::template TemplCreator<Second> MappedCreator;
		return _insert(std::forward<First>(value.first),
			MappedCreator(std::forward<Second>(value.second)));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<First, key_type>::value
		&& std::is_convertible<Second, mapped_type>::value, iterator>::type
	insert(const_iterator, std::pair<First, Second>&& value)
	{
		return insert(std::forward<std::pair<First, Second>>(value));
	}

	template<typename Iterator>
	void insert(Iterator first, Iterator last)
	{
		for (Iterator iter = first; iter != last; ++iter)
			insert(*iter);
	}

#ifdef MOMO_USE_INIT_LISTS
	void insert(std::initializer_list<value_type> values)
	{
		insert(values.begin(), values.end());
	}
#endif

#ifdef MOMO_USE_VARIADIC_TEMPLATES
	iterator emplace()
	{
		return emplace(std::piecewise_construct, std::tuple<>(), std::tuple<>());
	}

	iterator emplace_hint(const_iterator)
	{
		return emplace();
	}

	template<typename Arg>
	iterator emplace(Arg&& arg)
	{
		return insert(std::forward<Arg>(arg));
	}

	template<typename Arg>
	iterator emplace_hint(const_iterator, Arg&& arg)
	{
		return emplace(std::forward<Arg>(arg));
	}

	template<typename Arg1, typename Arg2>
	iterator emplace(Arg1&& arg1, Arg2&& arg2)
	{
		typedef typename internal::ObjectManager<mapped_type>::template TemplCreator<Arg2> MappedCreator;
		return _insert(std::forward<Arg1>(arg1), MappedCreator(std::forward<Arg2>(arg2)));
	}

	template<typename Arg1, typename Arg2>
	iterator emplace_hint(const_iterator, Arg1&& arg1, Arg2&& arg2)
	{
		return emplace(std::forward<Arg1>(arg1), std::forward<Arg2>(arg2));
	}

	template<typename... Args1, typename... Args2>
	iterator emplace(std::piecewise_construct_t,
		std::tuple<Args1...> args1, std::tuple<Args2...> args2)
	{
		return _emplace(std::move(args1), std::move(args2));
	}

	template<typename... Args1, typename... Args2>
	iterator emplace_hint(const_iterator, std::piecewise_construct_t,
		std::tuple<Args1...> args1, std::tuple<Args2...> args2)
	{
		return _emplace(std::move(args1), std::move(args2));
	}
#endif

	//iterator erase(const_iterator where)
	iterator erase(iterator where)
	{
		typename HashMultiMap::Iterator iter = where.GetBaseIterator();
		typename HashMultiMap::KeyIterator keyIter = iter.GetKeyIterator();
		if (keyIter->values.GetCount() == 1)
			return iterator(mHashMultiMap.RemoveKey(keyIter));
		else
			return iterator(mHashMultiMap.Remove(iter));
	}

	//iterator erase(const_iterator first, const_iterator last)
	iterator erase(iterator first, iterator last)
	{
		if (first == end())
			return end();
		if (first == last)
			return first;
		if (first == begin() && last == end())
		{
			clear();
			return end();
		}
		if (std::next(first) == last)
			return erase(first);
		typename HashMultiMap::KeyIterator keyIter = first.GetBaseIterator().GetKeyIterator();
		size_t count = keyIter->values.GetCount();
		assert(count > 0);
		if (std::next(mHashMultiMap.MakeIterator(keyIter, count - 1)) == last.GetBaseIterator())
			return iterator(mHashMultiMap.RemoveKey(keyIter));
		throw std::invalid_argument("invalid unordered_multimap erase arguments");
	}

	size_type erase(const key_type& key)
	{
		return mHashMultiMap.RemoveKey(key);
	}

	//size_type max_bucket_count() const MOMO_NOEXCEPT
	//size_type bucket_count() const MOMO_NOEXCEPT
	//size_type bucket_size(size_type bucketIndex) const
	//local_iterator begin(size_type bucketIndex)
	//const_local_iterator begin(size_type bucketIndex) const
	//local_iterator end(size_type bucketIndex)
	//const_local_iterator end(size_type bucketIndex) const
	//const_local_iterator cbegin(size_type bucketIndex) const
	//const_local_iterator cend(size_type bucketIndex) const
	//size_type bucket(const key_type& key) const
	//float load_factor() const MOMO_NOEXCEPT

	bool operator==(const unordered_multimap& right) const
	{
		if (mHashMultiMap.GetKeyCount() != right.mHashMultiMap.GetKeyCount())
			return false;
		if (mHashMultiMap.GetValueCount() != right.mHashMultiMap.GetValueCount())
			return false;
		for (typename HashMultiMap::KeyIterator::ConstReference ref : mHashMultiMap.GetKeyBounds())
		{
			typename HashMultiMap::ConstKeyIterator keyIterRight = right.mHashMultiMap.Find(ref.key);
			if (!keyIterRight)
				return false;
			typename HashMultiMap::ConstValueBounds values = ref.values;
			typename HashMultiMap::ConstValueBounds valuesRight = keyIterRight->values;
			if (values.GetCount() != valuesRight.GetCount())
				return false;
			if (!std::is_permutation(values.GetBegin(), values.GetEnd(), valuesRight.GetBegin()))
				return false;
		}
		return true;
	}

	bool operator!=(const unordered_multimap& right) const
	{
		return !(*this == right);
	}

private:
	template<typename Key, typename MappedCreator>
	iterator _insert(Key&& key, const MappedCreator& mappedCreator)
	{
		typedef typename internal::ObjectManager<key_type>::template TemplCreator<Key> KeyCreator;
		KeyBuffer keyBuffer;
		KeyCreator(std::forward<Key>(key))(&keyBuffer);
		iterator resIter;
		try
		{
			resIter = _insert(std::move(*&keyBuffer), mappedCreator);
		}
		catch (...)
		{
			HashMultiMap::KeyValueTraits::DestroyKey(*&keyBuffer);
			throw;
		}
		HashMultiMap::KeyValueTraits::DestroyKey(*&keyBuffer);
		return resIter;
	}

	template<typename MappedCreator>
	iterator _insert(key_type&& key, const MappedCreator& mappedCreator)
	{
		return iterator(mHashMultiMap.AddCrt(std::move(key), mappedCreator));
	}

	template<typename MappedCreator>
	iterator _insert(const key_type& key, const MappedCreator& mappedCreator)
	{
		return iterator(mHashMultiMap.AddCrt(key, mappedCreator));
	}

#ifdef MOMO_USE_VARIADIC_TEMPLATES
	template<typename... Args1, typename... Args2>
	iterator _emplace(std::tuple<Args1...>&& args1, std::tuple<Args2...>&& args2)
	{
		typedef typename internal::ObjectManager<key_type>::template VariadicCreator<Args1...> KeyCreator;
		typedef typename internal::ObjectManager<mapped_type>::template VariadicCreator<Args2...> MappedCreator;
		KeyBuffer keyBuffer;
		KeyCreator(std::move(args1))(&keyBuffer);
		iterator resIter;
		try
		{
			resIter = _insert(std::move(*&keyBuffer), MappedCreator(std::move(args2)));
		}
		catch (...)
		{
			HashMultiMap::KeyValueTraits::DestroyKey(*&keyBuffer);
			throw;
		}
		HashMultiMap::KeyValueTraits::DestroyKey(*&keyBuffer);
		return resIter;
	}
#endif

private:
	HashMultiMap mHashMultiMap;
};

#ifdef MOMO_USE_TYPE_ALIASES
template<typename TKey, typename TMapped,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_multimap_open = unordered_multimap<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
	HashMultiMap<TKey, TMapped, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketOneI1>,
		MemManagerStd<TAllocator>>>;
#endif

} // namespace stdish

} // namespace momo
