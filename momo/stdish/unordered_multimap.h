/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/stdish/unordered_multimap.h

  namespace momo::stdish:
    class unordered_multimap
    class unordered_multimap_open

  This classes are similar to `std::unordered_multimap`.
  `unordered_multimap` is much more efficient than standard one in
  memory usage. Its implementation is based on hash tables with
  buckets in the form of small arrays.
  `unordered_multimap_open` is based on open addressing hash table.

  Deviations from the `std::unordered_multimap`:
  1. Each of duplicate keys stored only once.
  2. `max_load_factor`, `rehash`, `reserve`, `load_factor` and all
    the functions, associated with the bucket, are not implemented.
  3. Functions `erase` take non-constant iterators.
  4. Container items must be movable (preferably without exceptions)
    or copyable, similar to items of `std::vector`.
  5. After each addition or removal of the item all iterators and
    references to items become invalid and should not be used.
  6. Type `reference` is not the same as `value_type&`, so
    `for (auto& p : map)` is illegal, but `for (auto p : map)` or
    `for (const auto& p : map)` or `for (auto&& p : map)` is allowed.
  7.1. Container move constructor, move assignment operator and swap
    function do not throw exceptions regardless of the allocator.
  7.2. Functions of the allocator `construct`, `destroy` and `address`
    are not used.
  7.3. It is expected that the allocator types `pointer`, `const_pointer`,
    `reference`, `const_reference`, `size_type` and `difference_type`
    have the standard definition (as in `std::allocator`).
  7.4. It is expected that the allocator types `propagate_on_container_swap`
    and `propagate_on_container_move_assignment` are the same as
    `std::true_type`.

  It is allowed to pass to functions `insert` and `emplace` references
  to items within the container.
  But in case of the function `insert`, receiving pair of iterators, it's
  not allowed to pass iterators pointing to the items within the container. 

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

	typedef internal::MapReferenceStd<key_type, mapped_type,
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

	explicit unordered_multimap(size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: mHashMultiMap(HashTraits(bucketCount), MemManager(alloc))
	{
	}

	unordered_multimap(size_type bucketCount, const hasher& hashFunc,
		const allocator_type& alloc = allocator_type())
		: mHashMultiMap(HashTraits(bucketCount, hashFunc), MemManager(alloc))
	{
	}

	unordered_multimap(size_type bucketCount, const hasher& hashFunc, const key_equal& equalFunc,
		const allocator_type& alloc = allocator_type())
		: mHashMultiMap(HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(bucketCount, alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last, size_type bucketCount,
		const hasher& hashFunc, const allocator_type& alloc = allocator_type())
		: unordered_multimap(bucketCount, hashFunc, alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalFunc,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(bucketCount, hashFunc, equalFunc, alloc)
	{
		insert(first, last);
	}

	unordered_multimap(std::initializer_list<value_type> values,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(values.begin(), values.end(), alloc)
	{
	}

	unordered_multimap(std::initializer_list<value_type> values, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(values.begin(), values.end(), bucketCount, alloc)
	{
	}

	unordered_multimap(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const allocator_type& alloc = allocator_type())
		: unordered_multimap(values.begin(), values.end(), bucketCount, hashFunc, alloc)
	{
	}

	unordered_multimap(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalFunc,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(values.begin(), values.end(), bucketCount, hashFunc, equalFunc, alloc)
	{
	}

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

	unordered_multimap& operator=(std::initializer_list<value_type> values)
	{
		clear();	//?
		insert(values);
		return *this;
	}

	void swap(unordered_multimap& right) MOMO_NOEXCEPT
	{
		mHashMultiMap.Swap(right.mHashMultiMap);
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

	MOMO_FRIENDS_SWAP_BEGIN_END_STD(unordered_multimap)

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
	//typename std::enable_if<std::is_constructible<value_type, Value>::value, iterator>::type
	//insert(Value&& value)

	//template<typename Value>
	//typename std::enable_if<std::is_constructible<value_type, Value>::value, iterator>::type
	//insert(const_iterator hint, Value&& value)

	//iterator insert(const value_type& value)

	//iterator insert(const_iterator hint, const value_type& value)

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, const First&>::value
		&& std::is_constructible<mapped_type, const Second&>::value, iterator>::type
	insert(const std::pair<First, Second>& value)
	{
		return _insert(std::forward_as_tuple(value.first), std::forward_as_tuple(value.second));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, const First&>::value
		&& std::is_constructible<mapped_type, const Second&>::value, iterator>::type
	insert(const_iterator, const std::pair<First, Second>& value)
	{
		return insert(value);
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, First&&>::value
		&& std::is_constructible<mapped_type, Second&&>::value, iterator>::type
	insert(std::pair<First, Second>&& value)
	{
		return _insert(std::forward_as_tuple(std::forward<First>(value.first)),
			std::forward_as_tuple(std::forward<Second>(value.second)));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, First&&>::value
		&& std::is_constructible<mapped_type, Second&&>::value, iterator>::type
	insert(const_iterator, std::pair<First, Second>&& value)
	{
		return insert(std::move(value));
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

	iterator emplace()
	{
		return _insert(std::tuple<>(), std::tuple<>());
	}

	iterator emplace_hint(const_iterator)
	{
		return emplace();
	}

	template<typename ValueArg>
	iterator emplace(ValueArg&& valueArg)
	{
		return insert(std::forward<ValueArg>(valueArg));
	}

	template<typename ValueArg>
	iterator emplace_hint(const_iterator, ValueArg&& valueArg)
	{
		return emplace(std::forward<ValueArg>(valueArg));
	}

	template<typename KeyArg, typename MappedArg>
	iterator emplace(KeyArg&& keyArg, MappedArg&& mappedArg)
	{
		return _insert(std::forward_as_tuple(std::forward<KeyArg>(keyArg)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg)));
	}

	template<typename KeyArg, typename MappedArg>
	iterator emplace_hint(const_iterator, KeyArg&& keyArg, MappedArg&& mappedArg)
	{
		return emplace(std::forward<KeyArg>(keyArg), std::forward<MappedArg>(mappedArg));
	}

	template<typename... KeyArgs, typename... MappedArgs>
	iterator emplace(std::piecewise_construct_t,
		std::tuple<KeyArgs...> keyArgs, std::tuple<MappedArgs...> mappedArgs)
	{
		return _insert(std::move(keyArgs), std::move(mappedArgs));
	}

	template<typename... KeyArgs, typename... MappedArgs>
	iterator emplace_hint(const_iterator, std::piecewise_construct_t,
		std::tuple<KeyArgs...> keyArgs, std::tuple<MappedArgs...> mappedArgs)
	{
		return _insert(std::move(keyArgs), std::move(mappedArgs));
	}

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
		MOMO_ASSERT(count > 0);
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
		for (typename HashMultiMap::ConstKeyIterator::Reference ref : mHashMultiMap.GetKeyBounds())
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
	template<typename... KeyArgs, typename... MappedArgs>
	iterator _insert(std::tuple<KeyArgs...>&& keyArgs, std::tuple<MappedArgs...>&& mappedArgs)
	{
		typedef typename HashMultiMap::KeyValueTraits
			::template ValueCreator<MappedArgs...> MappedCreator;
		return _insert(std::move(keyArgs), MappedCreator(std::move(mappedArgs)));
	}

	template<typename... KeyArgs, typename MappedCreator>
	iterator _insert(std::tuple<KeyArgs...>&& keyArgs, const MappedCreator& mappedCreator)
	{
		typedef internal::ObjectManager<key_type> KeyManager;
		typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
		KeyBuffer keyBuffer;
		KeyCreator(std::move(keyArgs))(&keyBuffer);
		iterator resIter;
		try
		{
			resIter = _insert(std::forward_as_tuple(std::move(*&keyBuffer)), mappedCreator);
		}
		catch (...)
		{
			KeyManager::Destroy(*&keyBuffer);
			throw;
		}
		KeyManager::Destroy(*&keyBuffer);
		return resIter;
	}

	template<typename MappedCreator>
	iterator _insert(std::tuple<key_type&&>&& key, const MappedCreator& mappedCreator)
	{
		return iterator(mHashMultiMap.AddCrt(std::move(std::get<0>(key)), mappedCreator));
	}

	template<typename MappedCreator>
	iterator _insert(std::tuple<const key_type&>&& key, const MappedCreator& mappedCreator)
	{
		return iterator(mHashMultiMap.AddCrt(std::get<0>(key), mappedCreator));
	}

private:
	HashMultiMap mHashMultiMap;
};

template<typename TKey, typename TMapped,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_multimap_open = unordered_multimap<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
	HashMultiMap<TKey, TMapped, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketOneI1>,
		MemManagerStd<TAllocator>>>;

} // namespace stdish

} // namespace momo
