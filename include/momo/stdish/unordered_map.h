/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/stdish/unordered_map.h

  namespace momo::stdish:
    class unordered_map
    class unordered_map_open

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_STDISH_UNORDERED_MAP
#define MOMO_INCLUDE_GUARD_STDISH_UNORDERED_MAP

#include "../HashMap.h"
#include "set_map_utility.h"

namespace momo
{

namespace stdish
{

/*!
	\brief
	`momo::stdish::unordered_map` is similar to `std::unordered_map`, but
	much more efficient in memory usage. The implementation is based on
	hash table with buckets in the form of small arrays.

	\details
	Deviations from the `std::unordered_map`:
	1. Container items must be movable (preferably without exceptions)
	or copyable, similar to items of `std::vector`.
	2. After each addition or removal of the item all iterators and
	references to items become invalid and should not be used.
	3. Type `reference` is not the same as `value_type&`, so
	`for (auto& p : map)` is illegal, but `for (auto p : map)` or
	`for (const auto& p : map)` or `for (auto&& p : map)` is allowed.
	4. Functions `clear`, `begin`, `cbegin` and iterator increment take
	O(bucket_count) time in worst case.
	5. If `ObjectManager<key_type>::isNothrowAnywayAssignable` is false
	or `ObjectManager<mapped_type>::isNothrowAnywayAssignable` is false,
	functions `erase` can throw exceptions.
	6. Functions `merge`, `extract` and `insert(node_type&&)` move items.

	It is allowed to pass to functions `insert` and `emplace` references
	to items within the container.
*/

template<typename TKey, typename TMapped,
	typename THasher = HashCoder<TKey>,
	typename TEqualComparer = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>,
	typename THashMap = HashMap<TKey, TMapped, HashTraitsStd<TKey, THasher, TEqualComparer>,
		MemManagerStd<TAllocator>>>
class unordered_map
{
private:
	typedef THashMap HashMap;
	typedef typename HashMap::HashTraits HashTraits;
	typedef typename HashMap::MemManager MemManager;

	typedef typename HashMap::Iterator HashMapIterator;

public:
	typedef TKey key_type;
	typedef TMapped mapped_type;
	typedef THasher hasher;
	typedef TEqualComparer key_equal;
	typedef TAllocator allocator_type;

	typedef HashMap nested_container_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef std::pair<const key_type, mapped_type> value_type;

	typedef momo::internal::HashDerivedIterator<HashMapIterator,
		momo::internal::MapReferenceStd> iterator;
	typedef typename iterator::ConstIterator const_iterator;

	typedef typename iterator::Reference reference;
	typedef typename const_iterator::Reference const_reference;

	typedef typename iterator::Pointer pointer;
	typedef typename const_iterator::Pointer const_pointer;
	//typedef typename std::allocator_traits<allocator_type>::pointer pointer;
	//typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;

	typedef internal::map_node_handle<typename HashMap::ExtractedPair> node_type;
	typedef internal::insert_return_type<iterator, node_type> insert_return_type;

	typedef momo::internal::HashDerivedIterator<typename HashMap::BucketBounds::Iterator,
		momo::internal::MapReferenceStd> local_iterator;
	typedef typename local_iterator::ConstIterator const_local_iterator;

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
		MOMO_DECLARE_PROXY_FUNCTION(Iterator, GetBaseIterator)
	};

	struct ConstLocalIteratorProxy : public const_local_iterator
	{
		typedef const_local_iterator ConstLocalIterator;
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstLocalIterator)
	};

	struct LocalIteratorProxy : public local_iterator
	{
		typedef local_iterator LocalIterator;
		MOMO_DECLARE_PROXY_CONSTRUCTOR(LocalIterator)
	};

	struct NodeTypeProxy : private node_type
	{
		typedef node_type NodeType;
		MOMO_DECLARE_PROXY_FUNCTION(NodeType, GetExtractedPair)
	};

public:
	unordered_map()
	{
	}

	explicit unordered_map(const allocator_type& alloc)
		: mHashMap(HashTraits(), MemManager(alloc))
	{
	}

	explicit unordered_map(size_type bucketCount, const allocator_type& alloc = allocator_type())
		: mHashMap(HashTraits(bucketCount), MemManager(alloc))
	{
	}

