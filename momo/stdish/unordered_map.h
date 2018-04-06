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
  4. Functions `begin`, `cbegin` and iterator increment take
    O(bucket_count) time in worst case.
  5. If `ObjectManager<key_type>::isNothrowAnywayAssignable` is false
    or `ObjectManager<mapped_type>::isNothrowAnywayAssignable` is false,
    functions `erase` can throw exceptions.

  It is allowed to pass to functions `insert` and `emplace` references
  to items within the container.

\**********************************************************/

#pragma once

#include "../HashMap.h"
#include "node_handle.h"

namespace momo
{

namespace stdish
{

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

	typedef momo::internal::MapReferenceStd<key_type, mapped_type,
		typename HashMapIterator::Reference> reference;
	typedef typename reference::ConstReference const_reference;

	typedef momo::internal::HashDerivedIterator<HashMapIterator, reference> iterator;
	typedef typename iterator::ConstIterator const_iterator;

	typedef typename iterator::Pointer pointer;
	typedef typename const_iterator::Pointer const_pointer;
	//typedef typename std::allocator_traits<allocator_type>::pointer pointer;
	//typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;

#ifdef MOMO_USE_NODE_HANDLE
	typedef internal::map_node_handle<typename HashMap::ExtractedPair> node_type;
	typedef internal::insert_return_type<iterator, node_type> insert_return_type;
#endif

	typedef momo::internal::HashDerivedIterator<typename HashMap::BucketBounds::Iterator,
		reference> local_iterator;
	typedef typename local_iterator::ConstIterator const_local_iterator;

private:
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
		MOMO_DECLARE_PROXY_FUNCTION(Iterator, GetBaseIterator, HashMapIterator)
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

#ifdef MOMO_USE_NODE_HANDLE
	struct NodeTypeProxy : private node_type
	{
		typedef node_type NodeType;
		MOMO_DECLARE_PROXY_FUNCTION(NodeType, GetExtractedPair,
			typename NodeType::MapExtractedPair&)
	};
#endif

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

	template<typename Iterator>
	unordered_map(Iterator first, Iterator last, const allocator_type& alloc = allocator_type())
		: unordered_map(alloc)
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
		const key_equal& equalFunc, const allocator_type& alloc = allocator_type())
		: unordered_map(bucketCount, hashFunc, equalFunc, alloc)
	{
		insert(first, last);
	}

	unordered_map(std::initializer_list<value_type> values,
		const allocator_type& alloc = allocator_type())
		: unordered_map(values.begin(), values.end(), alloc)
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

	unordered_map(unordered_map&& right) MOMO_NOEXCEPT
		: mHashMap(std::move(right.mHashMap))
	{
	}

	unordered_map(unordered_map&& right, const allocator_type& alloc)
		MOMO_NOEXCEPT_IF(momo::internal::IsAllocatorAlwaysEqual<allocator_type>::value)
		: mHashMap(pvCreateMap(std::move(right), alloc))
	{
	}

	unordered_map(const unordered_map& right)
		: mHashMap(right.mHashMap)
	{
	}

	unordered_map(const unordered_map& right, const allocator_type& alloc)
		: mHashMap(right.mHashMap, MemManager(alloc))
	{
	}

	~unordered_map() MOMO_NOEXCEPT
	{
	}

