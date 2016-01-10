/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/stdish/unordered_map.h

  namespace momo::stdish:
    class unordered_map
    class unordered_map_open

  This classes are similar to `std::unordered_map`.
  `unordered_map` is much more efficient than standard one in
  memory usage. Its implementation is based on hash tables with
  buckets in the form of small arrays.
  `unordered_map_open` is based on open addressing hash table.

  Deviations from the `std::unordered_map`:
  1. Container items must be movable (preferably without exceptions)
    or copyable, similar to items of `std::vector`.
  2. After each addition or removal of the item all iterators and
    references to items become invalid and should not be used.
  3. Type `reference` is not the same as `value_type&`, so
    `for (auto& p : map)` is illegal, but `for (auto p : map)` or
    `for (const auto& p : map)` or `for (auto&& p : map)` is allowed.
  4.1. Container move constructor, move assignment operator and swap
    function do not throw exceptions regardless of the allocator.
  4.2. Functions of the allocator `construct`, `destruct` and `address`
    are not used.
  4.3. It is expected that the allocator types `pointer`, `const_pointer`,
    `reference`, `const_reference`, `size_type` and `difference_type`
    have the standard definition (as in `std::allocator`).
  4.4. It is expected that the allocator types `propagate_on_container_swap`
    and `propagate_on_container_move_assignment` are the same as
    `std::true_type`.

  It is allowed to pass to functions `insert` and `emplace` references
  to items within the container.

\**********************************************************/

#pragma once

#include "../HashMap.h"

namespace momo
{

namespace stdish
{

template<typename TKey, typename TMapped,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>,
	typename THashMap = HashMap<TKey, TMapped, HashTraitsStd<TKey, THashFunc, TEqualFunc>,
		MemManagerStd<TAllocator>>>
class unordered_map
{
private:
	typedef THashMap HashMap;
	typedef typename HashMap::HashTraits HashTraits;
	typedef typename HashMap::MemManager MemManager;

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
		typename HashMap::Iterator::Reference> reference;
	typedef typename reference::ConstReference const_reference;

	typedef internal::HashDerivedIterator<typename HashMap::Iterator, reference> iterator;
	typedef typename iterator::ConstIterator const_iterator;

	typedef typename iterator::Pointer pointer;
	typedef typename const_iterator::Pointer const_pointer;

	typedef internal::HashDerivedIterator<typename HashMap::BucketBounds::Iterator,
		reference> local_iterator;
	typedef typename local_iterator::ConstIterator const_local_iterator;

private:
	typedef internal::ObjectBuffer<key_type, HashMap::KeyValueTraits::keyAlignment> KeyBuffer;

public:
	unordered_map()
	{
	}

	explicit unordered_map(const allocator_type& alloc)
		: mHashMap(HashTraits(), MemManager(alloc))
	{
	}

	explicit unordered_map(size_type bucketCount)
		: mHashMap(HashTraits(bucketCount))
	{
	}

	unordered_map(size_type bucketCount, const hasher& hashFunc)
		: mHashMap(HashTraits(bucketCount, hashFunc))
	{
	}

	unordered_map(size_type bucketCount, const hasher& hashFunc, const key_equal& equalFunc)
		: mHashMap(HashTraits(bucketCount, hashFunc, equalFunc))
	{
	}

	unordered_map(size_type bucketCount, const hasher& hashFunc, const key_equal& equalFunc,
		const allocator_type& alloc)
		: mHashMap(HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
	}

	template<typename Iterator>
	unordered_map(Iterator first, Iterator last)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_map(Iterator first, Iterator last, size_type bucketCount)
		: mHashMap(HashTraits(bucketCount))
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_map(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc)
		: mHashMap(HashTraits(bucketCount, hashFunc))
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_map(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalFunc)
		: mHashMap(HashTraits(bucketCount, hashFunc, equalFunc))
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_map(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalFunc, const allocator_type& alloc)
		: mHashMap(HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
		insert(first, last);
	}

	unordered_map(std::initializer_list<value_type> values)
	{
		insert(values);
	}