	unordered_map(size_type bucketCount, const hasher& hashFunc,
		const allocator_type& alloc = allocator_type())
		: mHashMap(HashTraits(bucketCount, hashFunc), MemManager(alloc))
	{
	}

	unordered_map(size_type bucketCount, const hasher& hashFunc, const key_equal& equalComp,
		const allocator_type& alloc = allocator_type())
		: mHashMap(HashTraits(bucketCount, hashFunc, equalComp), MemManager(alloc))
	{
	}

	template<typename Iterator>
	unordered_map(Iterator first, Iterator last)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_map(Iterator first, Iterator last, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_map(bucketCount, alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_map(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const allocator_type& alloc = allocator_type())
		: unordered_map(bucketCount, hashFunc, alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_map(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalComp, const allocator_type& alloc = allocator_type())
		: unordered_map(bucketCount, hashFunc, equalComp, alloc)
	{
		insert(first, last);
	}

	unordered_map(std::initializer_list<momo::internal::Identity<value_type>> values)
		: unordered_map(values.begin(), values.end())
	{
	}

	unordered_map(std::initializer_list<momo::internal::Identity<value_type>> values,
		size_type bucketCount, const allocator_type& alloc = allocator_type())
		: unordered_map(values.begin(), values.end(), bucketCount, alloc)
	{
	}

	unordered_map(std::initializer_list<momo::internal::Identity<value_type>> values,
		size_type bucketCount, const hasher& hashFunc, const allocator_type& alloc = allocator_type())
		: unordered_map(values.begin(), values.end(), bucketCount, hashFunc, alloc)
	{
	}

	unordered_map(std::initializer_list<momo::internal::Identity<value_type>> values,
		size_type bucketCount, const hasher& hashFunc, const key_equal& equalComp,
		const allocator_type& alloc = allocator_type())
		: unordered_map(values.begin(), values.end(), bucketCount, hashFunc, equalComp, alloc)
	{
	}

#ifdef MOMO_HAS_CONTAINERS_RANGES
	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_map(std::from_range_t, Range&& values)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_map(std::from_range_t, Range&& values, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_map(bucketCount, alloc)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_map(std::from_range_t, Range&& values, size_type bucketCount, const hasher& hashFunc,
		const allocator_type& alloc = allocator_type())
		: unordered_map(bucketCount, hashFunc, alloc)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_map(std::from_range_t, Range&& values, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalComp, const allocator_type& alloc = allocator_type())
		: unordered_map(bucketCount, hashFunc, equalComp, alloc)
	{
		insert_range(std::forward<Range>(values));
	}
#endif // MOMO_HAS_CONTAINERS_RANGES

	unordered_map(unordered_map&& right)
		: unordered_map(std::move(right), right.get_allocator())
	{
	}

	unordered_map(unordered_map&& right, const momo::internal::Identity<allocator_type>& alloc)
		: mHashMap(right.mHashMap.GetHashTraits(), MemManager(alloc))
	{
		if (right.get_allocator() == alloc)
		{
			mHashMap.Swap(right.mHashMap);
		}
		else
		{
			mHashMap.MergeFrom(right.mHashMap);
			right.clear();
		}
	}

	unordered_map(const unordered_map& right)
		: mHashMap(right.mHashMap)
	{
	}

	unordered_map(const unordered_map& right, const momo::internal::Identity<allocator_type>& alloc)
		: mHashMap(right.mHashMap, MemManager(alloc))
	{
	}

	~unordered_map() = default;

	unordered_map& operator=(unordered_map&& right)
		noexcept(momo::internal::ContainerAssignerStd::IsNothrowMoveAssignable<unordered_map>::value)
	{
		return momo::internal::ContainerAssignerStd::Move(std::move(right), *this);
	}

	unordered_map& operator=(const unordered_map& right)
	{
		return momo::internal::ContainerAssignerStd::Copy(right, *this);
	}

	unordered_map& operator=(std::initializer_list<value_type> values)
	{
		mHashMap = HashMap(values, mHashMap.GetHashTraits(), MemManager(get_allocator()));
		return *this;
	}

	void swap(unordered_map& right) noexcept
	{
		momo::internal::ContainerAssignerStd::Swap(*this, right);
	}

	friend void swap(unordered_map& left, unordered_map& right) noexcept
	{
		left.swap(right);
	}

	const nested_container_type& get_nested_container() const noexcept
	{
		return mHashMap;
	}

	nested_container_type& get_nested_container() noexcept
	{
		return mHashMap;
	}

	const_iterator begin() const noexcept
	{
		return ConstIteratorProxy(mHashMap.GetBegin());
	}

	iterator begin() noexcept
	{
		return IteratorProxy(mHashMap.GetBegin());
	}

	const_iterator end() const noexcept
	{
		return ConstIteratorProxy(mHashMap.GetEnd());
	}

	iterator end() noexcept
	{
		return IteratorProxy(mHashMap.GetEnd());
	}

	const_iterator cbegin() const noexcept
	{
		return begin();
	}

	const_iterator cend() const noexcept
	{
		return end();
	}

	float max_load_factor() const noexcept
	{
		return mHashMap.GetHashTraits().GetMaxLoadFactor(HashMap::bucketMaxItemCount);
	}

	void max_load_factor(float maxLoadFactor)
	{
		if (maxLoadFactor == max_load_factor())
			return;
		if (maxLoadFactor <= 0.0 || maxLoadFactor > static_cast<float>(HashMap::bucketMaxItemCount))
			throw std::out_of_range("invalid load factor");
		HashTraits hashTraits(mHashMap.GetHashTraits(), maxLoadFactor);
		HashMap hashMap(hashTraits, MemManager(get_allocator()));
		hashMap.Reserve(size());
		hashMap.Insert(begin(), end());
		mHashMap = std::move(hashMap);
	}

	hasher hash_function() const
	{
		return mHashMap.GetHashTraits().GetHasher();
	}

	key_equal key_eq() const
	{
		return mHashMap.GetHashTraits().GetEqualComparer();
	}

	allocator_type get_allocator() const noexcept
	{
		return allocator_type(mHashMap.GetMemManager().GetByteAllocator());
	}

	size_type max_size() const noexcept
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const noexcept
	{
		return mHashMap.GetCount();
	}

	MOMO_NODISCARD bool empty() const noexcept
	{
		return mHashMap.IsEmpty();
	}

	void clear() noexcept
	{
		mHashMap.Clear();
	}

	void rehash(size_type bucketCount)
	{
		bucketCount = std::minmax(bucketCount, size_t{2}).second;
		size_t logBucketCount = momo::internal::UIntMath<>::Log2(bucketCount - 1) + 1;
		bucketCount = size_t{1} << logBucketCount;
		reserve(mHashMap.GetHashTraits().CalcCapacity(bucketCount, HashMap::bucketMaxItemCount));
	}

	void reserve(size_type count)
	{
		mHashMap.Reserve(count);
	}

	MOMO_FORCEINLINE const_iterator find(const key_type& key) const
	{
		return ConstIteratorProxy(mHashMap.Find(key));
	}

	MOMO_FORCEINLINE iterator find(const key_type& key)
	{
		return IteratorProxy(mHashMap.Find(key));
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	const_iterator> find(const KeyArg& key) const
	{
		return ConstIteratorProxy(mHashMap.Find(key));
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	iterator> find(const KeyArg& key)
	{
		return IteratorProxy(mHashMap.Find(key));
	}

	MOMO_FORCEINLINE size_type count(const key_type& key) const
	{
		return contains(key) ? 1 : 0;
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	size_type> count(const KeyArg& key) const
	{
		return contains(key) ? 1 : 0;
	}

	MOMO_FORCEINLINE bool contains(const key_type& key) const
	{
		return mHashMap.ContainsKey(key);
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	bool> contains(const KeyArg& key) const
	{
		return mHashMap.ContainsKey(key);
	}

	MOMO_FORCEINLINE std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		return { find(key), end() };
	}

	MOMO_FORCEINLINE std::pair<iterator, iterator> equal_range(const key_type& key)
	{
		return { find(key), end() };
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	std::pair<const_iterator, const_iterator>> equal_range(const KeyArg& key) const
	{
		return { find(key), end() };
	}

	template<typename KeyArg>
	MOMO_FORCEINLINE momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	std::pair<iterator, iterator>> equal_range(const KeyArg& key)
	{
		return { find(key), end() };
	}

	template<typename ValueArg = std::pair<key_type, mapped_type>>
	momo::internal::EnableIf<std::is_constructible<value_type, ValueArg&&>::value,
	std::pair<iterator, bool>> insert(ValueArg&& valueArg)
	{
		return emplace(std::forward<ValueArg>(valueArg));
	}

	template<typename ValueArg = std::pair<key_type, mapped_type>>
	momo::internal::EnableIf<std::is_constructible<value_type, ValueArg&&>::value,
	iterator> insert(const_iterator hint, ValueArg&& valueArg)
	{
		return emplace_hint(hint, std::forward<ValueArg>(valueArg));
	}

	insert_return_type insert(node_type&& node)
	{
		if (node.empty())
			return { end(), false, node_type() };
		typename HashMap::InsertResult res = mHashMap.Insert(
			std::move(NodeTypeProxy::GetExtractedPair(node)));
		return { IteratorProxy(res.position), res.inserted,
			res.inserted ? node_type() : std::move(node) };
	}

	iterator insert(const_iterator hint, node_type&& node)
	{
#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
		if (node.empty())
			return end();
		return IteratorProxy(mHashMap.Add(ConstIteratorProxy::GetBaseIterator(hint),
			std::move(NodeTypeProxy::GetExtractedPair(node))));
#else
		(void)hint;
		return insert(std::move(node)).position;
#endif
	}

	template<typename Iterator>
	void insert(Iterator first, Iterator last)
	{
		pvInsertRange(first, last);
	}

	void insert(std::initializer_list<value_type> values)
	{
		mHashMap.Insert(values.begin(), values.end());
	}

#ifdef MOMO_HAS_CONTAINERS_RANGES
	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	void insert_range(Range&& values)
	{
		pvInsertRange(std::ranges::begin(values), std::ranges::end(values));
	}
#endif // MOMO_HAS_CONTAINERS_RANGES

	std::pair<iterator, bool> emplace()
	{
		return pvEmplace(nullptr, std::tuple<>(), std::tuple<>());
	}

	iterator emplace_hint(const_iterator hint)
	{
		return pvEmplace(hint, std::tuple<>(), std::tuple<>()).first;
	}

	template<typename ValueArg>
	std::pair<iterator, bool> emplace(ValueArg&& valueArg)
	{
		return pvEmplace(nullptr,
			std::forward_as_tuple(std::get<0>(std::forward<ValueArg>(valueArg))),
			std::forward_as_tuple(std::get<1>(std::forward<ValueArg>(valueArg))));
	}

	template<typename ValueArg>
	iterator emplace_hint(const_iterator hint, ValueArg&& valueArg)
	{
		return pvEmplace(hint,
			std::forward_as_tuple(std::get<0>(std::forward<ValueArg>(valueArg))),
			std::forward_as_tuple(std::get<1>(std::forward<ValueArg>(valueArg)))).first;
	}

	template<typename KeyArg, typename MappedArg>
	std::pair<iterator, bool> emplace(KeyArg&& keyArg, MappedArg&& mappedArg)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(std::forward<KeyArg>(keyArg)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg)));
	}

	template<typename KeyArg, typename MappedArg>
	iterator emplace_hint(const_iterator hint, KeyArg&& keyArg, MappedArg&& mappedArg)
	{
		return pvEmplace(hint, std::forward_as_tuple(std::forward<KeyArg>(keyArg)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg))).first;
	}

	template<typename... KeyArgs, typename... MappedArgs>
	std::pair<iterator, bool> emplace(std::piecewise_construct_t,
		std::tuple<KeyArgs...> keyArgs, std::tuple<MappedArgs...> mappedArgs)
	{
		return pvEmplace(nullptr, std::move(keyArgs), std::move(mappedArgs));
	}

	template<typename... KeyArgs, typename... MappedArgs>
	iterator emplace_hint(const_iterator hint, std::piecewise_construct_t,
		std::tuple<KeyArgs...> keyArgs, std::tuple<MappedArgs...> mappedArgs)
	{
		return pvEmplace(hint, std::move(keyArgs), std::move(mappedArgs)).first;
	}

	iterator erase(const_iterator where)
	{
		return IteratorProxy(mHashMap.Remove(ConstIteratorProxy::GetBaseIterator(where)));
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
			return IteratorProxy(mHashMap.MakeMutableIterator(
				ConstIteratorProxy::GetBaseIterator(first)));
		}
		if (first != end() && std::next(first) == last)
			return erase(first);
		throw std::invalid_argument("invalid unordered_map erase arguments");
	}

	size_type erase(const key_type& key)
	{
		return mHashMap.Remove(key) ? 1 : 0;
	}

	template<typename ValueFilter>
	friend size_type erase_if(unordered_map& cont, const ValueFilter& valueFilter)
	{
		auto pairFilter = [&valueFilter] (const key_type& key, const mapped_type& mapped)
			{ return valueFilter(const_reference(key, mapped)); };
		return cont.mHashMap.Remove(pairFilter);
	}

	MOMO_FORCEINLINE typename HashMap::ValueReferenceRKey operator[](key_type&& key)
	{
		return mHashMap[std::move(key)];
	}

	MOMO_FORCEINLINE typename HashMap::ValueReferenceCKey operator[](const key_type& key)
	{
		return mHashMap[key];
	}

	MOMO_FORCEINLINE const mapped_type& at(const key_type& key) const
	{
		const_iterator iter = find(key);
		if (iter == end())
			throw std::out_of_range("invalid unordered_map key");
		return iter->second;
	}

	MOMO_FORCEINLINE mapped_type& at(const key_type& key)
	{
		iterator iter = find(key);
		if (iter == end())
			throw std::out_of_range("invalid unordered_map key");
		return iter->second;
	}

	template<typename... MappedArgs>
	std::pair<iterator, bool> try_emplace(key_type&& key, MappedArgs&&... mappedArgs)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(std::move(key)),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...));
	}