	unordered_map& operator=(unordered_map&& right)
		MOMO_NOEXCEPT_IF(momo::internal::IsAllocatorAlwaysEqual<allocator_type>::value ||
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_move_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			mHashMap = pvCreateMap(std::move(right), alloc);
		}
		return *this;
	}

	unordered_map& operator=(const unordered_map& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_copy_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			mHashMap = HashMap(right.mHashMap, MemManager(alloc));
		}
		return *this;
	}

	unordered_map& operator=(std::initializer_list<value_type> values)
	{
		HashMap hashMap(mHashMap.GetHashTraits(), MemManager(get_allocator()));
		hashMap.InsertFS(values.begin(), values.end());
		mHashMap = std::move(hashMap);
		return *this;
	}

	void swap(unordered_map& right) MOMO_NOEXCEPT
	{
		MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
			|| get_allocator() == right.get_allocator());
		mHashMap.Swap(right.mHashMap);
	}

	friend void swap(unordered_map& left, unordered_map& right) MOMO_NOEXCEPT
	{
		left.swap(right);
	}

	const nested_container_type& get_nested_container() const MOMO_NOEXCEPT
	{
		return mHashMap;
	}

	nested_container_type& get_nested_container() MOMO_NOEXCEPT
	{
		return mHashMap;
	}

	iterator begin() MOMO_NOEXCEPT
	{
		return IteratorProxy(mHashMap.GetBegin());
	}

	const_iterator begin() const MOMO_NOEXCEPT
	{
		return ConstIteratorProxy(mHashMap.GetBegin());
	}

	iterator end() MOMO_NOEXCEPT
	{
		return IteratorProxy(mHashMap.GetEnd());
	}

	const_iterator end() const MOMO_NOEXCEPT
	{
		return ConstIteratorProxy(mHashMap.GetEnd());
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
		if (maxLoadFactor == max_load_factor())
			return;
		HashTraits hashTraits(mHashMap.GetHashTraits(), maxLoadFactor);
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

	allocator_type get_allocator() const MOMO_NOEXCEPT
	{
		return allocator_type(mHashMap.GetMemManager().GetCharAllocator());
	}

	size_type max_size() const MOMO_NOEXCEPT
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
		size_t logBucketCount = momo::internal::UIntMath<size_t>::Log2(bucketCount - 1) + 1;
		bucketCount = (size_t)1 << logBucketCount;
		reserve(mHashMap.GetHashTraits().CalcCapacity(bucketCount));
	}

	void reserve(size_type count)
	{
		mHashMap.Reserve(count);
	}

	const_iterator find(const key_type& key) const
	{
		return ConstIteratorProxy(mHashMap.Find(key));
	}

	iterator find(const key_type& key)
	{
		return IteratorProxy(mHashMap.Find(key));
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
	//typename std::enable_if<std::is_constructible<value_type, Value>::value, std::pair<iterator, bool>>::type
	//insert(Value&& value)

	//template<typename Value>
	//typename std::enable_if<std::is_constructible<value_type, Value>::value, iterator>::type
	//insert(const_iterator hint, Value&& value)

	//std::pair<iterator, bool> insert(const value_type& value)

	//iterator insert(const_iterator hint, const value_type& value)

	std::pair<iterator, bool> insert(value_type&& value)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(value.first),
			std::forward_as_tuple(std::move(value.second)));
	}

	iterator insert(const_iterator hint, value_type&& value)
	{
		return pvEmplace(hint, std::forward_as_tuple(value.first),
			std::forward_as_tuple(std::move(value.second))).first;
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, const First&>::value
		&& std::is_constructible<mapped_type, const Second&>::value,
		std::pair<iterator, bool>>::type
	insert(const std::pair<First, Second>& value)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(value.first),
			std::forward_as_tuple(value.second));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, const First&>::value
		&& std::is_constructible<mapped_type, const Second&>::value, iterator>::type
	insert(const_iterator hint, const std::pair<First, Second>& value)
	{
		return pvEmplace(hint, std::forward_as_tuple(value.first),
			std::forward_as_tuple(value.second)).first;
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, First&&>::value
		&& std::is_constructible<mapped_type, Second&&>::value,
		std::pair<iterator, bool>>::type
	insert(std::pair<First, Second>&& value)
	{
		return pvEmplace(nullptr, std::forward_as_tuple(std::forward<First>(value.first)),
			std::forward_as_tuple(std::forward<Second>(value.second)));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_constructible<key_type, First&&>::value
		&& std::is_constructible<mapped_type, Second&&>::value, iterator>::type
	insert(const_iterator hint, std::pair<First, Second>&& value)
	{
		return pvEmplace(hint, std::forward_as_tuple(std::forward<First>(value.first)),
			std::forward_as_tuple(std::forward<Second>(value.second))).first;
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

#ifdef MOMO_USE_NODE_HANDLE
	insert_return_type insert(node_type&& node)
	{
		if (node.empty())
			return { end(), false, node_type() };
		typename HashMap::InsertResult res = mHashMap.Insert(
			std::move(NodeTypeProxy::GetExtractedPair(node)));
		return { IteratorProxy(res.iterator), res.inserted,
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

	node_type extract(const_iterator where)
	{
		return node_type(mHashMap, ConstIteratorProxy::GetBaseIterator(where));
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
#endif

	size_type max_bucket_count() const MOMO_NOEXCEPT
	{
		return SIZE_MAX;
		//return momo::internal::HashSetBuckets<Bucket>::maxBucketCount;
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

	float load_factor() const MOMO_NOEXCEPT
	{
		size_t count = size();
		size_t bucketCount = bucket_count();
		if (count == 0 && bucketCount == 0)
			return 0;
		return (float)count / (float)bucketCount;
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
		const MappedCreator& mappedCreator)
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
			HashMapIterator iter = mHashMap.Find(*&keyBuffer);
			if (!!iter)
			{
				KeyManager::Destroy(memManager, *&keyBuffer);
				keyDestroyed = true;
				return std::pair<iterator, bool>(IteratorProxy(iter), false);
			}
			auto valueCreator = [&memManager, &keyBuffer, &mappedCreator, &keyDestroyed]
				(key_type* newKey, mapped_type* newMapped)
			{
				KeyManager::Relocate(memManager, *&keyBuffer, newKey);
				keyDestroyed = true;
				try
				{
					mappedCreator(newMapped);
				}
				catch (...)
				{
					KeyManager::Destroy(memManager, *newKey);
					throw;
				}
			};
			HashMapIterator resIter = mHashMap.AddCrt(iter, valueCreator);
			return std::pair<iterator, bool>(IteratorProxy(resIter), true);
		}
		catch (...)
		{
			if (!keyDestroyed)
				KeyManager::Destroy(memManager, *&keyBuffer);
			throw;
		}
	}

	template<typename Hint, typename RKey, typename MappedCreator,
		typename Key = typename std::decay<RKey>::type,
		typename = typename std::enable_if<std::is_same<key_type, Key>::value>::type>
	std::pair<iterator, bool> pvInsert(Hint /*hint*/, std::tuple<RKey>&& key,
		const MappedCreator& mappedCreator)
	{
		typename HashMap::InsertResult res = mHashMap.InsertCrt(
			std::forward<RKey>(std::get<0>(key)), mappedCreator);
		return std::pair<iterator, bool>(IteratorProxy(res.iterator), res.inserted);
	}

#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
	template<typename... KeyArgs, typename MappedCreator>
	std::pair<iterator, bool> pvInsert(const_iterator hint, std::tuple<KeyArgs...>&& keyArgs,
		const MappedCreator& mappedCreator)
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
				mappedCreator(newMapped);
			}
			catch (...)
			{
				KeyManager::Destroy(memManager, *newKey);
				throw;
			}
		};
		HashMapIterator resIter = mHashMap.AddCrt(
			ConstIteratorProxy::GetBaseIterator(hint), valueCreator);
		return std::pair<iterator, bool>(IteratorProxy(resIter), true);
	}

	template<typename RKey, typename MappedCreator,
		typename Key = typename std::decay<RKey>::type,
		typename = typename std::enable_if<std::is_same<key_type, Key>::value>::type>
	std::pair<iterator, bool> pvInsert(const_iterator hint, std::tuple<RKey>&& key,
		const MappedCreator& mappedCreator)
	{
		HashMapIterator resIter = mHashMap.AddCrt(ConstIteratorProxy::GetBaseIterator(hint),
			std::forward<RKey>(std::get<0>(key)), mappedCreator);
		return std::pair<iterator, bool>(IteratorProxy(resIter), true);
	}
#endif

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

#ifdef MOMO_HAS_DEDUCTION_GUIDES
template<typename Iterator,
	typename Allocator = std::allocator<std::pair<
		std::add_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>,
		typename std::iterator_traits<Iterator>::value_type::second_type>>>
unordered_map(Iterator, Iterator, Allocator = Allocator())
	-> unordered_map<std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>,
		typename std::iterator_traits<Iterator>::value_type::second_type,
		HashCoder<std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>>,
		std::equal_to<std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>>,
		Allocator>;

template<typename Iterator,
	typename Allocator = std::allocator<std::pair<
		std::add_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>,
		typename std::iterator_traits<Iterator>::value_type::second_type>>>
unordered_map(Iterator, Iterator, size_t, Allocator = Allocator())
	-> unordered_map<std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>,
		typename std::iterator_traits<Iterator>::value_type::second_type,
		HashCoder<std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>>,
		std::equal_to<std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>>,
		Allocator>;

template<typename Iterator, typename HashFunc,
	typename Allocator = std::allocator<std::pair<
		std::add_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>,
		typename std::iterator_traits<Iterator>::value_type::second_type>>>
unordered_map(Iterator, Iterator, size_t, HashFunc, Allocator = Allocator())
	-> unordered_map<std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>,
		typename std::iterator_traits<Iterator>::value_type::second_type, HashFunc,
		std::equal_to<std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>>,
		Allocator>;

template<typename Iterator, typename HashFunc, typename EqualFunc,
	typename Allocator = std::allocator<std::pair<
		std::add_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>,
		typename std::iterator_traits<Iterator>::value_type::second_type>>>
unordered_map(Iterator, Iterator, size_t, HashFunc, EqualFunc, Allocator = Allocator())
	-> unordered_map<std::remove_const_t<typename std::iterator_traits<Iterator>::value_type::first_type>,
		typename std::iterator_traits<Iterator>::value_type::second_type, HashFunc, EqualFunc, Allocator>;

template<typename Key, typename Mapped,
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>>
unordered_map(std::initializer_list<std::pair<const Key, Mapped>>, Allocator = Allocator())
	-> unordered_map<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>;

template<typename Key, typename Mapped,
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>>
unordered_map(std::initializer_list<std::pair<const Key, Mapped>>, size_t, Allocator = Allocator())
	-> unordered_map<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>;

template<typename Key, typename Mapped, typename HashFunc,
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>>
unordered_map(std::initializer_list<std::pair<const Key, Mapped>>, size_t, HashFunc,
	Allocator = Allocator())
	-> unordered_map<Key, Mapped, HashFunc, std::equal_to<Key>, Allocator>;

template<typename Key, typename Mapped, typename HashFunc, typename EqualFunc,
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>>
unordered_map(std::initializer_list<std::pair<const Key, Mapped>>, size_t, HashFunc, EqualFunc,
	Allocator = Allocator())
	-> unordered_map<Key, Mapped, HashFunc, EqualFunc, Allocator>;
#endif

template<typename TKey, typename TMapped,
	typename THashFunc = HashCoder<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
using unordered_map_open = unordered_map<TKey, TMapped, THashFunc, TEqualFunc, TAllocator,
	HashMap<TKey, TMapped, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketOpenDefault>,
		MemManagerStd<TAllocator>>>;

} // namespace stdish

} // namespace momo
