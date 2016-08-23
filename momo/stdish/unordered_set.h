/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/stdish/unordered_set.h

  namespace momo::stdish:
    class unordered_set
    class unordered_set_open

  This classes are similar to `std::unordered_set`.
  `unordered_set` is much more efficient than standard one in
  memory usage. Its implementation is based on hash tables with
  buckets in the form of small arrays.
  `unordered_set_open` is based on open addressing hash table.

  Deviations from the `std::unordered_set`:
  1. Container items must be movable (preferably without exceptions)
    or copyable, similar to items of `std::vector`.
  2. After each addition or removal of the item all iterators and
    references to items become invalid and should not be used.
  3. Functions `begin`, `cbegin` and iterator increment take
    O(bucket_count) time in worst case.
  4. Functions `erase` can throw exceptions thrown by `key_type`
    move assignment operator.
  5. Functions of the allocator `construct`, `destroy` and `address`
    are not used.

  It is allowed to pass to functions `insert` and `emplace` references
  to items within the container.

\**********************************************************/

#pragma once

#include "../HashSet.h"

namespace momo
{

namespace stdish
{

template<typename TKey,
	typename THashFunc = std::hash<TKey>,
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

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef key_type value_type;

	typedef typename HashSet::ConstIterator const_iterator;
	typedef const_iterator iterator;

	//typedef typename iterator::Reference reference;
	typedef value_type& reference;
	typedef typename const_iterator::Reference const_reference;

	//typedef typename iterator::Pointer pointer;
	typedef value_type* pointer;
	typedef typename const_iterator::Pointer const_pointer;
	//typedef typename std::allocator_traits<allocator_type>::pointer pointer;
	//typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;

	typedef typename HashSet::ConstBucketBounds::Iterator const_local_iterator;
	typedef const_local_iterator local_iterator;

private:
	typedef internal::ObjectBuffer<value_type, HashSet::ItemTraits::alignment> ValueBuffer;

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
		MOMO_NOEXCEPT_IF((std::is_same<allocator_type, std::allocator<value_type>>::value))
		: mHashSet(_create_set(std::move(right), alloc))
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
		MOMO_NOEXCEPT_IF((std::is_same<allocator_type, std::allocator<value_type>>::value) ||
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_move_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			mHashSet = _create_set(std::move(right), alloc);
		}
		return *this;
	}

	unordered_set& operator=(const unordered_set& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_copy_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			mHashSet = HashSet(right.mHashSet, MemManager(alloc));
		}
		return *this;
	}

	unordered_set& operator=(std::initializer_list<value_type> values)
	{
		clear();	//?
		insert(values);
		return *this;
	}

	void swap(unordered_set& right) MOMO_NOEXCEPT
	{
		MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
			|| get_allocator() == right.get_allocator());
		mHashSet.Swap(right.mHashSet);
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

	MOMO_FRIENDS_SWAP_BEGIN_END_STD(unordered_set)

	float max_load_factor() const MOMO_NOEXCEPT
	{
		return mHashSet.GetHashTraits().GetMaxLoadFactor();
	}

	void max_load_factor(float maxLoadFactor)
	{
		if (maxLoadFactor == max_load_factor())
			return;
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
		return mHashSet.GetMemManager().GetAllocator();
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
		size_t logBucketCount = internal::UIntMath<size_t>::Log2(bucketCount - 1) + 1;
		bucketCount = (size_t)1 << logBucketCount;
		reserve(mHashSet.GetHashTraits().CalcCapacity(bucketCount));
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
	//{
	//	return mHashSet.Find(key);
	//}

	size_type count(const key_type& key) const
	{
		return mHashSet.HasKey(key) ? 1 : 0;
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		const_iterator iter = find(key);
		if (iter == end())
			return std::pair<const_iterator, const_iterator>(end(), end());
		return std::pair<const_iterator, const_iterator>(iter, std::next(iter));
	}

	//std::pair<iterator, iterator> equal_range(const key_type& key)
	//{
	//	iterator iter = find(key);
	//	if (iter == end())
	//		return std::pair<iterator, iterator>(end(), end());
	//	return std::pair<iterator, iterator>(iter, std::next(iter));
	//}

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

	template<typename Iterator>
	void insert(Iterator first, Iterator last)
	{
		_insert(first, last,
			std::is_same<value_type, typename std::decay<decltype(*first)>::type>());
	}

	void insert(std::initializer_list<value_type> values)
	{
		mHashSet.Insert(values);
	}

	template<typename... ValueArgs>
	std::pair<iterator, bool> emplace(ValueArgs&&... valueArgs)
	{
		typedef typename HashSet::ItemTraits::template Creator<ValueArgs...> ValueCreator;
		ValueBuffer valueBuffer;
		ValueCreator(std::forward<ValueArgs>(valueArgs)...)(&valueBuffer);
		std::pair<iterator, bool> res;
		try
		{
			res = insert(std::move(*&valueBuffer));
		}
		catch (...)
		{
			HashSet::ItemTraits::Destroy(&valueBuffer, 1);
			throw;
		}
		HashSet::ItemTraits::Destroy(&valueBuffer, 1);
		return res;
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

	iterator erase(const_iterator first, const_iterator last)
	{
		if (first == end())
			return end();
		if (first == last)
			return find(*first);
		if (first == begin() && last == end())
		{
			clear();
			return end();
		}
		if (std::next(first) == last)
			return erase(first);
		throw std::invalid_argument("invalid unordered_set erase arguments");
	}

	size_type erase(const key_type& key)
	{
		return mHashSet.Remove(key) ? 1 : 0;
	}

	size_type max_bucket_count() const MOMO_NOEXCEPT
	{
		return SIZE_MAX;
		//return internal::HashSetBuckets<Bucket>::maxBucketCount;
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
			return 0;
		return (float)count / (float)bucketCount;
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
	static HashSet _create_set(unordered_set&& right, const allocator_type& alloc)
	{
		if (right.get_allocator() == alloc)
			return std::move(right.mHashSet);
		HashSet hashSet(right.mHashSet.GetHashTraits(), MemManager(alloc));
		hashSet.MergeFrom(right.mHashSet);
		return hashSet;
	}

	template<typename Iterator>
	void _insert(Iterator first, Iterator last, std::true_type /*isValueType*/)
	{
		mHashSet.Insert(first, last);
	}

	template<typename Iterator>
	void _insert(Iterator first, Iterator last, std::false_type /*isValueType*/)
	{
		for (Iterator iter = first; iter != last; ++iter)
			emplace(*iter);
	}

private:
	HashSet mHashSet;
};

template<typename TKey,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<TKey>>
using unordered_set_open = unordered_set<TKey, THashFunc, TEqualFunc, TAllocator,
	HashSet<TKey, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketOneI1>,
		MemManagerStd<TAllocator>>>;

} // namespace stdish

} // namespace momo