	template<typename... MappedArgs>
	iterator try_emplace(const_iterator hint, key_type&& key, MappedArgs&&... mappedArgs)
	{
		return pvEmplace(hint, std::forward_as_tuple(std::move(key)),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...)).first;
	}

	template<typename... MappedArgs>
	std::pair<iterator, bool> try_emplace(const key_type& key, MappedArgs&&... mappedArgs)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(key),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...));
	}

	template<typename... MappedArgs>
	iterator try_emplace(const_iterator hint, const key_type& key, MappedArgs&&... mappedArgs)
	{
		return pvEmplace(hint, std::forward_as_tuple(key),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...)).first;
	}

	template<typename MappedArg>
	std::pair<iterator, bool> insert_or_assign(key_type&& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(nullptr, std::move(key), std::forward<MappedArg>(mappedArg));
	}

	template<typename MappedArg>
	iterator insert_or_assign(const_iterator hint, key_type&& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(hint, std::move(key), std::forward<MappedArg>(mappedArg)).first;
	}

	template<typename MappedArg>
	std::pair<iterator, bool> insert_or_assign(const key_type& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(nullptr, key, std::forward<MappedArg>(mappedArg));
	}

	template<typename MappedArg>
	iterator insert_or_assign(const_iterator hint, const key_type& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(hint, key, std::forward<MappedArg>(mappedArg)).first;
	}

	node_type extract(const_iterator where)
	{
		return node_type(*this, where);	// need RVO for exception safety
	}

	node_type extract(const key_type& key)
	{
		const_iterator iter = find(key);
		return (iter != end()) ? extract(iter) : node_type();
	}

	template<typename Map>
	void merge(Map&& map)
	{
		mHashMap.MergeFrom(map.get_nested_container());
	}

	size_type max_bucket_count() const noexcept
	{
		return momo::internal::UIntConst::maxSize;
		//return momo::internal::HashSetBuckets<Bucket>::maxBucketCount;
	}

	size_type bucket_count() const noexcept
	{
		return mHashMap.GetBucketCount();
	}

	size_type bucket_size(size_type bucketIndex) const
	{
		return mHashMap.GetBucketBounds(bucketIndex).GetCount();
	}

	local_iterator begin(size_type bucketIndex)
	{
		return LocalIteratorProxy(mHashMap.GetBucketBounds(bucketIndex).GetBegin());
	}

	const_local_iterator begin(size_type bucketIndex) const
	{
		return ConstLocalIteratorProxy(mHashMap.GetBucketBounds(bucketIndex).GetBegin());
	}

	local_iterator end(size_type bucketIndex)
	{
		return LocalIteratorProxy(mHashMap.GetBucketBounds(bucketIndex).GetEnd());
	}

	const_local_iterator end(size_type bucketIndex) const
	{
		return ConstLocalIteratorProxy(mHashMap.GetBucketBounds(bucketIndex).GetEnd());
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

	float load_factor() const noexcept
	{
		size_t count = size();
		size_t bucketCount = bucket_count();
		if (count == 0 && bucketCount == 0)
			return 0.0;
		return static_cast<float>(count) / static_cast<float>(bucketCount);
	}

	friend bool operator==(const unordered_map& left, const unordered_map& right)
	{
		if (left.size() != right.size())
			return false;
		for (const_reference ref : left)
		{
			const_iterator iter = right.find(ref.first);
			if (iter == right.end())
				return false;
			if (!(iter->second == ref.second))
				return false;
		}
		return true;
	}

	friend bool operator!=(const unordered_map& left, const unordered_map& right)
	{
		return !(left == right);
	}

private:
	template<typename Hint, typename... KeyArgs, typename... MappedArgs>
	std::pair<iterator, bool> pvEmplace(Hint hint, std::tuple<KeyArgs...>&& keyArgs,
		std::tuple<MappedArgs...>&& mappedArgs)
	{
		typedef typename HashMap::KeyValueTraits
			::template ValueCreator<MappedArgs...> MappedCreator;
		return pvInsert(hint, std::move(keyArgs),
			MappedCreator(mHashMap.GetMemManager(), std::move(mappedArgs)));
	}

	template<typename Hint, typename... KeyArgs, typename MappedCreator>
	std::pair<iterator, bool> pvInsert(Hint /*hint*/, std::tuple<KeyArgs...>&& keyArgs,
		MappedCreator&& mappedCreator)
	{
		MemManager& memManager = mHashMap.GetMemManager();
		typedef momo::internal::ObjectBuffer<key_type, HashMap::KeyValueTraits::keyAlignment> KeyBuffer;
		typedef momo::internal::ObjectManager<key_type, MemManager> KeyManager;
		typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
		KeyBuffer keyBuffer;
		KeyCreator(memManager, std::move(keyArgs))(keyBuffer.GetPtr());
		key_type* keyPtr = keyBuffer.template GetPtr<true>();
		try
		{
			typename HashMap::Position pos = mHashMap.Find(*keyPtr);
			if (!!pos)
			{
				KeyManager::Destroy(memManager, *keyPtr);
				keyPtr = nullptr;
				return { IteratorProxy(pos), false };
			}
			auto valueCreator = [&memManager, &keyPtr, &mappedCreator]
				(key_type* newKey, mapped_type* newMapped)
			{
				KeyManager::Relocate(memManager, *keyPtr, newKey);
				keyPtr = nullptr;
				try
				{
					std::forward<MappedCreator>(mappedCreator)(newMapped);
				}
				catch (...)
				{
					KeyManager::Destroy(memManager, *newKey);
					throw;
				}
			};
			typename HashMap::Position resPos = mHashMap.AddCrt(pos, valueCreator);
			return { IteratorProxy(resPos), true };
		}
		catch (...)
		{
			if (keyPtr != nullptr)
				KeyManager::Destroy(memManager, *keyPtr);
			throw;
		}
	}

	template<typename Hint, typename RKey, typename MappedCreator,
		typename Key = typename std::decay<RKey>::type>
	momo::internal::EnableIf<std::is_same<key_type, Key>::value,
	std::pair<iterator, bool>> pvInsert(Hint /*hint*/, std::tuple<RKey>&& key,
		MappedCreator&& mappedCreator)
	{
		typename HashMap::InsertResult res = mHashMap.InsertCrt(
			std::forward<RKey>(std::get<0>(key)), std::forward<MappedCreator>(mappedCreator));
		return { IteratorProxy(res.position), res.inserted };
	}

#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
	template<typename... KeyArgs, typename MappedCreator>
	std::pair<iterator, bool> pvInsert(const_iterator hint, std::tuple<KeyArgs...>&& keyArgs,
		MappedCreator&& mappedCreator)
	{
		MemManager& memManager = mHashMap.GetMemManager();
		typedef momo::internal::ObjectManager<key_type, MemManager> KeyManager;
		typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
		auto valueCreator = [&memManager, &keyArgs, &mappedCreator]
			(key_type* newKey, mapped_type* newMapped)
		{
			KeyCreator(memManager, std::move(keyArgs))(newKey);
			try
			{
				std::forward<MappedCreator>(mappedCreator)(newMapped);
			}
			catch (...)
			{
				KeyManager::Destroy(memManager, *newKey);
				throw;
			}
		};
		typename HashMap::Position resPos = mHashMap.AddCrt(
			ConstIteratorProxy::GetBaseIterator(hint), valueCreator);
		return { IteratorProxy(resPos), true };
	}

	template<typename RKey, typename MappedCreator,
		typename Key = typename std::decay<RKey>::type>
	momo::internal::EnableIf<std::is_same<key_type, Key>::value,
	std::pair<iterator, bool>> pvInsert(const_iterator hint, std::tuple<RKey>&& key,
		MappedCreator&& mappedCreator)
	{
		typename HashMap::Position resPos = mHashMap.AddCrt(ConstIteratorProxy::GetBaseIterator(hint),
			std::forward<RKey>(std::get<0>(key)), std::forward<MappedCreator>(mappedCreator));
		return { IteratorProxy(resPos), true };
	}
#endif // MOMO_USE_UNORDERED_HINT_ITERATORS

	template<typename Iterator, typename Sentinel>
	momo::internal::EnableIf<momo::internal::IsMapArgIteratorStd<Iterator, key_type>::value,
	void> pvInsertRange(Iterator begin, Sentinel end)
	{
		mHashMap.Insert(std::move(begin), std::move(end));
	}

	template<typename Iterator, typename Sentinel>
	momo::internal::EnableIf<!momo::internal::IsMapArgIteratorStd<Iterator, key_type>::value,
	void> pvInsertRange(Iterator begin, Sentinel end)
	{
		for (Iterator iter = std::move(begin); iter != end; ++iter)
			insert(*iter);
	}

	template<typename Hint, typename RKey, typename MappedArg>
	std::pair<iterator, bool> pvInsertOrAssign(Hint hint, RKey&& key, MappedArg&& mappedArg)
	{
		std::pair<iterator, bool> res = pvEmplace(hint,
			std::forward_as_tuple(std::forward<RKey>(key)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg)));
		if (!res.second)
			res.first->second = std::forward<MappedArg>(mappedArg);
		return res;
	}

