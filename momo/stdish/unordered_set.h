/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/stdish/unordered_set.h

  namespace momo::stdish:
    class unordered_set
    class unordered_set_open

  This classes are similar to `std::unordered_set`.

  `unordered_set` is much more efficient than standard one in
  memory usage. Its implementation is based on hash table with
  buckets in the form of small arrays.
  `unordered_set_open` is based on open addressing hash table.

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

\**********************************************************/

#pragma once

#include "../HashSet.h"
#include "node_handle.h"

namespace momo
{

namespace stdish
{

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
	typedef TAllocator allocator_type;

	typedef typename HashTraits::HashFunc hasher;
	typedef typename HashTraits::EqualFunc key_equal;

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
	struct NodeTypeProxy : private node_type
	{
		typedef node_type NodeType;
		MOMO_DECLARE_PROXY_FUNCTION(NodeType, GetExtractedItem,
			typename NodeType::SetExtractedItem&)
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

	template<typename Iterator>
	unordered_set(Iterator first, Iterator last, const allocator_type& alloc = allocator_type())
		: unordered_set(alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_set(Iterator first, Iterator last, size_type bucketCount,
		const allocator_type& alloc = allocator_type())
		: unordered_set(bucketCount, alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_set(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const allocator_type& alloc = allocator_type())
		: unordered_set(bucketCount, hashFunc, alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_set(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalFunc, const allocator_type& alloc = allocator_type())
		: unordered_set(bucketCount, hashFunc, equalFunc, alloc)
	{
		insert(first, last);
	}

	unordered_set(std::initializer_list<value_type> values,
		const allocator_type& alloc = allocator_type())
		: mHashSet(values, HashTraits(), MemManager(alloc))
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

	unordered_set(unordered_set&& right) MOMO_NOEXCEPT
		: mHashSet(std::move(right.mHashSet))
	{
	}

	unordered_set(unordered_set&& right, const allocator_type& alloc)
		MOMO_NOEXCEPT_IF(momo::internal::IsAllocatorAlwaysEqual<allocator_type>::value)
		: mHashSet(pvCreateSet(std::move(right), alloc))
	{
	}

	unordered_set(const unordered_set& right)
		: mHashSet(right.mHashSet)
	{
	}

	unordered_set(const unordered_set& right, const allocator_type& alloc)
		: mHashSet(right.mHashSet, MemManager(alloc))
	{
	}

	~unordered_set() MOMO_NOEXCEPT
	{
	}

	unordered_set& operator=(unordered_set&& right)
		MOMO_NOEXCEPT_IF(momo::internal::IsAllocatorAlwaysEqual<allocator_type>::value ||
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
	{
		if (this != &right)
		{
			bool propagate = momo::internal::IsAllocatorAlwaysEqual<allocator_type>::value ||
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
			bool propagate = momo::internal::IsAllocatorAlwaysEqual<allocator_type>::value ||
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

	void swap(unordered_set& right) MOMO_NOEXCEPT
	{
		MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
			|| get_allocator() == right.get_allocator());
		mHashSet.Swap(right.mHashSet);
	}

	friend void swap(unordered_set& left, unordered_set& right) MOMO_NOEXCEPT
	{
		left.swap(right);
	}

	const nested_container_type& get_nested_container() const MOMO_NOEXCEPT
	{
		return mHashSet;
	}

	nested_container_type& get_nested_container() MOMO_NOEXCEPT
	{
		return mHashSet;
	}

	iterator begin() MOMO_NOEXCEPT
	{
		return mHashSet.GetBegin();
	}

	const_iterator begin() const MOMO_NOEXCEPT
	{
		return mHashSet.GetBegin();
	}

	iterator end() MOMO_NOEXCEPT
	{
		return mHashSet.GetEnd();
	}

	const_iterator end() const MOMO_NOEXCEPT
	{
		return mHashSet.GetEnd();
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

	allocator_type get_allocator() const MOMO_NOEXCEPT
	{
		return allocator_type(mHashSet.GetMemManager().GetCharAllocator());
	}

	size_type max_size() const MOMO_NOEXCEPT
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const MOMO_NOEXCEPT
	{
		return mHashSet.GetCount();
	}

	bool empty() const MOMO_NOEXCEPT
	{
		return mHashSet.IsEmpty();
	}

	void clear() MOMO_NOEXCEPT
	{
		mHashSet.Clear();
	}

	void rehash(size_type bucketCount)
	{
		bucketCount = std::minmax(bucketCount, (size_t)2).second;
		size_t logBucketCount = momo::internal::UIntMath<size_t>::Log2(bucketCount - 1) + 1;
		bucketCount = (size_t)1 << logBucketCount;
		reserve(mHashSet.GetHashTraits().CalcCapacity(bucketCount, HashSet::bucketMaxItemCount));
	}

	void reserve(size_type count)
	{
		mHashSet.Reserve(count);
	}

	const_iterator find(const key_type& key) const
	{
		return mHashSet.Find(key);
	}

	//iterator find(const key_type& key)

	size_type count(const key_type& key) const
	{
		return mHashSet.ContainsKey(key) ? 1 : 0;
	}

	bool contains(const key_type& key) const
	{
		return mHashSet.ContainsKey(key);
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		const_iterator iter = find(key);
		return std::pair<const_iterator, const_iterator>(iter,
			(iter != end()) ? std::next(iter) : iter);
	}

	//std::pair<iterator, iterator> equal_range(const key_type& key)

#ifdef MOMO_USE_UNORDERED_HETEROGENEOUS_LOOKUP
	template<typename KeyArg, typename H = hasher, typename = typename H::transparent_key_equal>
	const_iterator find(const KeyArg& key) const
	{
		return mHashSet.Find(key);
	}

	//template<typename KeyArg, typename H = hasher, typename = typename H::transparent_key_equal>
	//iterator find(const KeyArg& key)

	template<typename KeyArg, typename H = hasher, typename = typename H::transparent_key_equal>
	size_type count(const KeyArg& key) const
	{
		return mHashSet.ContainsKey(key) ? 1 : 0;
	}

	template<typename KeyArg, typename H = hasher, typename = typename H::transparent_key_equal>
	bool contains(const KeyArg& key) const
	{
		return mHashSet.ContainsKey(key);
	}

	template<typename KeyArg, typename H = hasher, typename = typename H::transparent_key_equal>
	std::pair<const_iterator, const_iterator> equal_range(const KeyArg& key) const
	{
		const_iterator iter = find(key);
		return std::pair<const_iterator, const_iterator>(iter,
			(iter != end()) ? std::next(iter) : iter);
	}

	//template<typename KeyArg, typename H = hasher, typename = typename H::transparent_key_equal>
	//std::pair<iterator, iterator> equal_range(const KeyArg& key)
#endif

	std::pair<iterator, bool> insert(value_type&& value)
	{
		typename HashSet::InsertResult res = mHashSet.Insert(std::move(value));
		return std::pair<iterator, bool>(res.iterator, res.inserted);
	}

	iterator insert(const_iterator hint, value_type&& value)
	{
#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
		return mHashSet.Add(hint, std::move(value));
#else
		(void)hint;
		return insert(std::move(value)).first;
#endif
	}

	std::pair<iterator, bool> insert(const value_type& value)
	{
		typename HashSet::InsertResult res = mHashSet.Insert(value);
		return std::pair<iterator, bool>(res.iterator, res.inserted);
	}

	iterator insert(const_iterator hint, const value_type& value)
	{
#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
		return mHashSet.Add(hint, value);
#else
		(void)hint;
		return insert(value).first;
#endif
	}

	insert_return_type insert(node_type&& node)
	{
		if (node.empty())
			return { end(), false, node_type() };
		typename HashSet::InsertResult res = mHashSet.Insert(
			std::move(NodeTypeProxy::GetExtractedItem(node)));
		return { res.iterator, res.inserted, res.inserted ? node_type() : std::move(node) };
	}

	iterator insert(const_iterator hint, node_type&& node)
	{
#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
		if (node.empty())
			return end();
		return mHashSet.Add(hint, std::move(NodeTypeProxy::GetExtractedItem(node)));
#else
		(void)hint;
		return insert(std::move(node)).position;
#endif
	}

	template<typename Iterator>
	void insert(Iterator first, Iterator last)
	{
		pvInsert(first, last,
			std::is_same<value_type, typename std::decay<decltype(*first)>::type>());
	}

	void insert(std::initializer_list<value_type> values)
	{
		mHashSet.Insert(values);
	}

	template<typename... ValueArgs>
	std::pair<iterator, bool> emplace(ValueArgs&&... valueArgs)
	{
		MemManager& memManager = mHashSet.GetMemManager();
		typename HashSet::ExtractedItem extItem;
		typedef typename HashSet::ItemTraits::template Creator<ValueArgs...> ValueCreator;
		extItem.Create(ValueCreator(memManager, std::forward<ValueArgs>(valueArgs)...));
		typename HashSet::InsertResult res = mHashSet.Insert(std::move(extItem));
		return std::pair<iterator, bool>(res.iterator, res.inserted);
	}

	template<typename... ValueArgs>
	iterator emplace_hint(const_iterator hint, ValueArgs&&... valueArgs)
	{
#ifdef MOMO_USE_UNORDERED_HINT_ITERATORS
		return mHashSet.AddVar(hint, std::forward<ValueArgs>(valueArgs)...);
#else
		(void)hint;
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
		if (std::next(first) == last)
			return erase(first);
		throw std::invalid_argument("invalid unordered_set erase arguments");
	}

	size_type erase(const key_type& key)
	{
		return mHashSet.Remove(key) ? 1 : 0;
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

	template<typename Set>
	void merge(Set&& set)
	{
		mHashSet.MergeFrom(set.get_nested_container());
	}

	size_type max_bucket_count() const MOMO_NOEXCEPT
	{
		return SIZE_MAX;
		//return momo::internal::HashSetBuckets<Bucket>::maxBucketCount;
	}

	size_type bucket_count() const MOMO_NOEXCEPT
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

	float load_factor() const MOMO_NOEXCEPT
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

	bool operator!=(const unordered_set& right) const
	{
		return !(*this == right);
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

	template<typename Iterator>
	void pvInsert(Iterator first, Iterator last, std::true_type /*isValueType*/)
	{
		mHashSet.Insert(first, last);
	}

	template<typename Iterator>
	void pvInsert(Iterator first, Iterator last, std::false_type /*isValueType*/)
	{
		for (Iterator iter = first; iter != last; ++iter)
			emplace(*iter);
	}

private:
	HashSet mHashSet;
};

template<typename TKey,
	typename THashFunc = HashCoder<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<TKey>>
using unordered_set_open = unordered_set<TKey, THashFunc, TEqualFunc, TAllocator,
	HashSet<TKey, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketOpenDefault>,
		MemManagerStd<TAllocator>>>;

} // namespace stdish

} // namespace momo
