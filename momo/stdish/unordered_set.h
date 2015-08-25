/**********************************************************\

  momo/stdish/unordered_set.h

  namespace momo::stdish:
    class unordered_set
    class unordered_set_open

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
	typedef typename THashSet::HashTraits HashTraits;
	typedef typename THashSet::MemManager MemManager;

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

	typedef value_type* pointer;
	//typedef typename iterator::Pointer pointer;
	typedef typename const_iterator::Pointer const_pointer;

	typedef value_type& reference;
	//typedef typename iterator::Reference reference;
	typedef typename const_iterator::Reference const_reference;

	typedef typename HashSet::ConstBucketBounds::Iterator const_local_iterator;
	typedef const_local_iterator local_iterator;

public:
	unordered_set()
	{
	}

	explicit unordered_set(const allocator_type& alloc)
		: mHashSet(HashTraits(), MemManager(alloc))
	{
	}

	explicit unordered_set(size_type bucketCount)
		: mHashSet(HashTraits(bucketCount))
	{
	}

	unordered_set(size_type bucketCount, const hasher& hashFunc)
		: mHashSet(HashTraits(bucketCount, hashFunc))
	{
	}

	unordered_set(size_type bucketCount, const hasher& hashFunc, const key_equal& equalFunc)
		: mHashSet(HashTraits(bucketCount, hashFunc, equalFunc))
	{
	}

	unordered_set(size_type bucketCount, const hasher& hashFunc, const key_equal& equalFunc,
		const allocator_type& alloc)
		: mHashSet(HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
	}

	template<typename Iterator>
	unordered_set(Iterator first, Iterator last)
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_set(Iterator first, Iterator last, size_type bucketCount)
		: mHashSet(HashTraits(bucketCount))
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_set(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc)
		: mHashSet(HashTraits(bucketCount, hashFunc))
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_set(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalFunc)
		: mHashSet(HashTraits(bucketCount, hashFunc, equalFunc))
	{
		insert(first, last);
	}

	template<typename Iterator>
	unordered_set(Iterator first, Iterator last, size_type bucketCount, const hasher& hashFunc,
		const key_equal& equalFunc, const allocator_type& alloc)
		: mHashSet(HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
		insert(first, last);
	}

#ifdef MOMO_USE_INIT_LISTS
	unordered_set(std::initializer_list<value_type> values)
	{
		insert(values.begin(), values.end());
	}

	unordered_set(std::initializer_list<value_type> values, size_type bucketCount)
		: mHashSet(HashTraits(bucketCount))
	{
		insert(values.begin(), values.end());
	}

	unordered_set(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc)
		: mHashSet(HashTraits(bucketCount, hashFunc))
	{
		insert(values.begin(), values.end());
	}

	unordered_set(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalFunc)
		: mHashSet(HashTraits(bucketCount, hashFunc, equalFunc))
	{
		insert(values.begin(), values.end());
	}

	unordered_set(std::initializer_list<value_type> values, size_type bucketCount,
		const hasher& hashFunc, const key_equal& equalFunc, const allocator_type& alloc)
		: mHashSet(HashTraits(bucketCount, hashFunc, equalFunc), MemManager(alloc))
	{
		insert(values.begin(), values.end());
	}
#endif

	unordered_set(unordered_set&& right) MOMO_NOEXCEPT
		: mHashSet(std::move(right.mHashSet))
	{
	}

	unordered_set(unordered_set&& right, const allocator_type& alloc)
		: mHashSet(right.mHashSet.GetHashTraits(), MemManager(alloc))
	{
		if (right.get_allocator() == alloc)
			*this = std::move(right);
		else
			insert(right.begin(), right.end());
		right.clear();
	}

	unordered_set(const unordered_set& right)
		: mHashSet(right.mHashSet)
	{
	}

	unordered_set(const unordered_set& right, const allocator_type& alloc)
		: mHashSet(right.mHashSet.GetHashTraits(), MemManager(alloc))
	{
		insert(right.begin(), right.end());
	}

	~unordered_set() MOMO_NOEXCEPT
	{
	}

	unordered_set& operator=(unordered_set&& right) MOMO_NOEXCEPT
	{
		mHashSet = std::move(right.mHashSet);
		return *this;
	}

	unordered_set& operator=(const unordered_set& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_copy_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			unordered_set(right, alloc).swap(*this);
		}
		return *this;
	}

#ifdef MOMO_USE_INIT_LISTS
	unordered_set& operator=(std::initializer_list<value_type> values)
	{
		clear();
		insert(values.begin(), values.end());
		return *this;
	}
#endif

	void swap(unordered_set& right) MOMO_NOEXCEPT
	{
		mHashSet.Swap(right.mHashSet);
	}

	friend void swap(unordered_set& left, unordered_set& right) MOMO_NOEXCEPT
	{
		left.swap(right);
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
		return mHashSet.GetHashTraits().GetMaxLoadFactor();
	}

	void max_load_factor(float maxLoadFactor)
	{
		size_t logStartBucketCount = internal::Log2<size_t>::Calc(bucket_count());
		HashTraits hashTraits(hash_function(), key_eq(), logStartBucketCount, maxLoadFactor);
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

	allocator_type get_allocator() const //MOMO_NOEXCEPT
	{
		return mHashSet.GetMemManager().GetAllocator();
	}

	size_type max_size() const //MOMO_NOEXCEPT
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
		size_t logBucketCount = internal::Log2<size_t>::Calc(bucketCount - 1) + 1;
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

	iterator find(const key_type& key)
	{
		return mHashSet.Find(key);
	}

	size_type count(const key_type& key) const
	{
		return (find(key) != end()) ? 1 : 0;
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

	std::pair<iterator, bool> insert(value_type&& value)
	{
		typename HashSet::InsertResult res = mHashSet.Insert(std::move(value));
		return std::pair<iterator, bool>(iterator(res.iterator), res.inserted);
	}

	iterator insert(const_iterator, value_type&& value)
	{
		return insert(std::move(value)).first;
	}

	std::pair<iterator, bool> insert(const value_type& value)
	{
		typename HashSet::InsertResult res = mHashSet.Insert(value);
		return std::pair<iterator, bool>(iterator(res.iterator), res.inserted);
	}

	iterator insert(const_iterator, const value_type& value)
	{
		return insert(value).first;
	}

	template<typename Iterator>
	void insert(Iterator first, Iterator last)
	{
#ifdef MOMO_USE_VARIADIC_TEMPLATES
		for (Iterator iter = first; iter != last; ++iter)
			emplace(*iter);
#else
		mHashSet.Insert(first, last);
#endif
	}

#ifdef MOMO_USE_INIT_LISTS
	void insert(std::initializer_list<value_type> values)
	{
		insert(values.begin(), values.end());
	}
#endif

#ifdef MOMO_USE_VARIADIC_TEMPLATES
	template<typename... Args>
	std::pair<iterator, bool> emplace(Args&&... args)
	{
		momo::internal::ObjectBuffer<value_type> buffer;
		new(&buffer) value_type(std::forward<Args>(args)...);
		std::pair<iterator, bool> res;
		try
		{
			res = insert(std::move(*&buffer));
		}
		catch (...)
		{
			HashSet::ItemTraits::Destroy(&buffer, 1);
			throw;
		}
		HashSet::ItemTraits::Destroy(&buffer, 1);
		return res;
	}

	template<typename... Args>
	iterator emplace_hint(const_iterator, Args&&... args)
	{
		return emplace(std::forward<Args>(args)...).first;
	}
#endif

	iterator erase(const_iterator where)
	{
		iterator resIter;
		mHashSet.Remove(where, resIter);
		return resIter;
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
		return (float)size() / (float)bucket_count();
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
	HashSet mHashSet;
};

#ifdef MOMO_USE_TYPE_ALIASES
template<typename TKey,
	typename THashFunc = std::hash<TKey>,
	typename TEqualFunc = std::equal_to<TKey>,
	typename TAllocator = std::allocator<TKey>>
using unordered_set_open = unordered_set<TKey, THashFunc, TEqualFunc, TAllocator,
	HashSet<TKey, HashTraitsStd<TKey, THashFunc, TEqualFunc, HashBucketOneI1>,
		MemManagerStd<TAllocator>>>;
#endif

} // namespace stdish

} // namespace momo