private:
	HashMap mHashMap;
};

/*!
	\brief
	`momo::stdish::unordered_map_open` is similar to `std::unordered_map`,
	but much more efficient in operation speed. The implementation is based
	on open addressing hash table.

	\copydetails momo::stdish::unordered_map
*/

template<typename TKey, typename TMapped,
	typename THasher = HashCoder<TKey>,
	typename TEqualComparer = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
class unordered_map_open : public unordered_map<TKey, TMapped, THasher, TEqualComparer, TAllocator,
	HashMap<TKey, TMapped, HashTraitsStd<TKey, THasher, TEqualComparer, HashBucketOpenDefault>,
		MemManagerStd<TAllocator>>>
{
private:
	typedef unordered_map<TKey, TMapped, THasher, TEqualComparer, TAllocator,
		momo::HashMap<TKey, TMapped, HashTraitsStd<TKey, THasher, TEqualComparer, HashBucketOpenDefault>,
		MemManagerStd<TAllocator>>> UnorderedMap;

public:
	using typename UnorderedMap::key_type;
	using typename UnorderedMap::mapped_type;
	using typename UnorderedMap::size_type;
	using typename UnorderedMap::value_type;
	using typename UnorderedMap::const_reference;

public:
	using UnorderedMap::UnorderedMap;

	unordered_map_open& operator=(std::initializer_list<value_type> values)
	{
		UnorderedMap::operator=(values);
		return *this;
	}

	friend void swap(unordered_map_open& left, unordered_map_open& right) noexcept
	{
		left.swap(right);
	}

	template<typename ValueFilter>
	friend size_type erase_if(unordered_map_open& cont, const ValueFilter& valueFilter)
	{
		auto pairFilter = [&valueFilter] (const key_type& key, const mapped_type& mapped)
			{ return valueFilter(const_reference(key, mapped)); };
		return cont.get_nested_container().Remove(pairFilter);
	}
};

