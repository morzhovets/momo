/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/stdish/unordered_map.h

  namespace momo::stdish:
    class unordered_map_adaptor
    class unordered_map
    class unordered_map_open

\**********************************************************/

#pragma once

#include "../HashMap.h"
#include "node_handle.h"

namespace momo::stdish
{

template<typename THashMap>
class unordered_map_adaptor
	: public momo::internal::Swappable<unordered_map_adaptor>
{
private:
	typedef THashMap HashMap;
	typedef typename HashMap::HashTraits HashTraits;
	typedef typename HashMap::MemManager MemManager;

	typedef typename HashMap::Iterator HashMapIterator;

public:
	typedef typename HashMap::Key key_type;
	typedef typename HashMap::Value mapped_type;
	typedef typename HashTraits::Hasher hasher;
	typedef typename HashTraits::EqualComparer key_equal;

	typedef HashMap nested_container_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef std::pair<const key_type, mapped_type> value_type;
	typedef typename std::allocator_traits<typename MemManager::ByteAllocator>
		::template rebind_alloc<value_type> allocator_type;

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
	using IsValidKeyArg = HashTraits::template IsValidKeyArg<KeyArg>;

	struct ConstIteratorProxy : private const_iterator
	{
		typedef const_iterator ConstIterator;
		MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBaseIterator)
	};

	struct IteratorProxy : private iterator
	{
		typedef iterator Iterator;
		MOMO_DECLARE_PROXY_FUNCTION(Iterator, GetBaseIterator)
	};

	struct NodeTypeProxy : private node_type
	{
		typedef node_type NodeType;
		MOMO_DECLARE_PROXY_FUNCTION(NodeType, GetExtractedPair)
	};

public:
	unordered_map_adaptor()
	{
	}

	explicit unordered_map_adaptor(const allocator_type& alloc)
		: mHashMap(HashTraits(), MemManager(alloc))
	{
	}

	explicit unordered_map_adaptor(size_type bucketCount, const allocator_type& alloc = allocator_type())
		: mHashMap(HashTraits(bucketCount), MemManager(alloc))
	{
	}

	unordered_map_adaptor(size_type bucketCount, const hasher& hashFunc,
		const allocator_type& alloc = allocator_type())
		: mHashMap(HashTraits(bucketCount, hashFunc), MemManager(alloc))
	{
	}

