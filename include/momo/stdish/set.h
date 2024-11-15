/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/stdish/set.h

  namespace momo::stdish:
    class set
    class multiset

\**********************************************************/

#pragma once

#include "../TreeSet.h"
#include "node_handle.h"

namespace momo::stdish
{

/*!
	\brief
	`momo::stdish::set` is similar to `std::set`, but much more
	efficient in memory usage. The implementation is based on a B-tree.

	\details
	Deviations from standard class:
	1. Container items must be movable (preferably without exceptions)
	or copyable, similar to items of `std::vector`.
	2. After each addition or removal of the item all iterators and
	references to items become invalid and should not be used.
	3. Functions `begin`, `cbegin`, `rend` and `crend` have logarithmic
	complexity.
	4. If `ObjectManager<key_type>::isNothrowAnywayAssignable` is false,
	functions `erase` can throw exceptions.
	5. Functions `merge`, `extract` and `insert(node_type&&)` move items.

	It is allowed to pass to functions `insert` and `emplace` references
	to items within the container.

	Function `merge` can work fast, if container types are same and each
	key from one container is less than each key from other container.
*/

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
	typedef typename TreeSet::Iterator iterator;

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

	typedef internal::set_node_handle<typename TreeSet::ExtractedItem> node_type;
	typedef internal::insert_return_type<iterator, node_type> insert_return_type;

private:
	template<typename KeyArg>
	struct IsValidKeyArg : public TreeTraits::template IsValidKeyArg<KeyArg>
	{
	};

	struct NodeTypeProxy : private node_type
	{
		typedef node_type NodeType;
		MOMO_DECLARE_PROXY_FUNCTION(NodeType, GetExtractedItem)
	};

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

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	set(Iterator first, Iterator last, const allocator_type& alloc = allocator_type())
		: set(alloc)
	{
		insert(first, last);
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
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

#if defined(__cpp_lib_containers_ranges)
	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	set(std::from_range_t, Range&& values, const allocator_type& alloc = allocator_type())
		: set(alloc)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	set(std::from_range_t, Range&& values, const key_compare& lessFunc,
		const allocator_type& alloc = allocator_type())
		: set(lessFunc, alloc)
	{
		insert_range(std::forward<Range>(values));
	}
#endif // __cpp_lib_containers_ranges

	set(set&& right) noexcept
		: mTreeSet(std::move(right.mTreeSet))
	{
	}

	set(set&& right, const std::type_identity_t<allocator_type>& alloc)
		noexcept(std::allocator_traits<allocator_type>::is_always_equal::value)
		: mTreeSet(pvCreateSet(std::move(right), alloc))
	{
	}

	set(const set& right)
		: mTreeSet(right.mTreeSet)
	{
	}

	set(const set& right, const std::type_identity_t<allocator_type>& alloc)
		: mTreeSet(right.mTreeSet, MemManager(alloc))
	{
	}

	~set() noexcept = default;

	set& operator=(set&& right)
		noexcept(std::allocator_traits<allocator_type>::is_always_equal::value ||
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>::is_always_equal::value ||
				std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value;
			allocator_type alloc = (propagate ? &right : this)->get_allocator();
			mTreeSet = pvCreateSet(std::move(right), alloc);
		}
		return *this;
	}