	unordered_map(std::initializer_list<value_type> values, size_type bucketCount)
		: mHashMap(HashTraits(bucketCount))
	{
		insert(values);
	}

	unordered_map(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc)
		: mHashMap(HashTraits(bucketCount, hashFunc))
	{
		insert(values);
	}

	unordered_map(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalFunc)
		: mHashMap(HashTraits(bucketCount, hashFunc, equalFunc))
	{
		insert(values);
	}

	unordered_map(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalFunc, const allocator_type& alloc)
		: mHashMap(HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
		insert(values);
	}

	unordered_map(unordered_map&& right) MOMO_NOEXCEPT
		: mHashMap(std::move(right.mHashMap))
	{
	}

	unordered_map(unordered_map&& right, const allocator_type& alloc)
		: mHashMap(right.mHashMap.GetHashTraits(), MemManager(alloc))
	{
		if (right.get_allocator() == alloc)
			*this = std::move(right);
		else
			insert(right.begin(), right.end());
		right.clear();
	}

	unordered_map(const unordered_map& right)
		: mHashMap(right.mHashMap)
	{
	}

	unordered_map(const unordered_map& right, const allocator_type& alloc)
		: mHashMap(right.mHashMap.GetHashTraits(), MemManager(alloc))
	{
		insert(right.begin(), right.end());
	}

	~unordered_map() MOMO_NOEXCEPT
	{
	}

	unordered_map& operator=(unordered_map&& right) MOMO_NOEXCEPT
	{
		mHashMap = std::move(right.mHashMap);
		return *this;
	}

	unordered_map& operator=(const unordered_map& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_copy_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			unordered_map(right, alloc).swap(*this);
		}
		return *this;
	}

	unordered_map& operator=(std::initializer_list<value_type> values)
	{
		clear();	//?
		insert(values);
		return *this;
	}

	void swap(unordered_map& right) MOMO_NOEXCEPT
	{
		mHashMap.Swap(right.mHashMap);
	}

	friend void swap(unordered_map& left, unordered_map& right) MOMO_NOEXCEPT
	{
		left.swap(right);
	}

	iterator begin() MOMO_NOEXCEPT
	{
		return iterator(mHashMap.GetBegin());
	}

	const_iterator begin() const MOMO_NOEXCEPT
	{
		return const_iterator(mHashMap.GetBegin());
	}

	iterator end() MOMO_NOEXCEPT
	{
		return iterator(mHashMap.GetEnd());
	}

	const_iterator end() const MOMO_NOEXCEPT
	{
		return const_iterator(mHashMap.GetEnd());
	}

	const_iterator cbegin() const MOMO_NOEXCEPT
	{
		return begin();
	}

	const_iterator cend() const MOMO_NOEXCEPT
	{
		return end();
	}

	float max_load_factor() const MOMO_NOEXCEPT
	{
		return mHashMap.GetHashTraits().GetMaxLoadFactor();
	}

	void max_load_factor(float maxLoadFactor)
	{
		size_t logStartBucketCount = internal::UIntMath<size_t>::Log2(bucket_count());
		HashTraits hashTraits(hash_function(), key_eq(), logStartBucketCount, maxLoadFactor);
		HashMap hashMap(hashTraits, MemManager(get_allocator()));
		hashMap.Reserve(size());
		hashMap.InsertFS(begin(), end());
		mHashMap = std::move(hashMap);
	}

	hasher hash_function() const
	{
		return mHashMap.GetHashTraits().GetHashFunc();
	}

	key_equal key_eq() const
	{
		return mHashMap.GetHashTraits().GetEqualFunc();
	}

	allocator_type get_allocator() const //MOMO_NOEXCEPT
	{
		return mHashMap.GetMemManager().GetAllocator();
	}

	size_type max_size() const //MOMO_NOEXCEPT
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const MOMO_NOEXCEPT
	{
		return mHashMap.GetCount();
	}

	bool empty() const MOMO_NOEXCEPT
	{
		return mHashMap.IsEmpty();
	}

	void clear() MOMO_NOEXCEPT
	{
		mHashMap.Clear();
	}