	unordered_map_adaptor(size_type bucketCount, const hasher& hashFunc, const key_equal& equalComp,
		const allocator_type& alloc = allocator_type())
		: mHashMap(HashTraits(bucketCount, hashFunc, equalComp), MemManager(alloc))
	{
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	unordered_map_adaptor(Iterator first, Iterator last)
	{
		insert(first, last);
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	unordered_map_adaptor(Iterator first, Iterator last, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_map_adaptor(bucketCount, alloc)
	{
		insert(first, last);
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	unordered_map_adaptor(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const allocator_type& alloc = allocator_type())
		: unordered_map_adaptor(bucketCount, hashFunc, alloc)
	{
		insert(first, last);
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	unordered_map_adaptor(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalComp, const allocator_type& alloc = allocator_type())
		: unordered_map_adaptor(bucketCount, hashFunc, equalComp, alloc)
	{
		insert(first, last);
	}

	unordered_map_adaptor(std::initializer_list<value_type> values)
		: unordered_map_adaptor(values.begin(), values.end())
	{
	}

	unordered_map_adaptor(std::initializer_list<value_type> values, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_map_adaptor(values.begin(), values.end(), bucketCount, alloc)
	{
	}

	unordered_map_adaptor(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const allocator_type& alloc = allocator_type())
		: unordered_map_adaptor(values.begin(), values.end(), bucketCount, hashFunc, alloc)
	{
	}

	unordered_map_adaptor(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalComp,
		const allocator_type& alloc = allocator_type())
		: unordered_map_adaptor(values.begin(), values.end(), bucketCount, hashFunc, equalComp, alloc)
	{
	}

#if defined(__cpp_lib_containers_ranges)
	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_map_adaptor(std::from_range_t, Range&& values)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_map_adaptor(std::from_range_t, Range&& values, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_map_adaptor(bucketCount, alloc)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_map_adaptor(std::from_range_t, Range&& values, size_type bucketCount, const hasher& hashFunc,
		const allocator_type& alloc = allocator_type())
		: unordered_map_adaptor(bucketCount, hashFunc, alloc)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_map_adaptor(std::from_range_t, Range&& values, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalComp, const allocator_type& alloc = allocator_type())
		: unordered_map_adaptor(bucketCount, hashFunc, equalComp, alloc)
	{
		insert_range(std::forward<Range>(values));
	}
#endif // __cpp_lib_containers_ranges

	unordered_map_adaptor(unordered_map_adaptor&& right)
		: unordered_map_adaptor(std::move(right), right.get_allocator())
	{
	}

	unordered_map_adaptor(unordered_map_adaptor&& right, const allocator_type& alloc)
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

	unordered_map_adaptor(const unordered_map_adaptor& right)
		: mHashMap(right.mHashMap)
	{
	}

	unordered_map_adaptor(const unordered_map_adaptor& right, const allocator_type& alloc)
		: mHashMap(right.mHashMap, MemManager(alloc))
	{
	}

	~unordered_map_adaptor() noexcept = default;

	unordered_map_adaptor& operator=(unordered_map_adaptor&& right)
		noexcept(momo::internal::ContainerAssignerStd::isNothrowMoveAssignable<unordered_map_adaptor>)
	{
		return momo::internal::ContainerAssignerStd::Move(std::move(right), *this);
	}

	unordered_map_adaptor& operator=(const unordered_map_adaptor& right)
	{
		return momo::internal::ContainerAssignerStd::Copy(right, *this);
	}

	template<momo::internal::conceptMutableThis RUnorderedMap>
	std::remove_reference_t<RUnorderedMap>& operator=(this RUnorderedMap&& left,
		std::initializer_list<value_type> values)
	{
		left.mHashMap = HashMap(values, left.mHashMap.GetHashTraits(), MemManager(left.get_allocator()));
		return left;
	}

	void swap(unordered_map_adaptor& right) noexcept
	{
		momo::internal::ContainerAssignerStd::Swap(*this, right);
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
		return momo::internal::ProxyConstructor<const_iterator>(mHashMap.GetBegin());
	}

	iterator begin() noexcept
	{
		return momo::internal::ProxyConstructor<iterator>(mHashMap.GetBegin());
	}

	const_iterator end() const noexcept
	{
		return momo::internal::ProxyConstructor<const_iterator>(mHashMap.GetEnd());
	}

	iterator end() noexcept
	{
		return momo::internal::ProxyConstructor<iterator>(mHashMap.GetEnd());
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
			MOMO_THROW(std::out_of_range("invalid load factor"));
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
		bucketCount = momo::internal::UIntMath<>::Max(bucketCount, 2);
		bucketCount = std::bit_ceil(bucketCount);
		reserve(mHashMap.GetHashTraits().CalcCapacity(bucketCount, HashMap::bucketMaxItemCount));
	}

	void reserve(size_type count)
	{
		mHashMap.Reserve(count);
	}

	MOMO_FORCEINLINE const_iterator find(const key_type& key) const
	{
		return momo::internal::ProxyConstructor<const_iterator>(mHashMap.Find(key));
	}

	MOMO_FORCEINLINE iterator find(const key_type& key)
	{
		return momo::internal::ProxyConstructor<iterator>(mHashMap.Find(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE const_iterator find(const KeyArg& key) const
	{
		return momo::internal::ProxyConstructor<const_iterator>(mHashMap.Find(key));
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE iterator find(const KeyArg& key)
	{
		return momo::internal::ProxyConstructor<iterator>(mHashMap.Find(key));
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
		const_iterator iter = find(key);
		return { iter, (iter != end()) ? std::next(iter) : iter };
	}

	MOMO_FORCEINLINE std::pair<iterator, iterator> equal_range(const key_type& key)
	{
		iterator iter = find(key);
		return { iter, (iter != end()) ? std::next(iter) : iter };
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE std::pair<const_iterator, const_iterator> equal_range(const KeyArg& key) const
	{
		const_iterator iter = find(key);
		return { iter, (iter != end()) ? std::next(iter) : iter };
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE std::pair<iterator, iterator> equal_range(const KeyArg& key)
	{
		iterator iter = find(key);
		return { iter, (iter != end()) ? std::next(iter) : iter };
	}

	template<typename ValueArg = std::pair<key_type, mapped_type>>
	requires std::is_constructible_v<value_type, ValueArg&&>
	std::pair<iterator, bool> insert(ValueArg&& valueArg)
	{
		return emplace(std::forward<ValueArg>(valueArg));
	}

	template<typename ValueArg = std::pair<key_type, mapped_type>>
	requires std::is_constructible_v<value_type, ValueArg&&>
	iterator insert(const_iterator hint, ValueArg&& valueArg)
	{
		return emplace_hint(hint, std::forward<ValueArg>(valueArg));
	}

	insert_return_type insert(node_type&& node)
	{
		if (node.empty())
			return { end(), false, node_type() };
		typename HashMap::InsertResult res = mHashMap.Insert(
			std::move(NodeTypeProxy::GetExtractedPair(node)));
		return { momo::internal::ProxyConstructor<iterator>(res.position), res.inserted,
			res.inserted ? node_type() : std::move(node) };
	}

	iterator insert(const_iterator hint, node_type&& node)
	{
		if constexpr (HashTraits::useHintIterators)
		{
			if (node.empty())
				return end();
			return momo::internal::ProxyConstructor<iterator>(mHashMap.Add(
				ConstIteratorProxy::GetBaseIterator(hint), std::move(NodeTypeProxy::GetExtractedPair(node))));
		}
		else
		{
			return insert(std::move(node)).position;
		}
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	void insert(Iterator first, Iterator last)
	{
		pvInsertRange(first, last);
	}

	void insert(std::initializer_list<value_type> values)
	{
		mHashMap.Insert(values.begin(), values.end());
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	void insert_range(Range&& values)
	{
		pvInsertRange(std::ranges::begin(values), std::ranges::end(values));
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
		return momo::internal::ProxyConstructor<iterator>(
			mHashMap.Remove(ConstIteratorProxy::GetBaseIterator(where)));
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
			return momo::internal::ProxyConstructor<iterator>(
				mHashMap.MakeMutableIterator(ConstIteratorProxy::GetBaseIterator(first)));
		}
		if (first != end() && std::next(first) == last)
			return erase(first);
		MOMO_THROW(std::invalid_argument("invalid unordered_map erase arguments"));
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

	template<momo::internal::conceptPredicate<const_reference> ValueFilter>
	friend size_type erase_if(unordered_map_adaptor& cont, ValueFilter valueFilter)
	{
		momo::FastCopyableFunctor fastValueFilter(valueFilter);
		auto pairFilter = [fastValueFilter] (const key_type& key, const mapped_type& mapped)
			{ return fastValueFilter(const_reference(key, mapped)); };
		return cont.mHashMap.Remove(pairFilter);
	}

	MOMO_FORCEINLINE decltype(auto) operator[](key_type&& key)
	{
		return mHashMap[std::move(key)];
	}

	MOMO_FORCEINLINE decltype(auto) operator[](const key_type& key)
	{
		return mHashMap[key];
	}

	MOMO_FORCEINLINE const mapped_type& at(const key_type& key) const
	{
		const_iterator iter = find(key);
		if (iter == end())
			MOMO_THROW(std::out_of_range("invalid unordered_map key"));
		return iter->second;
	}

	MOMO_FORCEINLINE mapped_type& at(const key_type& key)
	{
		iterator iter = find(key);
		if (iter == end())
			MOMO_THROW(std::out_of_range("invalid unordered_map key"));
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
		return momo::internal::ProxyConstructor<local_iterator>(
			mHashMap.GetBucketBounds(bucketIndex).GetBegin());
	}

	const_local_iterator begin(size_type bucketIndex) const
	{
		return momo::internal::ProxyConstructor<const_local_iterator>(
			mHashMap.GetBucketBounds(bucketIndex).GetBegin());
	}

	local_iterator end(size_type bucketIndex)
	{
		return momo::internal::ProxyConstructor<local_iterator>(
			mHashMap.GetBucketBounds(bucketIndex).GetEnd());
	}

	const_local_iterator end(size_type bucketIndex) const
	{
		return momo::internal::ProxyConstructor<const_local_iterator>(
			mHashMap.GetBucketBounds(bucketIndex).GetEnd());
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

	bool operator==(const unordered_map_adaptor& right) const
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
	template<typename Hint, typename... KeyArgs, typename... MappedArgs>
	std::pair<iterator, bool> pvEmplace(Hint hint, std::tuple<KeyArgs...>&& keyArgs,
		std::tuple<MappedArgs...>&& mappedArgs)
	{
		typedef typename HashMap::KeyValueTraits
			::template ValueCreator<MappedArgs...> MappedCreator;
		return pvInsert(hint, std::move(keyArgs),
			FastMovableFunctor(MappedCreator(mHashMap.GetMemManager(), std::move(mappedArgs))));
	}

	template<typename Hint, typename... KeyArgs,
		momo::internal::conceptObjectCreator<mapped_type> MappedCreator>
	std::pair<iterator, bool> pvInsert(Hint hint, std::tuple<KeyArgs...>&& keyArgs,
		FastMovableFunctor<MappedCreator> mappedCreator)
	{
		MemManager& memManager = mHashMap.GetMemManager();
		typedef momo::internal::ObjectManager<key_type, MemManager> KeyManager;
		typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
		typedef typename KeyManager::template DestroyFinalizer<> KeyDestroyFinalizer;
		if constexpr (HashTraits::useHintIterators && !std::is_null_pointer_v<Hint>)
		{
			auto valueCreator = [&memManager, &keyArgs, mappedCreator = std::move(mappedCreator)]
				(key_type* newKey, mapped_type* newMapped) mutable
			{
				KeyCreator(memManager, std::move(keyArgs))(newKey);
				KeyDestroyFinalizer keyFin(&memManager, newKey);
				std::move(mappedCreator)(newMapped);
				keyFin.ResetPtr();
			};
			typename HashMap::Position resPos = mHashMap.AddCrt(
				ConstIteratorProxy::GetBaseIterator(hint), std::move(valueCreator));
			return { momo::internal::ProxyConstructor<iterator>(resPos), true };
		}
		else
		{
			typedef momo::internal::ObjectBuffer<key_type,
				HashMap::KeyValueTraits::keyAlignment> KeyBuffer;
			KeyBuffer keyBuffer;
			KeyCreator(memManager, std::move(keyArgs))(keyBuffer.GetPtr());
			KeyDestroyFinalizer keyFin(&memManager, keyBuffer.template GetPtr<true>());
			typename HashMap::Position pos = mHashMap.Find(std::as_const(keyBuffer.Get()));
			if (!!pos)
				return { momo::internal::ProxyConstructor<iterator>(pos), false };
			auto valueCreator = [mappedCreator = std::move(mappedCreator), keyFin = std::move(keyFin)]
				(key_type* newKey, mapped_type* newMapped) mutable
			{
				KeyManager::Relocate(*keyFin.GetMemManager(), *keyFin.GetPtr(), newKey);
				keyFin.ResetPtr(newKey);
				std::move(mappedCreator)(newMapped);
				keyFin.ResetPtr();
			};
			typename HashMap::Position resPos = mHashMap.AddCrt(pos, std::move(valueCreator));
			return { momo::internal::ProxyConstructor<iterator>(resPos), true };
		}
	}

	template<typename Hint, typename RKey,
		momo::internal::conceptObjectCreator<mapped_type> MappedCreator,
		typename Key = std::decay_t<RKey>>
	requires std::is_same_v<key_type, Key>
	std::pair<iterator, bool> pvInsert(Hint hint, std::tuple<RKey>&& key,
		FastMovableFunctor<MappedCreator> mappedCreator)
	{
		if constexpr (HashTraits::useHintIterators && !std::is_null_pointer_v<Hint>)
		{
			typename HashMap::Position resPos = mHashMap.AddCrt(
				ConstIteratorProxy::GetBaseIterator(hint),
				std::forward<RKey>(std::get<0>(key)), std::move(mappedCreator));
			return { momo::internal::ProxyConstructor<iterator>(resPos), true };
		}
		else
		{
			typename HashMap::InsertResult res = mHashMap.InsertCrt(
				std::forward<RKey>(std::get<0>(key)), std::move(mappedCreator));
			return { momo::internal::ProxyConstructor<iterator>(res.position), res.inserted };
		}
	}

	template<std::input_iterator Iterator,
		momo::internal::conceptSentinel<Iterator> Sentinel>
	void pvInsertRange(Iterator begin, Sentinel end)
	{
		if constexpr (momo::internal::conceptMapArgIterator<Iterator, key_type, false>)
		{
			mHashMap.Insert(std::move(begin), std::move(end));
		}
		else
		{
			for (Iterator iter = std::move(begin); iter != end; ++iter)
				insert(*iter);
		}
	}

	template<typename RKey, typename MappedArg>
	std::pair<iterator, bool> pvInsertOrAssign(RKey&& key, MappedArg&& mappedArg)
	{
		typename HashMap::InsertResult res = mHashMap.InsertOrAssign(
			std::forward<RKey>(key), std::forward<MappedArg>(mappedArg));
		return { momo::internal::ProxyConstructor<iterator>(res.position), res.inserted };
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
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
class MOMO_EMPTY_BASES unordered_map
	: public unordered_map_adaptor<HashMap<TKey, TMapped,
		HashTraitsStd<TKey, THasher, TEqualComparer, HashBucketDefault>, MemManagerStd<TAllocator>>>,
	public momo::internal::Swappable<unordered_map>
{
private:
	typedef unordered_map_adaptor<HashMap<TKey, TMapped,
		HashTraitsStd<TKey, THasher, TEqualComparer, HashBucketDefault>,
			MemManagerStd<TAllocator>>> UnorderedMapAdaptor;

public:
	using UnorderedMapAdaptor::UnorderedMapAdaptor;

	using UnorderedMapAdaptor::operator=;
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
class MOMO_EMPTY_BASES unordered_map_open
	: public unordered_map_adaptor<HashMap<TKey, TMapped,
		HashTraitsStd<TKey, THasher, TEqualComparer, HashBucketOpenDefault>, MemManagerStd<TAllocator>>>,
	public momo::internal::Swappable<unordered_map_open>
{
private:
	typedef unordered_map_adaptor<HashMap<TKey, TMapped,
		HashTraitsStd<TKey, THasher, TEqualComparer, HashBucketOpenDefault>,
			MemManagerStd<TAllocator>>> UnorderedMapAdaptor;

public:
	using UnorderedMapAdaptor::UnorderedMapAdaptor;

	using UnorderedMapAdaptor::operator=;
};

#define MOMO_DECLARE_DEDUCTION_GUIDES(unordered_map) \
template<typename Iterator, \
	typename Value = std::iter_value_t<Iterator>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>> \
unordered_map(Iterator, Iterator) \
	-> unordered_map<Key, Mapped>; \
template<typename Iterator, \
	typename Value = std::iter_value_t<Iterator>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_map(Iterator, Iterator, size_t, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename Iterator, \
	typename Value = std::iter_value_t<Iterator>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	momo::internal::conceptCopyableHasher<Key> Hasher, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_map(Iterator, Iterator, size_t, Hasher, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, Hasher, std::equal_to<Key>, Allocator>; \
template<typename Iterator, \
	typename Value = std::iter_value_t<Iterator>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	momo::internal::conceptCopyableHasher<Key> Hasher, \
	momo::internal::conceptCopyableEqualComparer<Key> EqualComparer, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_map(Iterator, Iterator, size_t, Hasher, EqualComparer, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, Hasher, EqualComparer, Allocator>; \
template<typename QKey, typename Mapped, \
	typename Key = std::remove_const_t<QKey>> \
unordered_map(std::initializer_list<std::pair<QKey, Mapped>>) \
	-> unordered_map<Key, Mapped>; \
template<typename QKey, typename Mapped, \
	typename Key = std::remove_const_t<QKey>, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_map(std::initializer_list<std::pair<QKey, Mapped>>, size_t, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename QKey, typename Mapped, \
	typename Key = std::remove_const_t<QKey>, \
	momo::internal::conceptCopyableHasher<Key> Hasher, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_map(std::initializer_list<std::pair<QKey, Mapped>>, size_t, Hasher, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, Hasher, std::equal_to<Key>, Allocator>; \
template<typename QKey, typename Mapped, \
	typename Key = std::remove_const_t<QKey>, \
	momo::internal::conceptCopyableHasher<Key> Hasher, \
	momo::internal::conceptCopyableEqualComparer<Key> EqualComparer, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_map(std::initializer_list<std::pair<QKey, Mapped>>, size_t, Hasher, EqualComparer, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, Hasher, EqualComparer, Allocator>; \
template<typename Key, typename Mapped, typename Hasher, typename EqualComparer, typename Allocator> \
unordered_map(unordered_map<Key, Mapped, Hasher, EqualComparer, Allocator>, std::type_identity_t<Allocator>) \
	-> unordered_map<Key, Mapped, Hasher, EqualComparer, Allocator>;

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
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_map(std::from_range_t, Range&&, size_t, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<std::ranges::input_range Range, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	momo::internal::conceptCopyableHasher<Key> Hasher, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_map(std::from_range_t, Range&&, size_t, Hasher, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, Hasher, std::equal_to<Key>, Allocator>; \
template<std::ranges::input_range Range, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	momo::internal::conceptCopyableHasher<Key> Hasher, \
	momo::internal::conceptCopyableEqualComparer<Key> EqualComparer, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
unordered_map(std::from_range_t, Range&&, size_t, Hasher, EqualComparer, Allocator = Allocator()) \
	-> unordered_map<Key, Mapped, Hasher, EqualComparer, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES(unordered_map)
MOMO_DECLARE_DEDUCTION_GUIDES(unordered_map_open)

#if defined(__cpp_lib_containers_ranges)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(unordered_map)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(unordered_map_open)
#endif

#undef MOMO_DECLARE_DEDUCTION_GUIDES
#undef MOMO_DECLARE_DEDUCTION_GUIDES_RANGES

} // namespace momo::stdish
