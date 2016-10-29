/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/stdish/set.h

  namespace momo::stdish:
    class set

  This class is similar to `std::set`, but much more efficient in
  memory usage and operation speed.
  The implementation is based on a B-tree.

  Deviations from the `std::set`:
  1. Container items must be movable (preferably without exceptions)
    or copyable, similar to items of `std::vector`.
  2. After each addition or removal of the item all iterators and
    references to items become invalid and should not be used.
  3. Functions `begin`, `cbegin`, `rend` and `crend` have logarithmic
    complexity.
  4. If `ObjectManager<key_type>::isNothrowAnywayAssignable` is false,
    functions `erase` can throw exceptions.

  It is allowed to pass to functions `insert` and `emplace` references
  to items within the container.

\**********************************************************/

#pragma once

#include "../TreeSet.h"

namespace momo
{

namespace stdish
{

template<typename TKey,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<TKey>,
	typename TTreeSet = TreeSet<TKey, TreeTraitsStd<TKey, TLessFunc>, MemManagerStd<TAllocator>>>
class set
{
private:
	typedef TTreeSet TreeSet;
	typedef typename TreeSet::TreeTraits TreeTraits;
	typedef typename TreeSet::MemManager MemManager;

public:
	typedef TKey key_type;
	typedef TLessFunc key_compare;
	typedef TAllocator allocator_type;

	typedef TreeSet nested_container_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef key_type value_type;
	typedef key_compare value_compare;

	typedef typename TreeSet::ConstIterator const_iterator;
	typedef const_iterator iterator;

	//typedef typename iterator::Reference reference;
	typedef value_type& reference;
	typedef typename const_iterator::Reference const_reference;

	//typedef typename iterator::Pointer pointer;
	typedef value_type* pointer;
	typedef typename const_iterator::Pointer const_pointer;
	//typedef typename std::allocator_traits<allocator_type>::pointer pointer;
	//typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
	set()
	{
	}

	explicit set(const allocator_type& alloc)
		: mTreeSet(TreeTraits(), MemManager(alloc))
	{
	}

	explicit set(const key_compare& lessFunc, const allocator_type& alloc = allocator_type())
		: mTreeSet(TreeTraits(lessFunc), MemManager(alloc))
	{
	}

	template<typename Iterator>
	set(Iterator first, Iterator last, const allocator_type& alloc = allocator_type())
		: set(alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	set(Iterator first, Iterator last, const key_compare& lessFunc,
		const allocator_type& alloc = allocator_type())
		: set(lessFunc, alloc)
	{
		insert(first, last);
	}

	set(std::initializer_list<value_type> values, const allocator_type& alloc = allocator_type())
		: mTreeSet(values, TreeTraits(), MemManager(alloc))
	{
	}

	set(std::initializer_list<value_type> values, const key_compare& lessFunc,
		const allocator_type& alloc = allocator_type())
		: mTreeSet(values, TreeTraits(lessFunc), MemManager(alloc))
	{
	}

	set(set&& right) MOMO_NOEXCEPT
		: mTreeSet(std::move(right.mTreeSet))
	{
	}

	set(set&& right, const allocator_type& alloc)
		MOMO_NOEXCEPT_IF((std::is_same<allocator_type, std::allocator<value_type>>::value))
		: mTreeSet(pvCreateSet(std::move(right), alloc))
	{
	}

	set(const set& right)
		: mTreeSet(right.mTreeSet)
	{
	}

	set(const set& right, const allocator_type& alloc)
		: mTreeSet(right.mTreeSet, MemManager(alloc))
	{
	}

	~set() MOMO_NOEXCEPT
	{
	}

	set& operator=(set&& right)
		MOMO_NOEXCEPT_IF((std::is_same<allocator_type, std::allocator<value_type>>::value) ||
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_move_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			mTreeSet = pvCreateSet(std::move(right), alloc);
		}
		return *this;
	}

	set& operator=(const set& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_copy_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			mTreeSet = TreeSet(right.mTreeSet, MemManager(alloc));
		}
		return *this;
	}

	set& operator=(std::initializer_list<value_type> values)
	{
		clear();	//?
		insert(values);
		return *this;
	}

	void swap(set& right) MOMO_NOEXCEPT
	{
		MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
			|| get_allocator() == right.get_allocator());
		mTreeSet.Swap(right.mTreeSet);
	}

	friend void swap(set& left, set& right) MOMO_NOEXCEPT
	{
		left.swap(right);
	}

	const nested_container_type& get_nested_container() const MOMO_NOEXCEPT
	{
		return mTreeSet;
	}

	nested_container_type& get_nested_container() MOMO_NOEXCEPT
	{
		return mTreeSet;
	}

	iterator begin() MOMO_NOEXCEPT
	{
		return mTreeSet.GetBegin();
	}

	const_iterator begin() const MOMO_NOEXCEPT
	{
		return mTreeSet.GetBegin();
	}

