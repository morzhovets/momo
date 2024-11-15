/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/stdish/unordered_set.h

  namespace momo::stdish:
    class unordered_set
    class unordered_set_open

\**********************************************************/

#pragma once

#include "../HashSet.h"
#include "node_handle.h"

namespace momo::stdish
{

/*!
	\brief
	`momo::stdish::unordered_set` is similar to `std::unordered_set`, but
	much more efficient in memory usage. The implementation is based on
	hash table with buckets in the form of small arrays.

	\details
	Deviations from the `std::unordered_set`:
	1. Container items must be movable (preferably without exceptions)
	or copyable, similar to items of `std::vector`.
	2. After each addition or removal of the item all iterators and
	references to items become invalid and should not be used.
	3. Functions `clear`, `begin`, `cbegin` and iterator increment take
	O(bucket_count) time in worst case.
	4. If `ObjectManager<key_type>::isNothrowAnywayAssignable` is false,
	functions `erase` can throw exceptions.
	5. Functions `merge`, `extract` and `insert(node_type&&)` move items.

	It is allowed to pass to functions `insert` and `emplace` references
	to items within the container.
*/

template<typename TKey,
	typename THashFunc = HashCoder<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<TKey>,
	typename THashSet = HashSet<TKey, HashTraitsStd<TKey, THashFunc, TEqualFunc>,
		MemManagerStd<TAllocator>>>
class unordered_set
{
private:
	typedef THashSet HashSet;
	typedef typename HashSet::HashTraits HashTraits;
	typedef typename HashSet::MemManager MemManager;

public:
	typedef TKey key_type;
	typedef THashFunc hasher;
	typedef TEqualFunc key_equal;
	typedef TAllocator allocator_type;

	typedef HashSet nested_container_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef key_type value_type;

	typedef typename HashSet::ConstIterator const_iterator;
	typedef typename HashSet::Iterator iterator;

	//typedef typename iterator::Reference reference;
	typedef value_type& reference;
	typedef typename const_iterator::Reference const_reference;

	//typedef typename iterator::Pointer pointer;
	typedef value_type* pointer;
	typedef typename const_iterator::Pointer const_pointer;
	//typedef typename std::allocator_traits<allocator_type>::pointer pointer;
	//typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;

	typedef internal::set_node_handle<typename HashSet::ExtractedItem> node_type;
	typedef internal::insert_return_type<iterator, node_type> insert_return_type;

	typedef typename HashSet::ConstBucketBounds::Iterator const_local_iterator;
	typedef const_local_iterator local_iterator;

private:
	template<typename KeyArg>
	struct IsValidKeyArg : public HashTraits::template IsValidKeyArg<KeyArg>
	{
	};

	struct NodeTypeProxy : private node_type
	{
		typedef node_type NodeType;
		MOMO_DECLARE_PROXY_FUNCTION(NodeType, GetExtractedItem)
	};

public:
	unordered_set()
	{
	}

	explicit unordered_set(const allocator_type& alloc)
		: mHashSet(HashTraits(), MemManager(alloc))
	{
	}

	explicit unordered_set(size_type bucketCount, const allocator_type& alloc = allocator_type())
		: mHashSet(HashTraits(bucketCount), MemManager(alloc))
	{
	}

	unordered_set(size_type bucketCount, const hasher& hashFunc,
		const allocator_type& alloc = allocator_type())
		: mHashSet(HashTraits(bucketCount, hashFunc), MemManager(alloc))
	{
	}

