/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/stdish/unordered_multimap.h

  namespace momo::stdish:
    class unordered_multimap
    class unordered_multimap_open

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_STDISH_UNORDERED_MULTIMAP
#define MOMO_INCLUDE_GUARD_STDISH_UNORDERED_MULTIMAP

#include "../HashMultiMap.h"
#include "set_map_utility.h"

namespace momo
{

namespace stdish
{

/*!
	\brief
	`momo::stdish::unordered_multimap` is similar to `std::unordered_multimap`,
	but much more efficient in memory usage. The implementation is based on
	hash table with buckets in the form of small arrays.

	\details
	Deviations from the `std::unordered_multimap`:
	1. Each of duplicate keys stored only once.
	2. `max_load_factor`, `rehash`, `reserve`, `load_factor` and all
	the functions, associated with buckets or nodes, are not implemented.
	3. Container items must be movable (preferably without exceptions)
	or copyable, similar to items of `std::vector`.
	4. After each addition or removal of the item all iterators and
	references to items become invalid and should not be used.
	5. Type `reference` is not the same as `value_type&`, so
	`for (auto& p : map)` is illegal, but `for (auto p : map)` or
	`for (const auto& p : map)` or `for (auto&& p : map)` is allowed.
	6. Functions `clear`, `begin`, `cbegin` and iterator increment take
	O(bucket_count) time in worst case.
	7. Functions `erase` can throw exceptions thrown by `key_type` and
	`mapped_type` move assignment operators.

	It is allowed to pass to functions `insert` and `emplace` references
	to items within the container.
	But in case of the function `insert`, receiving pair of iterators, it's
	not allowed to pass iterators pointing to the items within the container.
*/

template<typename TKey, typename TMapped,
	typename THasher = HashCoder<TKey>,
	typename TEqualComparer = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>,
	typename THashMultiMap = HashMultiMap<TKey, TMapped, HashTraitsStd<TKey, THasher, TEqualComparer>,
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
	typedef THasher hasher;
	typedef TEqualComparer key_equal;
	typedef TAllocator allocator_type;

	typedef HashMultiMap nested_container_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef std::pair<const key_type, mapped_type> value_type;

	typedef momo::internal::HashDerivedIterator<typename HashMultiMap::Iterator,
		momo::internal::MapReferenceStd> iterator;
	typedef typename iterator::ConstIterator const_iterator;

	typedef typename iterator::Reference reference;
	typedef typename const_iterator::Reference const_reference;

	typedef typename iterator::Pointer pointer;
	typedef typename const_iterator::Pointer const_pointer;
	//typedef typename std::allocator_traits<allocator_type>::pointer pointer;
	//typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;

	//node_type;

	//local_iterator;
	//const_local_iterator;

private:
	template<typename KeyArg>
	struct IsValidKeyArg : public HashTraits::template IsValidKeyArg<KeyArg>
	{
	};

	struct ConstIteratorProxy : public const_iterator
	{
		typedef const_iterator ConstIterator;
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBaseIterator,
			typename ConstIterator::BaseIterator)
	};

	struct IteratorProxy : public iterator
	{
		typedef iterator Iterator;
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

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

	unordered_multimap(size_type bucketCount, const hasher& hashFunc, const key_equal& equalComp,
		const allocator_type& alloc = allocator_type())
		: mHashMultiMap(HashTraits(bucketCount, hashFunc, equalComp), MemManager(alloc))
	{
	}

	template<typename Iterator>
	unordered_multimap(Iterator first, Iterator last)
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
		const hasher& hashFunc, const key_equal& equalComp,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(bucketCount, hashFunc, equalComp, alloc)
	{
		insert(first, last);
	}

	unordered_multimap(std::initializer_list<momo::internal::Identity<value_type>> values)
		: unordered_multimap(values.begin(), values.end())
	{
	}

	unordered_multimap(std::initializer_list<momo::internal::Identity<value_type>> values,
		size_type bucketCount, const allocator_type& alloc = allocator_type())
		: unordered_multimap(values.begin(), values.end(), bucketCount, alloc)
	{
	}

	unordered_multimap(std::initializer_list<momo::internal::Identity<value_type>> values,
		size_type bucketCount, const hasher& hashFunc, const allocator_type& alloc = allocator_type())
		: unordered_multimap(values.begin(), values.end(), bucketCount, hashFunc, alloc)
	{
	}