	set& operator=(const set& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>::is_always_equal::value ||
				std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value;
			allocator_type alloc = (propagate ? &right : this)->get_allocator();
			mTreeSet = TreeSet(right.mTreeSet, MemManager(alloc));
		}
		return *this;
	}

	set& operator=(std::initializer_list<value_type> values)
	{
		mTreeSet = TreeSet(values, mTreeSet.GetTreeTraits(), MemManager(get_allocator()));
		return *this;
	}

	void swap(set& right) noexcept
	{
		MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
			|| get_allocator() == right.get_allocator());
		mTreeSet.Swap(right.mTreeSet);
	}

	friend void swap(set& left, set& right) noexcept
	{
		left.swap(right);
	}

	const nested_container_type& get_nested_container() const noexcept
	{
		return mTreeSet;
	}

	nested_container_type& get_nested_container() noexcept
	{
		return mTreeSet;
	}

	const_iterator begin() const noexcept
	{
		return mTreeSet.GetBegin();
	}

	//iterator begin() noexcept

	const_iterator end() const noexcept
	{
		return mTreeSet.GetEnd();
	}

	//iterator end() noexcept

	const_reverse_iterator rbegin() const noexcept
	{
		return const_reverse_iterator(end());
	}

	//reverse_iterator rbegin() noexcept

	const_reverse_iterator rend() const noexcept
	{
		return const_reverse_iterator(begin());
	}

	//reverse_iterator rend() noexcept

	const_iterator cbegin() const noexcept
	{
		return begin();
	}

	const_iterator cend() const noexcept
	{
		return end();
	}

	const_reverse_iterator crbegin() const noexcept
	{
		return rbegin();
	}

	const_reverse_iterator crend() const noexcept
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

	allocator_type get_allocator() const noexcept
	{
		return allocator_type(mTreeSet.GetMemManager().GetByteAllocator());
	}

	size_type max_size() const noexcept
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const noexcept
	{
		return mTreeSet.GetCount();
	}

	[[nodiscard]] bool empty() const noexcept
	{
		return mTreeSet.IsEmpty();
	}

	void clear() noexcept
	{
		mTreeSet.Clear();
	}

	const_iterator find(const key_type& key) const
	{
		return mTreeSet.Find(key);
	}

	//iterator find(const key_type& key)

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	const_iterator find(const KeyArg& key) const
	{
		return mTreeSet.Find(key);
	}

	//template<typename KeyArg>
	//requires IsValidKeyArg<KeyArg>::value
	//iterator find(const KeyArg& key)

	size_type count(const key_type& key) const
	{
		return mTreeSet.GetKeyCount(key);
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	size_type count(const KeyArg& key) const
	{
		return mTreeSet.GetKeyCount(key);
	}

	bool contains(const key_type& key) const
	{
		return mTreeSet.ContainsKey(key);
	}

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	bool contains(const KeyArg& key) const
	{
		return mTreeSet.ContainsKey(key);
	}

	const_iterator lower_bound(const key_type& key) const
	{
		return mTreeSet.GetLowerBound(key);
	}

	//iterator lower_bound(const key_type& key)

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	const_iterator lower_bound(const KeyArg& key) const
	{
		return mTreeSet.GetLowerBound(key);
	}

	//template<typename KeyArg>
	//requires IsValidKeyArg<KeyArg>::value
	//iterator lower_bound(const KeyArg& key)

	const_iterator upper_bound(const key_type& key) const
	{
		return mTreeSet.GetUpperBound(key);
	}

	//iterator upper_bound(const key_type& key)

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	const_iterator upper_bound(const KeyArg& key) const
	{
		return mTreeSet.GetUpperBound(key);
	}

	//template<typename KeyArg>
	//requires IsValidKeyArg<KeyArg>::value
	//iterator upper_bound(const KeyArg& key)

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		const_iterator iter = lower_bound(key);
		if (TreeTraits::multiKey)
			return { iter, upper_bound(key) };
		if (iter == end() || mTreeSet.GetTreeTraits().IsLess(key, *iter))
			return { iter, iter };
		return { iter, std::next(iter) };
	}

	//std::pair<iterator, iterator> equal_range(const key_type& key)

	template<typename KeyArg>
	requires IsValidKeyArg<KeyArg>::value
	std::pair<const_iterator, const_iterator> equal_range(const KeyArg& key) const
	{
		return { lower_bound(key), upper_bound(key) };
	}

	//template<typename KeyArg>
	//requires IsValidKeyArg<KeyArg>::value
	//std::pair<iterator, iterator> equal_range(const KeyArg& key)

	std::pair<iterator, bool> insert(value_type&& value)
	{
		typename TreeSet::InsertResult res = mTreeSet.Insert(std::move(value));
		return { res.position, res.inserted };
	}

	iterator insert(const_iterator hint, value_type&& value)
	{
		if (!pvCheckHint(hint, std::as_const(value)))
			return insert(std::move(value)).first;
		return mTreeSet.Add(hint, std::move(value));
	}

	std::pair<iterator, bool> insert(const value_type& value)
	{
		typename TreeSet::InsertResult res = mTreeSet.Insert(value);
		return { res.position, res.inserted };
	}

	iterator insert(const_iterator hint, const value_type& value)
	{
		if (!pvCheckHint(hint, value))
			return insert(value).first;
		return mTreeSet.Add(hint, value);
	}

	insert_return_type insert(node_type&& node)
	{
		if (node.empty())
			return { end(), false, node_type() };
		typename TreeSet::InsertResult res = mTreeSet.Insert(
			std::move(NodeTypeProxy::GetExtractedItem(node)));
		return { res.position, res.inserted, res.inserted ? node_type() : std::move(node) };
	}

	iterator insert(const_iterator hint, node_type&& node)
	{
		if (node.empty())
			return end();
		if (!pvCheckHint(hint, node.value()))
			return insert(std::move(node)).position;
		return mTreeSet.Add(hint, std::move(NodeTypeProxy::GetExtractedItem(node)));
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	void insert(Iterator first, Iterator last)
	{
		pvInsertRange(first, last);
	}

	void insert(std::initializer_list<value_type> values)
	{
		mTreeSet.Insert(values);
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
		MemManager& memManager = mTreeSet.GetMemManager();
		typename TreeSet::ExtractedItem extItem;
		typedef typename TreeSet::ItemTraits::template Creator<ValueArgs...> ValueCreator;
		extItem.Create(ValueCreator(memManager, std::forward<ValueArgs>(valueArgs)...));
		typename TreeSet::InsertResult res = mTreeSet.Insert(std::move(extItem));
		return { res.position, res.inserted };
	}

	template<typename ValueArg>
	requires std::is_same_v<key_type, std::decay_t<ValueArg>>
	std::pair<iterator, bool> emplace(ValueArg&& valueArg)
	{
		typename TreeSet::InsertResult res = mTreeSet.InsertVar(
			std::as_const(valueArg), std::forward<ValueArg>(valueArg));
		return { res.position, res.inserted };
	}

	template<typename... ValueArgs>
	iterator emplace_hint(const_iterator hint, ValueArgs&&... valueArgs)
	{
		MemManager& memManager = mTreeSet.GetMemManager();
		typename TreeSet::ExtractedItem extItem;
		typedef typename TreeSet::ItemTraits::template Creator<ValueArgs...> ValueCreator;
		extItem.Create(ValueCreator(memManager, std::forward<ValueArgs>(valueArgs)...));
		if (!pvCheckHint(hint, extItem.GetItem()))
			return mTreeSet.Insert(std::move(extItem)).position;
		return mTreeSet.Add(hint, std::move(extItem));
	}

	template<typename ValueArg>
	requires std::is_same_v<key_type, std::decay_t<ValueArg>>
	iterator emplace_hint(const_iterator hint, ValueArg&& valueArg)
	{
		if (!pvCheckHint(hint, std::as_const(valueArg)))
			return emplace(std::forward<ValueArg>(valueArg)).first;
		return mTreeSet.AddVar(hint, std::forward<ValueArg>(valueArg));
	}

	iterator erase(const_iterator where)
	{
		return mTreeSet.Remove(where);
	}

	//iterator erase(iterator where)

	iterator erase(const_iterator first, const_iterator last)
	{
		return mTreeSet.Remove(first, last);
	}

	size_type erase(const key_type& key)
	{
		return mTreeSet.Remove(key);
	}

	template<typename KeyArg>
	requires (IsValidKeyArg<KeyArg>::value && !std::is_convertible_v<KeyArg&&, const_iterator>)
	size_type erase(KeyArg&& key)
	{
		std::pair<iterator, iterator> range = equal_range(std::forward<KeyArg>(key));
		size_t count = momo::internal::UIntMath<>::Dist(range.first, range.second);
		erase(range.first, range.second);
		return count;
	}

	template<momo::internal::conceptPredicate<const_reference> ValueFilter>
	friend size_type erase_if(set& cont, ValueFilter valueFilter)
	{
		return cont.mTreeSet.Remove(momo::FastCopyableFunctor<ValueFilter>(valueFilter));
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
		mTreeSet.MergeFrom(set.get_nested_container());
	}

	bool operator==(const set& right) const
	{
		return size() == right.size() && std::equal(begin(), end(), right.begin());
	}

	auto operator<=>(const set& right) const
		requires requires (const_reference ref) { std::tie(ref) <=> std::tie(ref); }
	{
		auto comp = [] (const value_type& value1, const value_type& value2)
			{ return std::tie(value1) <=> std::tie(value2); };
		return std::lexicographical_compare_three_way(begin(), end(),
			right.begin(), right.end(), comp);
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

	bool pvIsOrdered(const key_type& key1, const key_type& key2) const
	{
		const TreeTraits& treeTraits = mTreeSet.GetTreeTraits();
		return TreeTraits::multiKey ? !treeTraits.IsLess(key2, key1)
			: treeTraits.IsLess(key1, key2);
	}

	bool pvCheckHint(const_iterator& hint, const key_type& key) const
	{
		if (hint != begin() && !pvIsOrdered(*std::prev(hint), key))
			return false;
		if (hint != end() && !pvIsOrdered(key, *hint))
		{
			if constexpr (TreeTraits::multiKey)
				hint = lower_bound(key);
			return TreeTraits::multiKey;
		}
		return true;
	}

	template<std::input_iterator Iterator,
		momo::internal::conceptSentinel<Iterator> Sentinel>
	void pvInsertRange(Iterator begin, Sentinel end)
	{
		if constexpr (momo::internal::conceptSetArgIterator<Iterator, value_type>)
		{
			mTreeSet.Insert(std::move(begin), std::move(end));
		}
		else
		{
			for (Iterator iter = std::move(begin); iter != end; ++iter)
				emplace(*iter);
		}
	}

private:
	TreeSet mTreeSet;
};

/*!
	\brief
	`momo::stdish::multiset` is similar to `std::multiset`, but much more
	efficient in memory usage. The implementation is based on a B-tree.

	\copydetails momo::stdish::set
*/

template<typename TKey,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<TKey>,
	typename TTreeSet = TreeSet<TKey, TreeTraitsStd<TKey, TLessFunc, true>,
		MemManagerStd<TAllocator>>>
class multiset : public set<TKey, TLessFunc, TAllocator, TTreeSet>
{
private:
	typedef set<TKey, TLessFunc, TAllocator, TTreeSet> Set;

public:
	using typename Set::size_type;
	using typename Set::value_type;
	using typename Set::iterator;
	using typename Set::node_type;
	using typename Set::const_reference;

	typedef iterator insert_return_type;

public:
	using Set::Set;

	multiset& operator=(std::initializer_list<value_type> values)
	{
		Set::operator=(values);
		return *this;
	}

	friend void swap(multiset& left, multiset& right) noexcept
	{
		left.swap(right);
	}

	using Set::insert;

	iterator insert(value_type&& value)
	{
		return Set::insert(std::move(value)).first;
	}

	iterator insert(const value_type& value)
	{
		return Set::insert(value).first;
	}

	iterator insert(node_type&& node)
	{
		return Set::insert(std::move(node)).position;
	}

	template<typename... ValueArgs>
	iterator emplace(ValueArgs&&... valueArgs)
	{
		return Set::emplace(std::forward<ValueArgs>(valueArgs)...).first;
	}

	template<momo::internal::conceptPredicate<const_reference> ValueFilter>
	friend size_type erase_if(multiset& cont, ValueFilter valueFilter)
	{
		return cont.get_nested_container().Remove(
			momo::FastCopyableFunctor<ValueFilter>(valueFilter));
	}
};

#define MOMO_DECLARE_DEDUCTION_GUIDES(set) \
template<typename Iterator, \
	typename Key = std::iter_value_t<Iterator>, \
	typename Allocator = std::allocator<Key>> \
set(Iterator, Iterator, Allocator = Allocator()) \
	-> set<Key, std::less<Key>, Allocator>; \
template<typename Iterator, typename LessFunc, \
	typename Key = std::iter_value_t<Iterator>, \
	typename Allocator = std::allocator<Key>> \
set(Iterator, Iterator, LessFunc, Allocator = Allocator()) \
	-> set<Key, LessFunc, Allocator>; \
template<typename Key, \
	typename Allocator = std::allocator<Key>> \
set(std::initializer_list<Key>, Allocator = Allocator()) \
	-> set<Key, std::less<Key>, Allocator>; \
template<typename Key, typename LessFunc, \
	typename Allocator = std::allocator<Key>> \
set(std::initializer_list<Key>, LessFunc, Allocator = Allocator()) \
	-> set<Key, LessFunc, Allocator>;

#define MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(set) \
template<std::ranges::input_range Range, \
	typename Key = std::ranges::range_value_t<Range>, \
	typename Allocator = std::allocator<Key>> \
set(std::from_range_t, Range&&, Allocator = Allocator()) \
	-> set<Key, std::less<Key>, Allocator>; \
template<std::ranges::input_range Range, typename LessFunc, \
	typename Key = std::ranges::range_value_t<Range>, \
	typename Allocator = std::allocator<Key>> \
set(std::from_range_t, Range&&, LessFunc, Allocator = Allocator()) \
	-> set<Key, LessFunc, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES(set)
MOMO_DECLARE_DEDUCTION_GUIDES(multiset)

#if defined(__cpp_lib_containers_ranges)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(set)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(multiset)
#endif

#undef MOMO_DECLARE_DEDUCTION_GUIDES
#undef MOMO_DECLARE_DEDUCTION_GUIDES_RANGES

} // namespace momo::stdish