	iterator end() MOMO_NOEXCEPT
	{
		return mTreeSet.GetEnd();
	}

	const_iterator end() const MOMO_NOEXCEPT
	{
		return mTreeSet.GetEnd();
	}

	reverse_iterator rbegin() MOMO_NOEXCEPT
	{
		return reverse_iterator(end());
	}

	const_reverse_iterator rbegin() const MOMO_NOEXCEPT
	{
		return const_reverse_iterator(end());
	}

	reverse_iterator rend() MOMO_NOEXCEPT
	{
		return reverse_iterator(begin());
	}

	const_reverse_iterator rend() const MOMO_NOEXCEPT
	{
		return const_reverse_iterator(begin());
	}

	const_iterator cbegin() const MOMO_NOEXCEPT
	{
		return begin();
	}

	const_iterator cend() const MOMO_NOEXCEPT
	{
		return end();
	}

	const_reverse_iterator crbegin() const MOMO_NOEXCEPT
	{
		return rbegin();
	}

	const_reverse_iterator crend() const MOMO_NOEXCEPT
	{
		return rend();
	}

	key_compare key_comp() const
	{
		return mTreeSet.GetTreeTraits().GetLessFunc();
	}

	value_compare value_comp() const
	{
		return key_comp();
	}

	allocator_type get_allocator() const MOMO_NOEXCEPT
	{
		return allocator_type(mTreeSet.GetMemManager().GetCharAllocator());
	}

	size_type max_size() const MOMO_NOEXCEPT
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const MOMO_NOEXCEPT
	{
		return mTreeSet.GetCount();
	}

	bool empty() const MOMO_NOEXCEPT
	{
		return mTreeSet.IsEmpty();
	}

	void clear() MOMO_NOEXCEPT
	{
		mTreeSet.Clear();
	}

	const_iterator find(const key_type& key) const
	{
		return mTreeSet.Find(key);
	}

	//iterator find(const key_type& key)
	//{
	//	return mTreeSet.Find(key);
	//}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	const_iterator find(const KeyArg& key) const
	{
		return mTreeSet.Find(key);
	}

	//template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	//iterator find(const KeyArg& key)
	//{
	//	return mTreeSet.Find(key);
	//}

	size_type count(const key_type& key) const
	{
		return mTreeSet.HasKey(key) ? 1 : 0;
	}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	size_type count(const KeyArg& key) const
	{
		return mTreeSet.HasKey(key) ? 1 : 0;
	}

	const_iterator lower_bound(const key_type& key) const
	{
		return mTreeSet.LowerBound(key);
	}

	//iterator lower_bound(const key_type& key)
	//{
	//	return mTreeSet.LowerBound(key);
	//}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	const_iterator lower_bound(const KeyArg& key) const
	{
		return mTreeSet.LowerBound(key);
	}

	//template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	//iterator lower_bound(const KeyArg& key)
	//{
	//	return mTreeSet.LowerBound(key);
	//}

	const_iterator upper_bound(const key_type& key) const
	{
		return mTreeSet.UpperBound(key);
	}

	//iterator upper_bound(const key_type& key)
	//{
	//	return mTreeSet.UpperBound(key);
	//}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	const_iterator upper_bound(const KeyArg& key) const
	{
		return mTreeSet.UpperBound(key);
	}