	unordered_multimap(std::initializer_list<momo::internal::Identity<value_type>> values,
		size_type bucketCount, const hasher& hashFunc, const key_equal& equalComp,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(values.begin(), values.end(), bucketCount, hashFunc, equalComp, alloc)
	{
	}

#ifdef MOMO_HAS_CONTAINERS_RANGES
	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_multimap(std::from_range_t, Range&& values)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_multimap(std::from_range_t, Range&& values, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(bucketCount, alloc)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_multimap(std::from_range_t, Range&& values, size_type bucketCount,
		const hasher& hashFunc, const allocator_type& alloc = allocator_type())
		: unordered_multimap(bucketCount, hashFunc, alloc)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_multimap(std::from_range_t, Range&& values, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalComp,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(bucketCount, hashFunc, equalComp, alloc)
	{
		insert_range(std::forward<Range>(values));
	}
#endif // MOMO_HAS_CONTAINERS_RANGES

	unordered_multimap(unordered_multimap&& right) noexcept
		: mHashMultiMap(std::move(right.mHashMultiMap))
	{
	}

	unordered_multimap(unordered_multimap&& right,
		const momo::internal::Identity<allocator_type>& alloc)
		noexcept(std::is_empty<allocator_type>::value)
		: mHashMultiMap(pvCreateMultiMap(std::move(right), alloc))
	{
	}

	unordered_multimap(const unordered_multimap& right)
		: mHashMultiMap(right.mHashMultiMap)
	{
	}

	unordered_multimap(const unordered_multimap& right,
		const momo::internal::Identity<allocator_type>& alloc)
		: mHashMultiMap(right.mHashMultiMap, MemManager(alloc))
	{
	}

	~unordered_multimap() = default;

	unordered_multimap& operator=(unordered_multimap&& right)
		noexcept(std::is_empty<allocator_type>::value ||
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
	{
		if (this != &right)
		{
			bool propagate = std::is_empty<allocator_type>::value ||
				std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value;
			allocator_type alloc = (propagate ? &right : this)->get_allocator();
			mHashMultiMap = pvCreateMultiMap(std::move(right), alloc);
		}
		return *this;
	}

	unordered_multimap& operator=(const unordered_multimap& right)
	{
		if (this != &right)
		{
			bool propagate = std::is_empty<allocator_type>::value ||
				std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value;
			allocator_type alloc = (propagate ? &right : this)->get_allocator();
			mHashMultiMap = HashMultiMap(right.mHashMultiMap, MemManager(alloc));
		}
		return *this;
	}

	unordered_multimap& operator=(std::initializer_list<value_type> values)
	{
		HashMultiMap hashMultiMap(mHashMultiMap.GetHashTraits(), MemManager(get_allocator()));
		hashMultiMap.Add(values.begin(), values.end());
		mHashMultiMap = std::move(hashMultiMap);
		return *this;
	}

	void swap(unordered_multimap& right) noexcept
	{
		MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
			|| get_allocator() == right.get_allocator());
		mHashMultiMap.Swap(right.mHashMultiMap);
	}

	friend void swap(unordered_multimap& left, unordered_multimap& right) noexcept
	{
		left.swap(right);
	}

	const nested_container_type& get_nested_container() const noexcept
	{
		return mHashMultiMap;
	}

	nested_container_type& get_nested_container() noexcept
	{
		return mHashMultiMap;
	}

	const_iterator begin() const noexcept
	{
		return ConstIteratorProxy(mHashMultiMap.GetBegin());
	}

	iterator begin() noexcept
	{
		return IteratorProxy(mHashMultiMap.GetBegin());
	}

	const_iterator end() const noexcept
	{
		return ConstIteratorProxy(mHashMultiMap.GetEnd());
	}

	iterator end() noexcept
	{
		return IteratorProxy(mHashMultiMap.GetEnd());
	}

	const_iterator cbegin() const noexcept
	{
		return begin();
	}

	const_iterator cend() const noexcept
	{
		return end();
	}

	//float max_load_factor() const noexcept
	//void max_load_factor(float maxLoadFactor)

	hasher hash_function() const
	{
		return mHashMultiMap.GetHashTraits().GetHasher();
	}

	key_equal key_eq() const
	{
		return mHashMultiMap.GetHashTraits().GetEqualComparer();
	}

	allocator_type get_allocator() const noexcept
	{
		return allocator_type(mHashMultiMap.GetMemManager().GetByteAllocator());
	}

	size_type max_size() const noexcept
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const noexcept
	{
		return mHashMultiMap.GetCount();
	}

	MOMO_NODISCARD bool empty() const noexcept
	{
		return mHashMultiMap.IsEmpty();
	}

	void clear() noexcept
	{
		mHashMultiMap.Clear();
	}

	//void rehash(size_type bucketCount)
	//void reserve(size_type count)

	MOMO_FORCEINLINE const_iterator find(const key_type& key) const
	{
		return equal_range(key).first;
	}

	MOMO_FORCEINLINE iterator find(const key_type& key)
	{
		return equal_range(key).first;
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	const_iterator> find(const KeyArg& key) const
	{
		return equal_range(key).first;
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	iterator> find(const KeyArg& key)
	{
		return equal_range(key).first;
	}

	MOMO_FORCEINLINE size_type count(const key_type& key) const
	{
		typename HashMultiMap::ConstKeyIterator keyIter = mHashMultiMap.Find(key);
		return !!keyIter ? keyIter->GetCount() : 0;
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	size_type> count(const KeyArg& key) const
	{
		typename HashMultiMap::ConstKeyIterator keyIter = mHashMultiMap.Find(key);
		return !!keyIter ? keyIter->GetCount() : 0;
	}

	MOMO_FORCEINLINE bool contains(const key_type& key) const
	{
		return count(key) > 0;
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	bool> contains(const KeyArg& key) const
	{
		return count(key) > 0;
	}

	MOMO_FORCEINLINE std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		return pvEqualRange<const_iterator, ConstIteratorProxy>(mHashMultiMap,
			mHashMultiMap.Find(key));
	}

	MOMO_FORCEINLINE std::pair<iterator, iterator> equal_range(const key_type& key)
	{
		return pvEqualRange<iterator, IteratorProxy>(mHashMultiMap, mHashMultiMap.Find(key));
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	std::pair<const_iterator, const_iterator>> equal_range(const KeyArg& key) const
	{
		return pvEqualRange<const_iterator, ConstIteratorProxy>(mHashMultiMap,
			mHashMultiMap.Find(key));
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	std::pair<iterator, iterator>> equal_range(const KeyArg& key)
	{
		return pvEqualRange<iterator, IteratorProxy>(mHashMultiMap, mHashMultiMap.Find(key));
	}

	template<typename ValueArg = std::pair<key_type, mapped_type>>
	momo::internal::EnableIf<std::is_constructible<value_type, ValueArg&&>::value,
	iterator> insert(ValueArg&& valueArg)
	{
		return emplace(std::forward<ValueArg>(valueArg));
	}

	template<typename ValueArg = std::pair<key_type, mapped_type>>
	momo::internal::EnableIf<std::is_constructible<value_type, ValueArg&&>::value,
	iterator> insert(const_iterator, ValueArg&& valueArg)
	{
		return insert(std::forward<ValueArg>(valueArg));
	}

	template<typename Iterator>
	void insert(Iterator first, Iterator last)
	{
		pvInsertRange(first, last);
	}

	void insert(std::initializer_list<value_type> values)
	{
		mHashMultiMap.Add(values.begin(), values.end());
	}

#ifdef MOMO_HAS_CONTAINERS_RANGES
	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	void insert_range(Range&& values)
	{
		pvInsertRange(std::ranges::begin(values), std::ranges::end(values));
	}
#endif // MOMO_HAS_CONTAINERS_RANGES

	iterator emplace()
	{
		return pvEmplace(std::tuple<>(), std::tuple<>());
	}

	iterator emplace_hint(const_iterator)
	{
		return emplace();
	}

	template<typename ValueArg>
	iterator emplace(ValueArg&& valueArg)
	{
		return pvEmplace(std::forward_as_tuple(std::get<0>(std::forward<ValueArg>(valueArg))),
			std::forward_as_tuple(std::get<1>(std::forward<ValueArg>(valueArg))));
	}

	template<typename ValueArg>
	iterator emplace_hint(const_iterator, ValueArg&& valueArg)
	{
		return emplace(std::forward<ValueArg>(valueArg));
	}

	template<typename KeyArg, typename MappedArg>
	iterator emplace(KeyArg&& keyArg, MappedArg&& mappedArg)
	{
		return pvEmplace(std::forward_as_tuple(std::forward<KeyArg>(keyArg)),
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
		return pvEmplace(std::move(keyArgs), std::move(mappedArgs));
	}

	template<typename... KeyArgs, typename... MappedArgs>
	iterator emplace_hint(const_iterator, std::piecewise_construct_t,
		std::tuple<KeyArgs...> keyArgs, std::tuple<MappedArgs...> mappedArgs)
	{
		return pvEmplace(std::move(keyArgs), std::move(mappedArgs));
	}

	iterator erase(const_iterator where)
	{
		typename HashMultiMap::ConstIterator iter = ConstIteratorProxy::GetBaseIterator(where);
		typename HashMultiMap::ConstKeyIterator keyIter = iter.GetKeyIterator();
		if (keyIter->GetCount() == 1)
			return IteratorProxy(mHashMultiMap.MakeIterator(mHashMultiMap.RemoveKey(keyIter)));
		else
			return IteratorProxy(mHashMultiMap.Remove(iter));
	}

	iterator erase(iterator where)
	{
		return erase(static_cast<const_iterator>(where));
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		if (first == begin() && last == end())
		{
			clear();
			return end();
		}
		if (first == last)
		{
			return IteratorProxy(mHashMultiMap.MakeMutableIterator(
				ConstIteratorProxy::GetBaseIterator(first)));
		}
		if (first != end())
		{
			if (std::next(first) == last)
				return erase(first);
			typename HashMultiMap::ConstKeyIterator keyIter =
				ConstIteratorProxy::GetBaseIterator(first).GetKeyIterator();
			if (last == ConstIteratorProxy(mHashMultiMap.MakeIterator(keyIter, keyIter->GetCount())))
				return IteratorProxy(mHashMultiMap.MakeIterator(mHashMultiMap.RemoveKey(keyIter)));
		}
		throw std::invalid_argument("invalid unordered_multimap erase arguments");
	}

	size_type erase(const key_type& key)
	{
		return mHashMultiMap.RemoveKey(key);
	}

	template<typename ValueFilter>
	friend size_type erase_if(unordered_multimap& cont, const ValueFilter& valueFilter)
	{
		auto pairFilter = [&valueFilter] (const key_type& key, const mapped_type& mapped)
			{ return valueFilter(const_reference(key, mapped)); };
		return cont.mHashMultiMap.Remove(pairFilter);
	}

	//iterator insert(node_type&& node)
	//iterator insert(const_iterator, node_type&& node)
	//node_type extract(const_iterator where)
	//node_type extract(const key_type& key)
	//void merge(...)

	//size_type max_bucket_count() const noexcept
	//size_type bucket_count() const noexcept
	//size_type bucket_size(size_type bucketIndex) const
	//local_iterator begin(size_type bucketIndex)
	//const_local_iterator begin(size_type bucketIndex) const
	//local_iterator end(size_type bucketIndex)
	//const_local_iterator end(size_type bucketIndex) const
	//const_local_iterator cbegin(size_type bucketIndex) const
	//const_local_iterator cend(size_type bucketIndex) const
	//size_type bucket(const key_type& key) const
	//float load_factor() const noexcept

	friend bool operator==(const unordered_multimap& left, const unordered_multimap& right)
	{
		if (left.mHashMultiMap.GetKeyCount() != right.mHashMultiMap.GetKeyCount())
			return false;
		if (left.mHashMultiMap.GetCount() != right.mHashMultiMap.GetCount())
			return false;
		typedef typename HashMultiMap::ConstKeyIterator ConstKeyIterator;
		for (typename ConstKeyIterator::Reference ref : left.mHashMultiMap.GetKeyBounds())
		{
			if (ref.GetCount() == 0)
				continue;
			ConstKeyIterator rightKeyIter = right.mHashMultiMap.Find(ref.key);
			if (!rightKeyIter)
				return false;
			if (ref.GetCount() != rightKeyIter->GetCount())
				return false;
			if (!std::is_permutation(ref.GetBegin(), ref.GetEnd(), rightKeyIter->GetBegin()))
				return false;
		}
		return true;
	}

	friend bool operator!=(const unordered_multimap& left, const unordered_multimap& right)
	{
		return !(left == right);
	}

private:
	static HashMultiMap pvCreateMultiMap(unordered_multimap&& right, const allocator_type& alloc)
	{
		if (right.get_allocator() == alloc)
			return std::move(right.mHashMultiMap);
		HashMultiMap hashMultiMap(right.mHashMultiMap.GetHashTraits(), MemManager(alloc));
		for (reference ref : right)
			hashMultiMap.Add(ref.first, std::move(ref.second));
		right.clear();
		return hashMultiMap;
	}

	template<typename Iterator, typename IteratorProxy, typename HashMultiMap, typename KeyIterator>
	static std::pair<Iterator, Iterator> pvEqualRange(HashMultiMap& hashMultiMap,
		KeyIterator keyIter)
	{
		Iterator end = IteratorProxy(hashMultiMap.GetEnd());
		if (!keyIter)
			return { end, end };
		size_t count = keyIter->GetCount();
		if (count == 0)
			return { end, end };
		Iterator first = IteratorProxy(hashMultiMap.MakeIterator(keyIter, 0));
		Iterator last = IteratorProxy(hashMultiMap.MakeIterator(keyIter, count));
		return { first, last };
	}

	template<typename... KeyArgs, typename... MappedArgs>
	iterator pvEmplace(std::tuple<KeyArgs...>&& keyArgs, std::tuple<MappedArgs...>&& mappedArgs)
	{
		typedef typename HashMultiMap::KeyValueTraits
			::template ValueCreator<MappedArgs...> MappedCreator;
		return pvInsert(std::move(keyArgs),
			MappedCreator(mHashMultiMap.GetMemManager(), std::move(mappedArgs)));
	}

	template<typename... KeyArgs, typename MappedCreator>
	iterator pvInsert(std::tuple<KeyArgs...>&& keyArgs, MappedCreator&& mappedCreator)
	{
		MemManager& memManager = mHashMultiMap.GetMemManager();
		typedef momo::internal::ObjectBuffer<key_type, HashMultiMap::KeyValueTraits::keyAlignment> KeyBuffer;
		typedef momo::internal::ObjectManager<key_type, MemManager> KeyManager;
		typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
		KeyBuffer keyBuffer;
		KeyCreator(memManager, std::move(keyArgs))(keyBuffer.GetPtr());
		iterator resIter;
		try
		{
			resIter = pvInsert(std::forward_as_tuple(std::move(keyBuffer.Get())),
				std::forward<MappedCreator>(mappedCreator));
		}
		catch (...)
		{
			KeyManager::Destroy(memManager, keyBuffer.Get());
			throw;
		}
		KeyManager::Destroy(memManager, keyBuffer.Get());
		return resIter;
	}

	template<typename RKey, typename MappedCreator,
		typename Key = typename std::decay<RKey>::type>
	momo::internal::EnableIf<std::is_same<key_type, Key>::value,
	iterator> pvInsert(std::tuple<RKey>&& key, MappedCreator&& mappedCreator)
	{
		return IteratorProxy(mHashMultiMap.AddCrt(
			std::forward<RKey>(std::get<0>(key)), std::forward<MappedCreator>(mappedCreator)));
	}

	template<typename Iterator, typename Sentinel>
	momo::internal::EnableIf<momo::internal::IsMapArgIteratorStd<Iterator, key_type>::value,
	void> pvInsertRange(Iterator begin, Sentinel end)
	{
		mHashMultiMap.Add(std::move(begin), std::move(end));
	}

	template<typename Iterator, typename Sentinel>
	momo::internal::EnableIf<!momo::internal::IsMapArgIteratorStd<Iterator, key_type>::value,
	void> pvInsertRange(Iterator begin, Sentinel end)
	{
		for (Iterator iter = std::move(begin); iter != end; ++iter)
			insert(*iter);
	}

private:
	HashMultiMap mHashMultiMap;
};

/*!
	\brief
	`momo::stdish::unordered_multimap_open` is similar to `std::unordered_multimap`,
	but much more efficient in operation speed. The implementation is based
	on open addressing hash table.

	\copydetails momo::stdish::unordered_multimap
*/

template<typename TKey, typename TMapped,
	typename THasher = HashCoder<TKey>,
	typename TEqualComparer = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
class unordered_multimap_open : public unordered_multimap<TKey, TMapped, THasher, TEqualComparer, TAllocator,
	HashMultiMap<TKey, TMapped, HashTraitsStd<TKey, THasher, TEqualComparer, HashBucketOpenDefault>,
		MemManagerStd<TAllocator>>>
{
private:
	typedef unordered_multimap<TKey, TMapped, THasher, TEqualComparer, TAllocator,
		momo::HashMultiMap<TKey, TMapped, HashTraitsStd<TKey, THasher, TEqualComparer, HashBucketOpenDefault>,
		MemManagerStd<TAllocator>>> UnorderedMultiMap;

public:
	using typename UnorderedMultiMap::key_type;
	using typename UnorderedMultiMap::mapped_type;
	using typename UnorderedMultiMap::size_type;
	using typename UnorderedMultiMap::value_type;
	using typename UnorderedMultiMap::const_reference;

public:
	using UnorderedMultiMap::UnorderedMultiMap;

	unordered_multimap_open() {}	// clang 3.6

	unordered_multimap_open& operator=(std::initializer_list<value_type> values)
	{
		UnorderedMultiMap::operator=(values);
		return *this;
	}

	friend void swap(unordered_multimap_open& left, unordered_multimap_open& right) noexcept
	{
		left.swap(right);
	}

	template<typename ValueFilter>
	friend size_type erase_if(unordered_multimap_open& cont, const ValueFilter& valueFilter)
	{
		auto pairFilter = [&valueFilter] (const key_type& key, const mapped_type& mapped)
			{ return valueFilter(const_reference(key, mapped)); };
		return cont.get_nested_container().Remove(pairFilter);
	}
};

#ifdef MOMO_HAS_DEDUCTION_GUIDES

#define MOMO_DECLARE_DEDUCTION_GUIDES(unordered_multimap) \
template<typename Iterator, \
	typename Value = typename std::iterator_traits<Iterator>::value_type, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>> \
unordered_multimap(Iterator, Iterator) \
	-> unordered_multimap<Key, Mapped>; \
template<typename Iterator, \
	typename Value = typename std::iterator_traits<Iterator>::value_type, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, HashCoder<Key>>> \
unordered_multimap(Iterator, Iterator, size_t, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename Iterator, typename Hasher, \
	typename Value = typename std::iterator_traits<Iterator>::value_type, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, Hasher>> \
unordered_multimap(Iterator, Iterator, size_t, Hasher, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, Hasher, std::equal_to<Key>, Allocator>; \
template<typename Iterator, typename Hasher, typename EqualComparer, \
	typename Value = typename std::iterator_traits<Iterator>::value_type, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, Hasher, EqualComparer>> \
unordered_multimap(Iterator, Iterator, size_t, Hasher, EqualComparer, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, Hasher, EqualComparer, Allocator>; \
template<typename QKey, typename Mapped, \
	typename Key = std::remove_const_t<QKey>> \
unordered_multimap(std::initializer_list<std::pair<QKey, Mapped>>) \
	-> unordered_multimap<Key, Mapped>; \
template<typename QKey, typename Mapped, \
	typename Key = std::remove_const_t<QKey>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, HashCoder<Key>>> \
unordered_multimap(std::initializer_list<std::pair<QKey, Mapped>>, size_t, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename QKey, typename Mapped, typename Hasher, \
	typename Key = std::remove_const_t<QKey>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, Hasher>> \
unordered_multimap(std::initializer_list<std::pair<QKey, Mapped>>, size_t, Hasher, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, Hasher, std::equal_to<Key>, Allocator>; \
template<typename QKey, typename Mapped, typename Hasher, typename EqualComparer, \
	typename Key = std::remove_const_t<QKey>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, Hasher, EqualComparer>> \
unordered_multimap(std::initializer_list<std::pair<QKey, Mapped>>, size_t, Hasher, EqualComparer, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, Hasher, EqualComparer, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES(unordered_multimap)
MOMO_DECLARE_DEDUCTION_GUIDES(unordered_multimap_open)

#undef MOMO_DECLARE_DEDUCTION_GUIDES

#ifdef MOMO_HAS_CONTAINERS_RANGES

#define MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(unordered_multimap) \
template<std::ranges::input_range Range, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>> \
unordered_multimap(std::from_range_t, Range&&) \
	-> unordered_multimap<Key, Mapped>; \
template<std::ranges::input_range Range, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, HashCoder<Key>>> \
unordered_multimap(std::from_range_t, Range&&, size_t, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<std::ranges::input_range Range, typename Hasher, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, Hasher>> \
unordered_multimap(std::from_range_t, Range&&, size_t, Hasher, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, Hasher, std::equal_to<Key>, Allocator>; \
template<std::ranges::input_range Range, typename Hasher, typename EqualComparer, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, Hasher, EqualComparer>> \
unordered_multimap(std::from_range_t, Range&&, size_t, Hasher, EqualComparer, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, Hasher, EqualComparer, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(unordered_multimap)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(unordered_multimap_open)

#undef MOMO_DECLARE_DEDUCTION_GUIDES_RANGES

#endif // MOMO_HAS_CONTAINERS_RANGES

#endif // MOMO_HAS_DEDUCTION_GUIDES

} // namespace stdish

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_STDISH_UNORDERED_MULTIMAP