	void rehash(size_type bucketCount)
	{
		bucketCount = std::minmax(bucketCount, (size_t)2).second;
		size_t logBucketCount = internal::UIntMath<size_t>::Log2(bucketCount - 1) + 1;
		bucketCount = (size_t)1 << logBucketCount;
		reserve(mHashMap.GetHashTraits().CalcCapacity(bucketCount));
	}

	void reserve(size_type count)
	{
		mHashMap.Reserve(count);
	}

	const_iterator find(const key_type& key) const
	{
		return const_iterator(mHashMap.Find(key));
	}

	iterator find(const key_type& key)
	{
		return iterator(mHashMap.Find(key));
	}

	size_type count(const key_type& key) const
	{
		return mHashMap.HasKey(key) ? 1 : 0;
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		const_iterator iter = find(key);
		if (iter == end())
			return std::pair<const_iterator, const_iterator>(end(), end());
		return std::pair<const_iterator, const_iterator>(iter, std::next(iter));
	}

	std::pair<iterator, iterator> equal_range(const key_type& key)
	{
		iterator iter = find(key);
		if (iter == end())
			return std::pair<iterator, iterator>(end(), end());
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
		typedef typename HashMap::KeyValueTraits::template ValueCreator<const Second&> MappedCreator;
		return _insert(nullptr, value.first, MappedCreator(value.second));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<const First&, key_type>::value
		&& std::is_convertible<const Second&, mapped_type>::value, iterator>::type
	insert(const_iterator hint, const std::pair<First, Second>& value)
	{
		typedef typename HashMap::KeyValueTraits::template ValueCreator<const Second&> MappedCreator;
		return _insert(hint, value.first, MappedCreator(value.second)).first;
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<First, key_type>::value
		&& std::is_convertible<Second, mapped_type>::value,
		std::pair<iterator, bool>>::type
	insert(std::pair<First, Second>&& value)
	{
		typedef typename HashMap::KeyValueTraits::template ValueCreator<Second> MappedCreator;
		return _insert(nullptr, std::forward<First>(value.first),
			MappedCreator(std::forward<Second>(value.second)));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<First, key_type>::value
		&& std::is_convertible<Second, mapped_type>::value, iterator>::type
	insert(const_iterator hint, std::pair<First, Second>&& value)
	{
		typedef typename HashMap::KeyValueTraits::template ValueCreator<Second> MappedCreator;
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
		typedef typename HashMap::KeyValueTraits::template ValueCreator<Arg2> MappedCreator;
		return _insert(nullptr, std::forward<Arg1>(arg1), MappedCreator(std::forward<Arg2>(arg2)));
	}

	template<typename Arg1, typename Arg2>
	iterator emplace_hint(const_iterator hint, Arg1&& arg1, Arg2&& arg2)
	{
		typedef typename HashMap::KeyValueTraits::template ValueCreator<Arg2> MappedCreator;
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
		return iterator(mHashMap.Remove(where.GetBaseIterator()));
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		if (first == end())
			return end();
		if (first == last)
			return find(first->first);
		if (first == begin() && last == end())
		{
			clear();
			return end();
		}
		if (std::next(first) == last)
			return erase(first);
		throw std::invalid_argument("invalid unordered_map erase arguments");
	}

	size_type erase(const key_type& key)
	{
		return mHashMap.Remove(key) ? 1 : 0;
	}

	typename HashMap::ValueReferenceRKey operator[](key_type&& key)
	{
		return mHashMap[std::move(key)];
	}

	typename HashMap::ValueReferenceCKey operator[](const key_type& key)
	{
		return mHashMap[key];
	}

	const mapped_type& at(const key_type& key) const
	{
		const_iterator iter = find(key);
		if (iter == end())
			throw std::out_of_range("invalid unordered_map key");
		return iter->second;
	}

	mapped_type& at(const key_type& key)
	{
		iterator iter = find(key);
		if (iter == end())
			throw std::out_of_range("invalid unordered_map key");
		return iter->second;
	}

	size_type max_bucket_count() const MOMO_NOEXCEPT
	{
		return SIZE_MAX;
		//return internal::HashSetBuckets<Bucket>::maxBucketCount;
	}

	size_type bucket_count() const MOMO_NOEXCEPT
	{
		return mHashMap.GetBucketCount();
	}

	size_type bucket_size(size_type bucketIndex) const
	{
		return mHashMap.GetBucketBounds(bucketIndex).GetCount();
	}

	local_iterator begin(size_type bucketIndex)
	{
		return local_iterator(mHashMap.GetBucketBounds(bucketIndex).GetBegin());
	}

	const_local_iterator begin(size_type bucketIndex) const
	{
		return const_local_iterator(mHashMap.GetBucketBounds(bucketIndex).GetBegin());
	}

	local_iterator end(size_type bucketIndex)
	{
		return local_iterator(mHashMap.GetBucketBounds(bucketIndex).GetEnd());
	}

	const_local_iterator end(size_type bucketIndex) const
	{
		return const_local_iterator(mHashMap.GetBucketBounds(bucketIndex).GetEnd());
	}

	const_local_iterator cbegin(size_type bucketIndex) const
	{
		return begin(bucketIndex);
	}

	const_local_iterator cend(size_type bucketIndex) const
	{
		return end(bucketIndex);
	}

	size_type bucket(const key_type& key) const
	{
		return mHashMap.GetBucketIndex(key);
	}

	float load_factor() const MOMO_NOEXCEPT
	{
		return (float)size() / (float)bucket_count();
	}

	bool operator==(const unordered_map& right) const
	{
		if (size() != right.size())
			return false;
		for (const_reference ref : *this)
		{
			const_iterator iter = right.find(ref.first);
			if (iter == right.end())
				return false;
			if (!(iter->second == ref.second))
				return false;
		}
		return true;
	}

	bool operator!=(const unordered_map& right) const
	{
		return !(*this == right);
	}

private:
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
		typename HashMap::InsertResult res = mHashMap.InsertCrt(std::move(key), mappedCreator);
		return std::pair<iterator, bool>(iterator(res.iterator), res.inserted);
	}

	template<typename MappedCreator>
	std::pair<iterator, bool> _insert(const_iterator hint, key_type&& key,
		const MappedCreator& mappedCreator)
	{
#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
		typename HashMap::Iterator resIter = mHashMap.AddCrt(hint.GetBaseIterator(),
			std::move(key), mappedCreator);
		return std::pair<iterator, bool>(iterator(resIter), true);
#else
		(void)hint;
		return _insert(nullptr, std::move(key), mappedCreator);
#endif
	}

	template<typename MappedCreator>
	std::pair<iterator, bool> _insert(std::nullptr_t, const key_type& key,
		const MappedCreator& mappedCreator)
	{
		typename HashMap::InsertResult res = mHashMap.InsertCrt(key, mappedCreator);
		return std::pair<iterator, bool>(iterator(res.iterator), res.inserted);
	}

	template<typename MappedCreator>
	std::pair<iterator, bool> _insert(const_iterator hint, const key_type& key,
		const MappedCreator& mappedCreator)
	{
#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
		typename HashMap::Iterator resIter = mHashMap.AddCrt(hint.GetBaseIterator(),
			key, mappedCreator);
		return std::pair<iterator, bool>(iterator(resIter), true);
#else
		(void)hint;
		return _insert(nullptr, key, mappedCreator);
#endif
	}

	template<typename Hint, typename... Args1, typename... Args2>
	std::pair<iterator, bool> _emplace(Hint hint, std::tuple<Args1...>&& args1,
		std::tuple<Args2...>&& args2)
	{
		typedef internal::ObjectManager<key_type> KeyManager;
		typedef typename KeyManager::template Creator<Args1...> KeyCreator;
		typedef typename HashMap::KeyValueTraits::template ValueCreator<Args2...> MappedCreator;
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
	HashMap mHashMap;
};

template<typename TKey, typename TMapped,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_map_open = unordered_map<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
	HashMap<TKey, TMapped, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketOneI1>,
		MemManagerStd<TAllocator>>>;

} // namespace stdish

} // namespace momo
