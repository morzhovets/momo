/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/stdish/unordered_map.h

  namespace momo::stdish:
    class unordered_map
    class unordered_map_open

\**********************************************************/

#pragma once

#include "../HashMap.h"
#include "node_handle.h"

namespace momo::stdish
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
	typename THashFunc = HashCoder<TKey>,
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

	typedef typename HashMap::Iterator HashMapIterator;

public:
	typedef TKey key_type;
	typedef TMapped mapped_type;
	typedef THashFunc hasher;
	typedef TEqualFunc key_equal;
	typedef TAllocator allocator_type;

	typedef HashMap nested_container_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef std::pair<const key_type, mapped_type> value_type;

	typedef momo::internal::DerivedForwardIterator<HashMapIterator,
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

	typedef momo::internal::DerivedForwardIterator<typename HashMap::BucketBounds::Iterator,
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

	unordered_map(size_type bucketCount, const hasher& hashFunc, const key_equal& equalFunc,
		const allocator_type& alloc = allocator_type())
		: mHashMap(HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
	}

	template<momo::internal::conceptInputIterator Iterator>
	unordered_map(Iterator first, Iterator last)
	{
		insert(first, last);
	}

	template<momo::internal::conceptInputIterator Iterator>
	unordered_map(Iterator first, Iterator last, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_map(bucketCount, alloc)
	{
		insert(first, last);
	}

	template<momo::internal::conceptInputIterator Iterator>
	unordered_map(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const allocator_type& alloc = allocator_type())
		: unordered_map(bucketCount, hashFunc, alloc)
	{
		insert(first, last);
	}

	template<momo::internal::conceptInputIterator Iterator>
	unordered_map(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalFunc, const allocator_type& alloc = allocator_type())
		: unordered_map(bucketCount, hashFunc, equalFunc, alloc)
	{
		insert(first, last);
	}

	unordered_map(std::initializer_list<value_type> values)
		: unordered_map(values.begin(), values.end())
	{
	}

	unordered_map(std::initializer_list<value_type> values, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_map(values.begin(), values.end(), bucketCount, alloc)
	{
	}

	unordered_map(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const allocator_type& alloc = allocator_type())
		: unordered_map(values.begin(), values.end(), bucketCount, hashFunc, alloc)
	{
	}

	unordered_map(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalFunc,
		const allocator_type& alloc = allocator_type())
		: unordered_map(values.begin(), values.end(), bucketCount, hashFunc, equalFunc, alloc)
	{
	}

	unordered_map(unordered_map&& right) noexcept
		: mHashMap(std::move(right.mHashMap))
	{
	}

	unordered_map(unordered_map&& right, const std::type_identity_t<allocator_type>& alloc)
		noexcept(std::allocator_traits<allocator_type>::is_always_equal::value)
		: mHashMap(pvCreateMap(std::move(right), alloc))
	{
	}

	unordered_map(const unordered_map& right)
		: mHashMap(right.mHashMap)
	{
	}

	unordered_map(const unordered_map& right, const std::type_identity_t<allocator_type>& alloc)
		: mHashMap(right.mHashMap, MemManager(alloc))
	{
	}

	~unordered_map() noexcept = default;

	unordered_map& operator=(unordered_map&& right)
		noexcept(std::allocator_traits<allocator_type>::is_always_equal::value ||
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>::is_always_equal::value ||
				std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value;
			allocator_type alloc = (propagate ? &right : this)->get_allocator();
			mHashMap = pvCreateMap(std::move(right), alloc);
		}
		return *this;
	}

	unordered_map& operator=(const unordered_map& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>::is_always_equal::value ||
				std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value;
			allocator_type alloc = (propagate ? &right : this)->get_allocator();
			mHashMap = HashMap(right.mHashMap, MemManager(alloc));
		}
		return *this;
	}

	unordered_map& operator=(std::initializer_list<value_type> values)
	{
		HashMap hashMap(mHashMap.GetHashTraits(), MemManager(get_allocator()));
		hashMap.Insert(values.begin(), values.end());
		mHashMap = std::move(hashMap);
		return *this;
	}

	void swap(unordered_map& right) noexcept
	{
		MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
			|| get_allocator() == right.get_allocator());
		mHashMap.Swap(right.mHashMap);
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
		return mHashMap.GetHashTraits().GetHashFunc();
	}

	key_equal key_eq() const
	{
		return mHashMap.GetHashTraits().GetEqualFunc();
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

	[[nodiscard]] bool empty() const noexcept
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
		bucketCount = std::bit_ceil(bucketCount);
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
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE const_iterator find(const KeyArg& key) const
	{
		return ConstIteratorProxy(mHashMap.Find(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE iterator find(const KeyArg& key)
	{
		return IteratorProxy(mHashMap.Find(key));
	}

	MOMO_FORCEINLINE size_type count(const key_type& key) const
	{
		return contains(key) ? 1 : 0;
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE size_type count(const KeyArg& key) const
	{
		return contains(key) ? 1 : 0;
	}

	MOMO_FORCEINLINE bool contains(const key_type& key) const
	{
		return mHashMap.ContainsKey(key);
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE bool contains(const KeyArg& key) const
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
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE std::pair<const_iterator, const_iterator> equal_range(const KeyArg& key) const
	{
		return { find(key), end() };
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE std::pair<iterator, iterator> equal_range(const KeyArg& key)
	{
		return { find(key), end() };
	}

	//template<typename Value>
	//requires std::is_constructible_v<value_type, Value>
	//std::pair<iterator, bool> insert(Value&& value)

	//template<typename Value>
	//requires std::is_constructible_v<value_type, Value>
	//iterator insert(const_iterator hint, Value&& value)

	//std::pair<iterator, bool> insert(value_type&& value)

	//iterator insert(const_iterator hint, value_type&& value)

	//std::pair<iterator, bool> insert(const value_type& value)

	//iterator insert(const_iterator hint, const value_type& value)

	std::pair<iterator, bool> insert(std::pair<key_type, mapped_type>&& value)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(std::move(value.first)),
			std::forward_as_tuple(std::move(value.second)));
	}

	iterator insert(const_iterator hint, std::pair<key_type, mapped_type>&& value)
	{
		return pvEmplace(hint, std::forward_as_tuple(std::move(value.first)),
			std::forward_as_tuple(std::move(value.second))).first;
	}

	template<typename First, typename Second>
	requires std::is_constructible_v<key_type, const First&>
		&& std::is_constructible_v<mapped_type, const Second&>
	std::pair<iterator, bool> insert(const std::pair<First, Second>& value)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(value.first),
			std::forward_as_tuple(value.second));
	}

	template<typename First, typename Second>
	requires std::is_constructible_v<key_type, const First&>
		&& std::is_constructible_v<mapped_type, const Second&>
	iterator insert(const_iterator hint, const std::pair<First, Second>& value)
	{
		return pvEmplace(hint, std::forward_as_tuple(value.first),
			std::forward_as_tuple(value.second)).first;
	}

	template<typename First, typename Second>
	requires std::is_constructible_v<key_type, First&&>
		&& std::is_constructible_v<mapped_type, Second&&>
	std::pair<iterator, bool> insert(std::pair<First, Second>&& value)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(std::forward<First>(value.first)),
			std::forward_as_tuple(std::forward<Second>(value.second)));
	}

	template<typename First, typename Second>
	requires std::is_constructible_v<key_type, First&&>
		&& std::is_constructible_v<mapped_type, Second&&>
	iterator insert(const_iterator hint, std::pair<First, Second>&& value)
	{
		return pvEmplace(hint, std::forward_as_tuple(std::forward<First>(value.first)),
			std::forward_as_tuple(std::forward<Second>(value.second))).first;
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

	iterator insert([[maybe_unused]] const_iterator hint, node_type&& node)
	{
#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
		if (node.empty())
			return end();
		return IteratorProxy(mHashMap.Add(ConstIteratorProxy::GetBaseIterator(hint),
			std::move(NodeTypeProxy::GetExtractedPair(node))));
#else
		return insert(std::move(node)).position;
#endif
	}

	template<momo::internal::conceptInputIterator Iterator>
	void insert(Iterator first, Iterator last)
	{
		if constexpr (momo::internal::conceptMapArgIterator<Iterator, key_type, false>)
		{
			mHashMap.Insert(first, last);
		}
		else
		{
			for (Iterator iter = first; iter != last; ++iter)
				insert(*iter);
		}
	}

	void insert(std::initializer_list<value_type> values)
	{
		mHashMap.Insert(values.begin(), values.end());
	}

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
		return insert(std::forward<ValueArg>(valueArg));
	}

	template<typename ValueArg>
	iterator emplace_hint(const_iterator hint, ValueArg&& valueArg)
	{
		return insert(hint, std::forward<ValueArg>(valueArg));
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

	template<typename KeyArg>
	requires (IsValidKeyArg<KeyArg>::value &&
		!std::is_convertible_v<KeyArg&&, const_iterator> && !std::is_convertible_v<KeyArg&&, iterator>)
	size_type erase(KeyArg&& key)
	{
		iterator iter = find(std::forward<KeyArg>(key));
		if (iter == end())
			return 0;
		erase(iter);
		return 1;
	}

	template<typename Predicate>
	requires std::predicate<const Predicate&, const_reference>
	friend size_type erase_if(unordered_map& cont, const Predicate& pred)
	{
		auto pairPred = [&pred] (const key_type& key, const mapped_type& mapped)
			{ return pred(const_reference(key, mapped)); };
		return cont.mHashMap.Remove(pairPred);
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
		return pvInsertOrAssign(std::move(key), std::forward<MappedArg>(mappedArg));
	}

	template<typename MappedArg>
	iterator insert_or_assign(const_iterator hint, key_type&& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(hint, std::move(key), std::forward<MappedArg>(mappedArg));
	}

	template<typename MappedArg>
	std::pair<iterator, bool> insert_or_assign(const key_type& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(key, std::forward<MappedArg>(mappedArg));
	}

	template<typename MappedArg>
	iterator insert_or_assign(const_iterator hint, const key_type& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(hint, key, std::forward<MappedArg>(mappedArg));
	}

	node_type extract(const_iterator where)
	{
		return node_type(*this, where);
	}

	node_type extract(const key_type& key)
	{
		iterator iter = find(key);
		return (iter != end()) ? extract(iter) : node_type();
	}

	template<typename KeyArg>
	requires (IsValidKeyArg<KeyArg>::value &&
		!std::is_convertible_v<KeyArg&&, const_iterator> && !std::is_convertible_v<KeyArg&&, iterator>)
	node_type extract(KeyArg&& key)
	{
		iterator iter = find(std::forward<KeyArg>(key));
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

private:
	static HashMap pvCreateMap(unordered_map&& right, const allocator_type& alloc)
	{
		if (right.get_allocator() == alloc)
			return std::move(right.mHashMap);
		HashMap hashMap(right.mHashMap.GetHashTraits(), MemManager(alloc));
		hashMap.MergeFrom(right.mHashMap);
		return hashMap;
	}

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
		MappedCreator mappedCreator)
	{
		MemManager& memManager = mHashMap.GetMemManager();
		typedef momo::internal::ObjectBuffer<key_type, HashMap::KeyValueTraits::keyAlignment> KeyBuffer;
		typedef momo::internal::ObjectManager<key_type, MemManager> KeyManager;
		typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
		KeyBuffer keyBuffer;
		KeyCreator(memManager, std::move(keyArgs))(&keyBuffer);
		bool keyDestroyed = false;
		try
		{
			typename HashMap::Position pos = mHashMap.Find(*&keyBuffer);
			if (!!pos)
			{
				KeyManager::Destroy(memManager, *&keyBuffer);
				keyDestroyed = true;
				return { IteratorProxy(pos), false };
			}
			auto valueCreator = [&memManager, &keyBuffer, &keyDestroyed,
				mappedCreator = std::move(mappedCreator)]
				(key_type* newKey, mapped_type* newMapped) mutable
			{
				KeyManager::Relocate(memManager, *&keyBuffer, newKey);
				keyDestroyed = true;
				try
				{
					std::move(mappedCreator)(newMapped);
				}
				catch (...)
				{
					KeyManager::Destroy(memManager, *newKey);
					throw;
				}
			};
			typename HashMap::Position resPos = mHashMap.AddCrt(pos, std::move(valueCreator));
			return { IteratorProxy(resPos), true };
		}
		catch (...)
		{
			if (!keyDestroyed)
				KeyManager::Destroy(memManager, *&keyBuffer);
			throw;
		}
	}

	template<typename Hint, typename RKey, typename MappedCreator,
		typename Key = std::decay_t<RKey>>
	requires std::is_same_v<key_type, Key>
	std::pair<iterator, bool> pvInsert(Hint /*hint*/, std::tuple<RKey>&& key,
		MappedCreator mappedCreator)
	{
		typename HashMap::InsertResult res = mHashMap.InsertCrt(
			std::forward<RKey>(std::get<0>(key)), std::move(mappedCreator));
		return { IteratorProxy(res.position), res.inserted };
	}

#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
	template<typename... KeyArgs, typename MappedCreator>
	std::pair<iterator, bool> pvInsert(const_iterator hint, std::tuple<KeyArgs...>&& keyArgs,
		MappedCreator mappedCreator)
	{
		MemManager& memManager = mHashMap.GetMemManager();
		typedef momo::internal::ObjectManager<key_type, MemManager> KeyManager;
		typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
		auto valueCreator = [&memManager, &keyArgs, mappedCreator = std::move(mappedCreator)]
			(key_type* newKey, mapped_type* newMapped) mutable
		{
			KeyCreator(memManager, std::move(keyArgs))(newKey);
			try
			{
				std::move(mappedCreator)(newMapped);
			}
			catch (...)
			{
				KeyManager::Destroy(memManager, *newKey);
				throw;
			}
		};
		typename HashMap::Position resPos = mHashMap.AddCrt(
			ConstIteratorProxy::GetBaseIterator(hint), std::move(valueCreator));
		return { IteratorProxy(resPos), true };
	}

	template<typename RKey, typename MappedCreator,
		typename Key = std::decay_t<RKey>>
	requires std::is_same_v<key_type, Key>
	std::pair<iterator, bool> pvInsert(const_iterator hint, std::tuple<RKey>&& key,
		MappedCreator mappedCreator)
	{
		typename HashMap::Position resPos = mHashMap.AddCrt(ConstIteratorProxy::GetBaseIterator(hint),
			std::forward<RKey>(std::get<0>(key)), std::move(mappedCreator));
		return { IteratorProxy(resPos), true };
	}
#endif

	template<typename RKey, typename MappedArg>
	std::pair<iterator, bool> pvInsertOrAssign(RKey&& key, MappedArg&& mappedArg)
	{
		typename HashMap::InsertResult res = mHashMap.InsertOrAssign(
			std::forward<RKey>(key), std::forward<MappedArg>(mappedArg));
		return { IteratorProxy(res.position), res.inserted };
	}

	template<typename RKey, typename MappedArg>
	iterator pvInsertOrAssign(const_iterator hint, RKey&& key, MappedArg&& mappedArg)
	{
		std::pair<iterator, bool> res = pvEmplace(hint,
			std::forward_as_tuple(std::forward<RKey>(key)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg)));
		if (!res.second)
		{
			HashMap::KeyValueTraits::AssignValue(mHashMap.GetMemManager(),
				std::forward<MappedArg>(mappedArg), res.first->second);
		}
		return res.first;
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
	typename THashFunc = HashCoder<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
class unordered_map_open : public unordered_map<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
	HashMap<TKey, TMapped, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketOpenDefault>,
		MemManagerStd<TAllocator>>>
{
private:
	typedef unordered_map<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
		momo::HashMap<TKey, TMapped, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketOpenDefault>,
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

	template<typename Predicate>
	requires std::predicate<const Predicate&, const_reference>
	friend size_type erase_if(unordered_map_open& cont, const Predicate& pred)
	{
		auto pairPred = [&pred] (const key_type& key, const mapped_type& mapped)
			{ return pred(const_reference(key, mapped)); };
		return cont.get_nested_container().Remove(pairPred);
	}
};

#define MOMO_DECLARE_DEDUCTION_GUIDES(unordered_map) \
template<typename Iterator, \
	typename Key = std::remove_const_t<typename std::iter_value_t<Iterator>::first_type>, \
	typename Mapped = typename std::iter_value_t<Iterator>::second_type> \
unordered_map(Iterator, Iterator) \
	-> unordered_map<Key, Mapped>; \
template<typename Iterator, \
	typename Key = std::remove_const_t<typename std::iter_value_t<Iterator>::first_type>, \
	typename Mapped = typename std::iter_value_t<Iterator>::second_type, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
requires momo::internal::conceptAllocator<Allocator> \
unordered_map(Iterator, Iterator, size_t, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename Iterator, typename HashFunc, \
	typename Key = std::remove_const_t<typename std::iter_value_t<Iterator>::first_type>, \
	typename Mapped = typename std::iter_value_t<Iterator>::second_type, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
requires momo::internal::conceptHashFunc<HashFunc, Key> && \
	momo::internal::conceptAllocator<Allocator> \
unordered_map(Iterator, Iterator, size_t, HashFunc, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, HashFunc, std::equal_to<Key>, Allocator>; \
template<typename Iterator, typename HashFunc, typename EqualFunc, \
	typename Key = std::remove_const_t<typename std::iter_value_t<Iterator>::first_type>, \
	typename Mapped = typename std::iter_value_t<Iterator>::second_type, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
requires momo::internal::conceptHashFunc<HashFunc, Key> && \
	momo::internal::conceptEqualFunc<EqualFunc, Key> && \
	momo::internal::conceptAllocator<Allocator> \
unordered_map(Iterator, Iterator, size_t, HashFunc, EqualFunc, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, HashFunc, EqualFunc, Allocator>; \
template<typename Key, typename Mapped> \
unordered_map(std::initializer_list<std::pair<Key, Mapped>>) \
	-> unordered_map<Key, Mapped>; \
template<typename Key, typename Mapped, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
requires momo::internal::conceptAllocator<Allocator> \
unordered_map(std::initializer_list<std::pair<Key, Mapped>>, size_t, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename Key, typename Mapped, typename HashFunc, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
requires momo::internal::conceptHashFunc<HashFunc, Key> && \
	momo::internal::conceptAllocator<Allocator> \
unordered_map(std::initializer_list<std::pair<Key, Mapped>>, size_t, HashFunc, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, HashFunc, std::equal_to<Key>, Allocator>; \
template<typename Key, typename Mapped, typename HashFunc, typename EqualFunc, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
requires momo::internal::conceptHashFunc<HashFunc, Key> && \
	momo::internal::conceptEqualFunc<EqualFunc, Key> && \
	momo::internal::conceptAllocator<Allocator> \
unordered_map(std::initializer_list<std::pair<Key, Mapped>>, size_t, HashFunc, EqualFunc, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, HashFunc, EqualFunc, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES(unordered_map)
MOMO_DECLARE_DEDUCTION_GUIDES(unordered_map_open)

#undef MOMO_DECLARE_DEDUCTION_GUIDES

} // namespace momo::stdish