#ifdef MOMO_HAS_DEDUCTION_GUIDES

#define MOMO_DECLARE_DEDUCTION_GUIDES(unordered_map) \
template<typename Iterator, \
	typename Value = typename std::iterator_traits<Iterator>::value_type, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>> \
unordered_map(Iterator, Iterator) \
	-> unordered_map<Key, Mapped>; \
template<typename Iterator, \
	typename Value = typename std::iterator_traits<Iterator>::value_type, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, HashCoder<Key>>> \
unordered_map(Iterator, Iterator, size_t, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename Iterator, typename Hasher, \
	typename Value = typename std::iterator_traits<Iterator>::value_type, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, Hasher>> \
unordered_map(Iterator, Iterator, size_t, Hasher, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, Hasher, std::equal_to<Key>, Allocator>; \
template<typename Iterator, typename Hasher, typename EqualComparer, \
	typename Value = typename std::iterator_traits<Iterator>::value_type, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, Hasher, EqualComparer>> \
unordered_map(Iterator, Iterator, size_t, Hasher, EqualComparer, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, Hasher, EqualComparer, Allocator>; \
template<typename QKey, typename Mapped, \
	typename Key = std::remove_const_t<QKey>> \
unordered_map(std::initializer_list<std::pair<QKey, Mapped>>) \
	-> unordered_map<Key, Mapped>; \
template<typename QKey, typename Mapped, \
	typename Key = std::remove_const_t<QKey>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, HashCoder<Key>>> \
unordered_map(std::initializer_list<std::pair<QKey, Mapped>>, size_t, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename QKey, typename Mapped, typename Hasher, \
	typename Key = std::remove_const_t<QKey>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, Hasher>> \
unordered_map(std::initializer_list<std::pair<QKey, Mapped>>, size_t, Hasher, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, Hasher, std::equal_to<Key>, Allocator>; \
template<typename QKey, typename Mapped, typename Hasher, typename EqualComparer, \
	typename Key = std::remove_const_t<QKey>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, Hasher, EqualComparer>> \
unordered_map(std::initializer_list<std::pair<QKey, Mapped>>, size_t, Hasher, EqualComparer, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, Hasher, EqualComparer, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES(unordered_map)
MOMO_DECLARE_DEDUCTION_GUIDES(unordered_map_open)

#undef MOMO_DECLARE_DEDUCTION_GUIDES

#ifdef MOMO_HAS_CONTAINERS_RANGES

#define MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(unordered_map) \
template<std::ranges::input_range Range, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>> \
unordered_map(std::from_range_t, Range&&) \
	-> unordered_map<Key, Mapped>; \
template<std::ranges::input_range Range, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, HashCoder<Key>>> \
unordered_map(std::from_range_t, Range&&, size_t, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<std::ranges::input_range Range, typename Hasher, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, Hasher>> \
unordered_map(std::from_range_t, Range&&, size_t, Hasher, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, Hasher, std::equal_to<Key>, Allocator>; \
template<std::ranges::input_range Range, typename Hasher, typename EqualComparer, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::unordered_checker<Key, Allocator, Hasher, EqualComparer>> \
unordered_map(std::from_range_t, Range&&, size_t, Hasher, EqualComparer, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, Hasher, EqualComparer, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(unordered_map)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(unordered_map_open)

#undef MOMO_DECLARE_DEDUCTION_GUIDES_RANGES

#endif // MOMO_HAS_CONTAINERS_RANGES

#endif // MOMO_HAS_DEDUCTION_GUIDES

} // namespace stdish

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_STDISH_UNORDERED_MAP