	//template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	//iterator upper_bound(const KeyArg& key)
	//{
	//	return mTreeSet.UpperBound(key);
	//}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		return equal_range<key_type, key_compare, void>(key);
	}

	//std::pair<iterator, iterator> equal_range(const key_type& key)
	//{
	//	return equal_range<key_type, key_compare, void>(key);
	//}

	template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	std::pair<const_iterator, const_iterator> equal_range(const KeyArg& key) const
	{
		const_iterator iter = lower_bound(key);
		if (iter == end() || mTreeSet.GetTreeTraits().IsLess(key, *iter))
			return std::pair<const_iterator, const_iterator>(iter, iter);
		return std::pair<const_iterator, const_iterator>(iter, std::next(iter));
	}

	//template<typename KeyArg, typename KC = key_compare, typename = typename KC::is_transparent>
	//std::pair<iterator, iterator> equal_range(const KeyArg& key)
	//{
	//	return static_cast<const set*>(this)->equal_range(key);
	//}

	std::pair<iterator, bool> insert(value_type&& value)
	{
		typename TreeSet::InsertResult res = mTreeSet.Insert(std::move(value));
		return std::pair<iterator, bool>(res.iterator, res.inserted);
	}

	iterator insert(const_iterator hint, value_type&& value)
	{
		if (!pvCheckHint(hint, static_cast<const key_type&>(value)))
			return insert(std::move(value)).first;
		return mTreeSet.Add(hint, std::move(value));
	}

	std::pair<iterator, bool> insert(const value_type& value)
	{
		typename TreeSet::InsertResult res = mTreeSet.Insert(value);
		return std::pair<iterator, bool>(res.iterator, res.inserted);
	}

	iterator insert(const_iterator hint, const value_type& value)
	{
		if (!pvCheckHint(hint, value))
			return insert(value).first;
		return mTreeSet.Add(hint, value);
	}

	template<typename Iterator>
	void insert(Iterator first, Iterator last)
	{
		pvInsert(first, last,
			std::is_same<value_type, typename std::decay<decltype(*first)>::type>());
	}

	void insert(std::initializer_list<value_type> values)
	{
		mTreeSet.Insert(values);
	}

	template<typename... ValueArgs>
	std::pair<iterator, bool> emplace(ValueArgs&&... valueArgs)
	{
		return pvEmplace(nullptr, std::forward<ValueArgs>(valueArgs)...);
	}

	template<typename... ValueArgs>
	iterator emplace_hint(const_iterator hint, ValueArgs&&... valueArgs)
	{
		return pvEmplace(hint, std::forward<ValueArgs>(valueArgs)...).first;
	}

	iterator erase(const_iterator where)
	{
		return mTreeSet.Remove(where);
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		if (first == begin() && last == end())
		{
			clear();
			return end();
		}
		size_t count = std::distance(first, last);
		iterator iter = first;
		for (size_t i = 0; i < count; ++i)
			iter = erase(iter);
		return iter;
	}

	size_type erase(const key_type& key)
	{
		return mTreeSet.Remove(key) ? 1 : 0;
	}

	bool operator==(const set& right) const
	{
		return size() == right.size() && std::equal(begin(), end(), right.begin());
	}

	bool operator!=(const set& right) const
	{
		return !(*this == right);
	}

	bool operator<(const set& right) const
	{
		return std::lexicographical_compare(begin(), end(), right.begin(), right.end());
	}

	bool operator>(const set& right) const
	{
		return right < *this;
	}

	bool operator<=(const set& right) const
	{
		return !(right < *this);
	}

	bool operator>=(const set& right) const
	{
		return right <= *this;
	}

private:
	static TreeSet pvCreateSet(set&& right, const allocator_type& alloc)
	{
		if (right.get_allocator() == alloc)
			return std::move(right.mTreeSet);
		TreeSet treeSet(right.mTreeSet.GetTreeTraits(), MemManager(alloc));
		treeSet.MergeFrom(right.mTreeSet);
		return treeSet;
	}

	bool pvCheckHint(const_iterator hint, const key_type& key) const
	{
		const TreeTraits& treeTraits = mTreeSet.GetTreeTraits();
		return (hint == begin() || treeTraits.IsLess(*std::prev(hint), key))
			&& (hint == end() || treeTraits.IsLess(key, *hint));
	}

	template<typename Iterator>
	void pvInsert(Iterator first, Iterator last, std::true_type /*isValueType*/)
	{
		mTreeSet.Insert(first, last);
	}

	template<typename Iterator>
	void pvInsert(Iterator first, Iterator last, std::false_type /*isValueType*/)
	{
		for (Iterator iter = first; iter != last; ++iter)
			emplace(*iter);
	}

	template<typename ValueCreator>
	std::pair<iterator, bool> pvInsert(const_iterator hint, const key_type& key,
		const ValueCreator& valueCreator)
	{
		if (!pvCheckHint(hint, key))
			return pvInsert(nullptr, key, valueCreator);
		return std::pair<iterator, bool>(mTreeSet.AddCrt(hint, valueCreator), true);
	}

	template<typename ValueCreator>
	std::pair<iterator, bool> pvInsert(std::nullptr_t, const key_type& key,
		const ValueCreator& valueCreator)
	{
		typename TreeSet::InsertResult res = mTreeSet.InsertCrt(key, valueCreator);
		return std::pair<iterator, bool>(res.iterator, res.inserted);
	}

	template<typename Hint, typename... ValueArgs>
	std::pair<iterator, bool> pvEmplace(Hint hint, ValueArgs&&... valueArgs)
	{
		typename TreeSet::MemManager& memManager = mTreeSet.GetMemManager();
		typedef internal::ObjectBuffer<value_type, TreeSet::ItemTraits::alignment> ValueBuffer;
		typedef typename TreeSet::ItemTraits::template Creator<ValueArgs...> ValueCreator;
		ValueBuffer valueBuffer;
		ValueCreator(memManager, std::forward<ValueArgs>(valueArgs)...)(&valueBuffer);
		try
		{
			auto valueCreator = [&memManager, &valueBuffer] (value_type* newValue)
				{ TreeSet::ItemTraits::Relocate(memManager, *&valueBuffer, newValue); };
			return pvInsert(hint, *&valueBuffer, valueCreator);
		}
		catch (...)
		{
			TreeSet::ItemTraits::Destroy(memManager, *&valueBuffer);
			throw;
		}
	}

private:
	TreeSet mTreeSet;
};

} // namespace stdish

} // namespace momo