	unordered_set(size_type bucketCount, const hasher& hashFunc, const key_equal& equalFunc,
		const allocator_type& alloc = allocator_type())
		: mHashSet(HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	unordered_set(Iterator first, Iterator last)
	{
		insert(first, last);
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	unordered_set(Iterator first, Iterator last, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_set(bucketCount, alloc)
	{
		insert(first, last);
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	unordered_set(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const allocator_type& alloc = allocator_type())
		: unordered_set(bucketCount, hashFunc, alloc)
	{
		insert(first, last);
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	unordered_set(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalFunc, const allocator_type& alloc = allocator_type())
		: unordered_set(bucketCount, hashFunc, equalFunc, alloc)
	{
		insert(first, last);
	}

	unordered_set(std::initializer_list<value_type> values)
		: mHashSet(values)
	{
	}

	unordered_set(std::initializer_list<value_type> values, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: mHashSet(values, HashTraits(bucketCount), MemManager(alloc))
	{
	}

	unordered_set(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const allocator_type& alloc = allocator_type())
		: mHashSet(values, HashTraits(bucketCount, hashFunc), MemManager(alloc))
	{
	}

	unordered_set(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalFunc,
		const allocator_type& alloc = allocator_type())
		: mHashSet(values, HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
	}

#if defined(__cpp_lib_containers_ranges)
	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_set(std::from_range_t, Range&& values)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_set(std::from_range_t, Range&& values, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_set(bucketCount, alloc)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_set(std::from_range_t, Range&& values, size_type bucketCount, const hasher& hashFunc,
		const allocator_type& alloc = allocator_type())
		: unordered_set(bucketCount, hashFunc, alloc)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	unordered_set(std::from_range_t, Range&& values, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalFunc, const allocator_type& alloc = allocator_type())
		: unordered_set(bucketCount, hashFunc, equalFunc, alloc)
	{
		insert_range(std::forward<Range>(values));
	}
#endif // __cpp_lib_containers_ranges

	unordered_set(unordered_set&& right) noexcept
		: mHashSet(std::move(right.mHashSet))
	{
	}

	unordered_set(unordered_set&& right, const std::type_identity_t<allocator_type>& alloc)
		noexcept(std::allocator_traits<allocator_type>::is_always_equal::value)
		: mHashSet(pvCreateSet(std::move(right), alloc))
	{
	}

	unordered_set(const unordered_set& right)
		: mHashSet(right.mHashSet)
	{
	}

	unordered_set(const unordered_set& right, const std::type_identity_t<allocator_type>& alloc)
		: mHashSet(right.mHashSet, MemManager(alloc))
	{
	}

	~unordered_set() noexcept = default;

	unordered_set& operator=(unordered_set&& right)
		noexcept(std::allocator_traits<allocator_type>::is_always_equal::value ||
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>::is_always_equal::value ||
				std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value;
			allocator_type alloc = (propagate ? &right : this)->get_allocator();
			mHashSet = pvCreateSet(std::move(right), alloc);
		}
		return *this;
	}

	unordered_set& operator=(const unordered_set& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>::is_always_equal::value ||
				std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value;
			allocator_type alloc = (propagate ? &right : this)->get_allocator();
			mHashSet = HashSet(right.mHashSet, MemManager(alloc));
		}
		return *this;
	}

	unordered_set& operator=(std::initializer_list<value_type> values)
	{
		mHashSet = HashSet(values, mHashSet.GetHashTraits(), MemManager(get_allocator()));
		return *this;
	}

	void swap(unordered_set& right) noexcept
	{
		MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
			|| get_allocator() == right.get_allocator());
		mHashSet.Swap(right.mHashSet);
	}

	friend void swap(unordered_set& left, unordered_set& right) noexcept
	{
		left.swap(right);
	}

	const nested_container_type& get_nested_container() const noexcept
	{
		return mHashSet;
	}

	nested_container_type& get_nested_container() noexcept
	{
		return mHashSet;
	}

	const_iterator begin() const noexcept
	{
		return mHashSet.GetBegin();
	}

	//iterator begin() noexcept

	const_iterator end() const noexcept
	{
		return mHashSet.GetEnd();
	}

	//iterator end() noexcept

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
		return mHashSet.GetHashTraits().GetMaxLoadFactor(HashSet::bucketMaxItemCount);
	}

	void max_load_factor(float maxLoadFactor)
	{
		if (maxLoadFactor == max_load_factor())
			return;
		if (maxLoadFactor <= 0.0 || maxLoadFactor > static_cast<float>(HashSet::bucketMaxItemCount))
			throw std::out_of_range("invalid load factor");
		HashTraits hashTraits(mHashSet.GetHashTraits(), maxLoadFactor);
		HashSet hashSet(hashTraits, MemManager(get_allocator()));
		hashSet.Reserve(size());
		hashSet.Insert(begin(), end());
		mHashSet = std::move(hashSet);
	}

	hasher hash_function() const
	{
		return mHashSet.GetHashTraits().GetHashFunc();
	}

	key_equal key_eq() const
	{
		return mHashSet.GetHashTraits().GetEqualFunc();
	}

	allocator_type get_allocator() const noexcept
	{
		return allocator_type(mHashSet.GetMemManager().GetByteAllocator());
	}

	size_type max_size() const noexcept
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const noexcept
	{
		return mHashSet.GetCount();
	}

	[[nodiscard]] bool empty() const noexcept
	{
		return mHashSet.IsEmpty();
	}

	void clear() noexcept
	{
		mHashSet.Clear();
	}

	void rehash(size_type bucketCount)
	{
		bucketCount = std::minmax(bucketCount, size_t{2}).second;
		bucketCount = std::bit_ceil(bucketCount);
		reserve(mHashSet.GetHashTraits().CalcCapacity(bucketCount, HashSet::bucketMaxItemCount));
	}

	void reserve(size_type count)
	{
		mHashSet.Reserve(count);
	}

	MOMO_FORCEINLINE const_iterator find(const key_type& key) const
	{
		return mHashSet.Find(key);
	}

	//iterator find(const key_type& key)

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE const_iterator find(const KeyArg& key) const
	{
		return mHashSet.Find(key);
	}

	//template<typename KeyArg>
	//requires IsValidKeyArg<KeyArg>::value
	//iterator find(const KeyArg& key)

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
		return mHashSet.ContainsKey(key);
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE bool contains(const KeyArg& key) const
	{
		return mHashSet.ContainsKey(key);
	}

	MOMO_FORCEINLINE std::pair<const_iterator, const_iterator> equal_range(
		const key_type& key) const
	{
		return { find(key), end() };
	}

	//std::pair<iterator, iterator> equal_range(const key_type& key)

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	MOMO_FORCEINLINE std::pair<const_iterator, const_iterator> equal_range(const KeyArg& key) const
	{
		return { find(key), end() };
	}

	//template<typename KeyArg>
	//requires IsValidKeyArg<KeyArg>::value
	//std::pair<iterator, iterator> equal_range(const KeyArg& key)

	std::pair<iterator, bool> insert(value_type&& value)
	{
		typename HashSet::InsertResult res = mHashSet.Insert(std::move(value));
		return { res.position, res.inserted };
	}

	iterator insert([[maybe_unused]] const_iterator hint, value_type&& value)
	{
#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
		return mHashSet.Add(hint, std::move(value));
#else
		return insert(std::move(value)).first;
#endif
	}

	std::pair<iterator, bool> insert(const value_type& value)
	{
		typename HashSet::InsertResult res = mHashSet.Insert(value);
		return { res.position, res.inserted };
	}

	iterator insert([[maybe_unused]] const_iterator hint, const value_type& value)
	{
#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
		return mHashSet.Add(hint, value);
#else
		return insert(value).first;
#endif
	}

	insert_return_type insert(node_type&& node)
	{
		if (node.empty())
			return { end(), false, node_type() };
		typename HashSet::InsertResult res = mHashSet.Insert(
			std::move(NodeTypeProxy::GetExtractedItem(node)));
		return { res.position, res.inserted, res.inserted ? node_type() : std::move(node) };
	}

	iterator insert([[maybe_unused]] const_iterator hint, node_type&& node)
	{
#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
		if (node.empty())
			return end();
		return mHashSet.Add(hint, std::move(NodeTypeProxy::GetExtractedItem(node)));
#else
		return insert(std::move(node)).position;
#endif
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	void insert(Iterator first, Iterator last)
	{
		pvInsertRange(first, last);
	}

	void insert(std::initializer_list<value_type> values)
	{
		mHashSet.Insert(values);
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	void insert_range(Range&& values)
	{
		pvInsertRange(std::ranges::begin(values), std::ranges::end(values));
	}

	template<typename... ValueArgs>
	std::pair<iterator, bool> emplace(ValueArgs&&... valueArgs)
	{
		MemManager& memManager = mHashSet.GetMemManager();
		typename HashSet::ExtractedItem extItem;
		typedef typename HashSet::ItemTraits::template Creator<ValueArgs...> ValueCreator;
		extItem.Create(ValueCreator(memManager, std::forward<ValueArgs>(valueArgs)...));
		typename HashSet::InsertResult res = mHashSet.Insert(std::move(extItem));
		return { res.position, res.inserted };
	}

	template<typename ValueArg>
	requires std::is_same_v<key_type, std::decay_t<ValueArg>>
	std::pair<iterator, bool> emplace(ValueArg&& valueArg)
	{
		typename HashSet::InsertResult res = mHashSet.InsertVar(
			std::as_const(valueArg), std::forward<ValueArg>(valueArg));
		return { res.position, res.inserted };
	}

	template<typename... ValueArgs>
	iterator emplace_hint([[maybe_unused]] const_iterator hint, ValueArgs&&... valueArgs)
	{
#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
		return mHashSet.AddVar(hint, std::forward<ValueArgs>(valueArgs)...);
#else
		return emplace(std::forward<ValueArgs>(valueArgs)...).first;
#endif
	}

	iterator erase(const_iterator where)
	{
		return mHashSet.Remove(where);
	}

	//iterator erase(iterator where)

	iterator erase(const_iterator first, const_iterator last)
	{
		if (first == begin() && last == end())
		{
			clear();
			return end();
		}
		if (first == last)
			return first;
		if (first != end() && std::next(first) == last)
			return erase(first);
		throw std::invalid_argument("invalid unordered_set erase arguments");
	}

	size_type erase(const key_type& key)
	{
		return mHashSet.Remove(key) ? 1 : 0;
	}

	template<typename KeyArg>
	requires (IsValidKeyArg<KeyArg>::value && !std::is_convertible_v<KeyArg&&, const_iterator>)
	size_type erase(KeyArg&& key)
	{
		iterator iter = find(std::forward<KeyArg>(key));
		if (iter == end())
			return 0;
		erase(iter);
		return 1;
	}

	template<momo::internal::conceptPredicate<const_reference> ValueFilter>
	friend size_type erase_if(unordered_set& cont, ValueFilter valueFilter)
	{
		return cont.mHashSet.Remove(momo::FastCopyableFunctor<ValueFilter>(valueFilter));
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
	requires (IsValidKeyArg<KeyArg>::value && !std::is_convertible_v<KeyArg&&, const_iterator>)
	node_type extract(KeyArg&& key)
	{
		iterator iter = find(std::forward<KeyArg>(key));
		return (iter != end()) ? extract(iter) : node_type();
	}

	template<typename Set>
	void merge(Set&& set)
	{
		mHashSet.MergeFrom(set.get_nested_container());
	}

	size_type max_bucket_count() const noexcept
	{
		return momo::internal::UIntConst::maxSize;
		//return momo::internal::HashSetBuckets<Bucket>::maxBucketCount;
	}

	size_type bucket_count() const noexcept
	{
		return mHashSet.GetBucketCount();
	}

	size_type bucket_size(size_type bucketIndex) const
	{
		return mHashSet.GetBucketBounds(bucketIndex).GetCount();
	}

	local_iterator begin(size_type bucketIndex)
	{
		return mHashSet.GetBucketBounds(bucketIndex).GetBegin();
	}

	const_local_iterator begin(size_type bucketIndex) const
	{
		return mHashSet.GetBucketBounds(bucketIndex).GetBegin();
	}

	local_iterator end(size_type bucketIndex)
	{
		return mHashSet.GetBucketBounds(bucketIndex).GetEnd();
	}

	const_local_iterator end(size_type bucketIndex) const
	{
		return mHashSet.GetBucketBounds(bucketIndex).GetEnd();
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
		return mHashSet.GetBucketIndex(key);
	}

	float load_factor() const noexcept
	{
		size_t count = size();
		size_t bucketCount = bucket_count();
		if (count == 0 && bucketCount == 0)
			return 0.0;
		return static_cast<float>(count) / static_cast<float>(bucketCount);
	}

	bool operator==(const unordered_set& right) const
	{
		if (size() != right.size())
			return false;
		for (const_reference ref : *this)
		{
			if (right.find(ref) == right.end())
				return false;
		}
		return true;
	}

private:
	static HashSet pvCreateSet(unordered_set&& right, const allocator_type& alloc)
	{
		if (right.get_allocator() == alloc)
			return std::move(right.mHashSet);
		HashSet hashSet(right.mHashSet.GetHashTraits(), MemManager(alloc));
		hashSet.MergeFrom(right.mHashSet);
		return hashSet;
	}

	template<std::input_iterator Iterator,
		momo::internal::conceptSentinel<Iterator> Sentinel>
	void pvInsertRange(Iterator begin, Sentinel end)
	{
		if constexpr (momo::internal::conceptSetArgIterator<Iterator, value_type>)
		{
			mHashSet.Insert(std::move(begin), std::move(end));
		}
		else
		{
			for (Iterator iter = std::move(begin); iter != end; ++iter)
				emplace(*iter);
		}
	}

private:
	HashSet mHashSet;
};

/*!
	\brief
	`momo::stdish::unordered_set_open` is similar to `std::unordered_set`,
	but much more efficient in operation speed. The implementation is based
	on open addressing hash table.

	\copydetails momo::stdish::unordered_set
*/

template<typename TKey,
	typename THashFunc = HashCoder<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<TKey>>
class unordered_set_open : public unordered_set<TKey, THashFunc, TEqualFunc, TAllocator,
	HashSet<TKey, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketOpenDefault>,
		MemManagerStd<TAllocator>>>
{
private:
	typedef unordered_set<TKey, THashFunc, TEqualFunc, TAllocator,
		momo::HashSet<TKey, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketOpenDefault>,
		MemManagerStd<TAllocator>>> UnorderedSet;

public:
	using typename UnorderedSet::size_type;
	using typename UnorderedSet::value_type;
	using typename UnorderedSet::const_reference;

public:
	using UnorderedSet::UnorderedSet;

	unordered_set_open& operator=(std::initializer_list<value_type> values)
	{
		UnorderedSet::operator=(values);
		return *this;
	}

	friend void swap(unordered_set_open& left, unordered_set_open& right) noexcept
	{
		left.swap(right);
	}

	template<momo::internal::conceptPredicate<const_reference> ValueFilter>
	friend size_type erase_if(unordered_set_open& cont, ValueFilter valueFilter)
	{
		return cont.get_nested_container().Remove(
			momo::FastCopyableFunctor<ValueFilter>(valueFilter));
	}
};

#define MOMO_DECLARE_DEDUCTION_GUIDES(unordered_set) \
template<typename Iterator, \
	typename Key = std::iter_value_t<Iterator>> \
unordered_set(Iterator, Iterator) \
	-> unordered_set<Key>; \
template<typename Iterator, \
	typename Key = std::iter_value_t<Iterator>, \
	typename Allocator = std::allocator<Key>> \
unordered_set(Iterator, Iterator, size_t, Allocator = Allocator()) \
	-> unordered_set<Key, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename Iterator, typename HashFunc, \
	typename Key = std::iter_value_t<Iterator>, \
	typename Allocator = std::allocator<Key>> \
unordered_set(Iterator, Iterator, size_t, HashFunc, Allocator = Allocator()) \
	-> unordered_set<Key, HashFunc, std::equal_to<Key>, Allocator>; \
template<typename Iterator, typename HashFunc, typename EqualFunc, \
	typename Key = std::iter_value_t<Iterator>, \
	typename Allocator = std::allocator<Key>> \
unordered_set(Iterator, Iterator, size_t, HashFunc, EqualFunc, Allocator = Allocator()) \
	-> unordered_set<Key, HashFunc, EqualFunc, Allocator>; \
template<typename Key> \
unordered_set(std::initializer_list<Key>) \
	-> unordered_set<Key>; \
template<typename Key, \
	typename Allocator = std::allocator<Key>> \
unordered_set(std::initializer_list<Key>, size_t, Allocator = Allocator()) \
	-> unordered_set<Key, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<typename Key, typename HashFunc, \
	typename Allocator = std::allocator<Key>> \
unordered_set(std::initializer_list<Key>, size_t, HashFunc, Allocator = Allocator()) \
	-> unordered_set<Key, HashFunc, std::equal_to<Key>, Allocator>; \
template<typename Key, typename HashFunc, typename EqualFunc, \
	typename Allocator = std::allocator<Key>> \
unordered_set(std::initializer_list<Key>, size_t, HashFunc, EqualFunc, Allocator = Allocator()) \
	-> unordered_set<Key, HashFunc, EqualFunc, Allocator>;

#define MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(unordered_set) \
template<std::ranges::input_range Range, \
	typename Key = std::ranges::range_value_t<Range>> \
unordered_set(std::from_range_t, Range&&) \
	-> unordered_set<Key>; \
template<std::ranges::input_range Range, \
	typename Key = std::ranges::range_value_t<Range>, \
	typename Allocator = std::allocator<Key>> \
unordered_set(std::from_range_t, Range&&, size_t, Allocator = Allocator()) \
	-> unordered_set<Key, HashCoder<Key>, std::equal_to<Key>, Allocator>; \
template<std::ranges::input_range Range, typename HashFunc, \
	typename Key = std::ranges::range_value_t<Range>, \
	typename Allocator = std::allocator<Key>> \
unordered_set(std::from_range_t, Range&&, size_t, HashFunc, Allocator = Allocator()) \
	-> unordered_set<Key, HashFunc, std::equal_to<Key>, Allocator>; \
template<std::ranges::input_range Range, typename HashFunc, typename EqualFunc, \
	typename Key = std::ranges::range_value_t<Range>, \
	typename Allocator = std::allocator<Key>> \
unordered_set(std::from_range_t, Range&&, size_t, HashFunc, EqualFunc, Allocator = Allocator()) \
	-> unordered_set<Key, HashFunc, EqualFunc, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES(unordered_set)
MOMO_DECLARE_DEDUCTION_GUIDES(unordered_set_open)

#if defined(__cpp_lib_containers_ranges)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(unordered_set)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(unordered_set_open)
#endif

#undef MOMO_DECLARE_DEDUCTION_GUIDES
#undef MOMO_DECLARE_DEDUCTION_GUIDES_RANGES

} // namespace momo::stdish
