/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/stdish/unordered_multimap.h

  namespace momo::stdish:
    class unordered_multimap
    class unordered_multimap_open

\**********************************************************/

#pragma once

#include "../HashMultiMap.h"

namespace momo::stdish
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

	typedef momo::internal::DerivedForwardIterator<typename HashMultiMap::Iterator,
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
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBaseIterator)
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

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	unordered_multimap(Iterator first, Iterator last)
	{
		insert(first, last);
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	unordered_multimap(Iterator first, Iterator last, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(bucketCount, alloc)
	{
		insert(first, last);
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	unordered_multimap(Iterator first, Iterator last, size_type bucketCount,
		const hasher& hashFunc, const allocator_type& alloc = allocator_type())
		: unordered_multimap(bucketCount, hashFunc, alloc)
	{
		insert(first, last);
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	unordered_multimap(Iterator first, Iterator last, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalComp,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(bucketCount, hashFunc, equalComp, alloc)
	{
		insert(first, last);
	}

	unordered_multimap(std::initializer_list<value_type> values)
		: unordered_multimap(values.begin(), values.end())
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
		const hasher& hashFunc, const key_equal& equalComp,
		const allocator_type& alloc = allocator_type())
		: unordered_multimap(values.begin(), values.end(), bucketCount, hashFunc, equalComp, alloc)
	{
	}

#if defined(__cpp_lib_containers_ranges)
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
#endif // __cpp_lib_containers_ranges

	unordered_multimap(unordered_multimap&& right)
		: unordered_multimap(std::move(right), right.get_allocator())
	{
	}

	unordered_multimap(unordered_multimap&& right,
		const std::type_identity_t<allocator_type>& alloc)
		: mHashMultiMap(right.mHashMultiMap.GetHashTraits(), MemManager(alloc))
	{
		if (right.get_allocator() == alloc)
		{
			mHashMultiMap.Swap(right.mHashMultiMap);
		}
		else
		{
			for (reference ref : right)
				mHashMultiMap.Add(ref.first, std::move(ref.second));
			right.clear();
		}
	}

	unordered_multimap(const unordered_multimap& right)
		: mHashMultiMap(right.mHashMultiMap)
	{
	}

	unordered_multimap(const unordered_multimap& right,
		const std::type_identity_t<allocator_type>& alloc)
		: mHashMultiMap(right.mHashMultiMap, MemManager(alloc))
	{
	}

	~unordered_multimap() noexcept = default;

	unordered_multimap& operator=(unordered_multimap&& right)
		noexcept(momo::internal::ContainerAssignerStd::isNothrowMoveAssignable<unordered_multimap>)
	{
		return momo::internal::ContainerAssignerStd::Move(std::move(right), *this);
	}

	unordered_multimap& operator=(const unordered_multimap& right)
	{
		return momo::internal::ContainerAssignerStd::Copy(right, *this);
	}

	unordered_multimap& operator=(std::initializer_list<value_type> values)
	{
		mHashMultiMap = HashMultiMap(values, mHashMultiMap.GetHashTraits(), MemManager(get_allocator()));
		return *this;
	}

	void swap(unordered_multimap& right) noexcept
	{
		momo::internal::ContainerAssignerStd::Swap(*this, right);
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

	[[nodiscard]] bool empty() const noexcept
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
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE const_iterator find(const KeyArg& key) const
	{
		return equal_range(key).first;
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE iterator find(const KeyArg& key)
	{
		return equal_range(key).first;
	}

	MOMO_FORCEINLINE size_type count(const key_type& key) const
	{
		typename HashMultiMap::ConstKeyIterator keyIter = mHashMultiMap.Find(key);
		return !!keyIter ? keyIter->GetCount() : 0;
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE size_type count(const KeyArg& key) const
	{
		typename HashMultiMap::ConstKeyIterator keyIter = mHashMultiMap.Find(key);
		return !!keyIter ? keyIter->GetCount() : 0;
	}

	MOMO_FORCEINLINE bool contains(const key_type& key) const
	{
		return count(key) > 0;
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE bool contains(const KeyArg& key) const
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
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE std::pair<const_iterator, const_iterator> equal_range(const KeyArg& key) const
	{
		return pvEqualRange<const_iterator, ConstIteratorProxy>(mHashMultiMap,
			mHashMultiMap.Find(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE std::pair<iterator, iterator> equal_range(const KeyArg& key)
	{
		return pvEqualRange<iterator, IteratorProxy>(mHashMultiMap, mHashMultiMap.Find(key));
	}

	template<typename ValueArg = std::pair<key_type, mapped_type>>
	requires std::is_constructible_v<value_type, ValueArg&&>
	iterator insert(ValueArg&& valueArg)
	{
		return emplace(std::forward<ValueArg>(valueArg));
	}

	template<typename ValueArg = std::pair<key_type, mapped_type>>
	requires std::is_constructible_v<value_type, ValueArg&&>
	iterator insert(const_iterator, ValueArg&& valueArg)
	{
		return insert(std::forward<ValueArg>(valueArg));
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	void insert(Iterator first, Iterator last)
	{
		pvInsertRange(first, last);
	}

	void insert(std::initializer_list<value_type> values)
	{
		mHashMultiMap.Add(values.begin(), values.end());
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	void insert_range(Range&& values)
	{
		pvInsertRange(std::ranges::begin(values), std::ranges::end(values));
	}

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
		MOMO_THROW(std::invalid_argument("invalid unordered_multimap erase arguments"));
	}

	size_type erase(const key_type& key)
	{
		return mHashMultiMap.RemoveKey(key);
	}

	template<typename KeyArg>
	requires (IsValidKeyArg<KeyArg>::value &&
		!std::is_convertible_v<KeyArg&&, const_iterator> && !std::is_convertible_v<KeyArg&&, iterator>)
	size_type erase(KeyArg&& key)
	{
		typename HashMultiMap::KeyIterator keyIter = mHashMultiMap.Find(std::forward<KeyArg>(key));
		if (!keyIter)
			return 0;
		size_t count = keyIter->GetCount();
		mHashMultiMap.RemoveKey(keyIter);
		return count;
	}

	template<momo::internal::conceptPredicate<const_reference> ValueFilter>
	friend size_type erase_if(unordered_multimap& cont, ValueFilter valueFilter)
	{
		momo::FastCopyableFunctor<ValueFilter> fastValueFilter(valueFilter);
		auto pairFilter = [fastValueFilter] (const key_type& key, const mapped_type& mapped)
			{ return fastValueFilter(const_reference(key, mapped)); };
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

	bool operator==(const unordered_multimap& right) const
	{
		if (mHashMultiMap.GetKeyCount() != right.mHashMultiMap.GetKeyCount())
			return false;
		if (mHashMultiMap.GetCount() != right.mHashMultiMap.GetCount())
			return false;
		for (typename HashMultiMap::ConstKeyIterator::Reference ref : mHashMultiMap.GetKeyBounds())
		{
			if (ref.GetCount() == 0)
				continue;
			typename HashMultiMap::ConstKeyIterator rightKeyIter = right.mHashMultiMap.Find(ref.key);
			if (!rightKeyIter)
				return false;
			if (ref.GetCount() != rightKeyIter->GetCount())
				return false;
			if (!std::is_permutation(ref.GetBegin(), ref.GetEnd(), rightKeyIter->GetBegin()))
				return false;
		}
		return true;
	}

private:
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
			FastMovableFunctor(MappedCreator(mHashMultiMap.GetMemManager(), std::move(mappedArgs))));
	}

	template<typename... KeyArgs, momo::internal::conceptObjectCreator<mapped_type> MappedCreator>
	iterator pvInsert(std::tuple<KeyArgs...>&& keyArgs, FastMovableFunctor<MappedCreator> mappedCreator)
	{
		MemManager& memManager = mHashMultiMap.GetMemManager();
		typedef momo::internal::ObjectBuffer<key_type,
			HashMultiMap::KeyValueTraits::keyAlignment> KeyBuffer;
		typedef momo::internal::ObjectManager<key_type, MemManager> KeyManager;
		typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
		typedef typename KeyManager::template FinalDestroyer<> KeyFinalDestroyer;
		KeyBuffer keyBuffer;
		KeyCreator(memManager, std::move(keyArgs))(keyBuffer.GetPtr());
		KeyFinalDestroyer keyFin(&memManager, keyBuffer.template GetPtr<true>());
		return pvInsert(std::forward_as_tuple(std::move(keyBuffer.Get())),
			std::move(mappedCreator));
	}

	template<typename RKey, momo::internal::conceptObjectCreator<mapped_type> MappedCreator,
		typename Key = std::decay_t<RKey>>
	requires std::is_same_v<key_type, Key>
	iterator pvInsert(std::tuple<RKey>&& key, FastMovableFunctor<MappedCreator> mappedCreator)
	{
		return IteratorProxy(mHashMultiMap.AddCrt(
			std::forward<RKey>(std::get<0>(key)), std::move(mappedCreator)));
	}

	template<std::input_iterator Iterator,
		momo::internal::conceptSentinel<Iterator> Sentinel>
	void pvInsertRange(Iterator begin, Sentinel end)
	{
		if constexpr (momo::internal::conceptMapArgIterator<Iterator, key_type, false>)
		{
			mHashMultiMap.Add(std::move(begin), std::move(end));
		}
		else
		{
			for (Iterator iter = std::move(begin); iter != end; ++iter)
				insert(*iter);
		}
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

	unordered_multimap_open& operator=(std::initializer_list<value_type> values)
	{
		UnorderedMultiMap::operator=(values);
		return *this;
	}

	friend void swap(unordered_multimap_open& left, unordered_multimap_open& right) noexcept
	{
		left.swap(right);
	}

	template<momo::internal::conceptPredicate<const_reference> ValueFilter>
	friend size_type erase_if(unordered_multimap_open& cont, ValueFilter valueFilter)
	{
		momo::FastCopyableFunctor<ValueFilter> fastValueFilter(valueFilter);
		auto pairFilter = [fastValueFilter] (const key_type& key, const mapped_type& mapped)
			{ return fastValueFilter(const_reference(key, mapped)); };
		return cont.get_nested_container().Remove(pairFilter);
	}
};

#define MOMO_DECLARE_DEDUCTION_GUIDES(unordered_multimap) \
template<typename Iterator, \
	typename Value = std::iter_value_t<Iterator>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>> \
unordered_multimap(Iterator, Iterator) \
	-> unordered_multimap<Key, Mapped>; \
template<typename Iterator, \
	typename Value = std::iter_value_t<Iterator>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_multimap(Iterator, Iterator, size_t, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename Iterator, typename Hasher, \
	typename Value = std::iter_value_t<Iterator>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_multimap(Iterator, Iterator, size_t, Hasher, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, Hasher, std::equal_to<Key>, Allocator>; \
template<typename Iterator, typename Hasher, typename EqualComparer, \
	typename Value = std::iter_value_t<Iterator>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_multimap(Iterator, Iterator, size_t, Hasher, EqualComparer, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, Hasher, EqualComparer, Allocator>; \
template<typename QKey, typename Mapped, \
	typename Key = std::remove_const_t<QKey>> \
unordered_multimap(std::initializer_list<std::pair<QKey, Mapped>>) \
	-> unordered_multimap<Key, Mapped>; \
template<typename QKey, typename Mapped, \
	typename Key = std::remove_const_t<QKey>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_multimap(std::initializer_list<std::pair<QKey, Mapped>>, size_t, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename QKey, typename Mapped, typename Hasher, \
	typename Key = std::remove_const_t<QKey>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_multimap(std::initializer_list<std::pair<QKey, Mapped>>, size_t, Hasher, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, Hasher, std::equal_to<Key>, Allocator>; \
template<typename QKey, typename Mapped, typename Hasher, typename EqualComparer, \
	typename Key = std::remove_const_t<QKey>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_multimap(std::initializer_list<std::pair<QKey, Mapped>>, size_t, Hasher, EqualComparer, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, Hasher, EqualComparer, Allocator>;

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
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_multimap(std::from_range_t, Range&&, size_t, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<std::ranges::input_range Range, typename Hasher, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_multimap(std::from_range_t, Range&&, size_t, Hasher, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, Hasher, std::equal_to<Key>, Allocator>; \
template<std::ranges::input_range Range, typename Hasher, typename EqualComparer, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_multimap(std::from_range_t, Range&&, size_t, Hasher, EqualComparer, Allocator = Allocator()) \
	-> unordered_multimap<Key, Mapped, Hasher, EqualComparer, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES(unordered_multimap)
MOMO_DECLARE_DEDUCTION_GUIDES(unordered_multimap_open)

#if defined(__cpp_lib_containers_ranges)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(unordered_multimap)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(unordered_multimap_open)
#endif

#undef MOMO_DECLARE_DEDUCTION_GUIDES
#undef MOMO_DECLARE_DEDUCTION_GUIDES_RANGES

} // namespace momo::stdish
