/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/stdish/set.h

  namespace momo::stdish:
    class set
    class multiset

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_STDISH_SET
#define MOMO_INCLUDE_GUARD_STDISH_SET

#ifdef __has_include
# if __has_include(<momo/Utility.h>)
#  include <momo/Utility.h>
# endif
#endif
#ifndef MOMO_PARENT_HEADER
# include "../Utility.h"
#endif

#include MOMO_PARENT_HEADER(TreeSet)
#include "set_map_utility.h"

namespace momo
{

namespace stdish
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
	typename TLessComparer = std::less<TKey>,
	typename TAllocator = std::allocator<TKey>,
	typename TTreeSet = TreeSet<TKey, TreeTraitsStd<TKey, TLessComparer>, MemManagerStd<TAllocator>>>
class set
{
private:
	typedef TTreeSet TreeSet;
	typedef typename TreeSet::TreeTraits TreeTraits;
	typedef typename TreeSet::MemManager MemManager;

public:
	typedef TKey key_type;
	typedef TLessComparer key_compare;
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

	explicit set(const key_compare& lessComp, const allocator_type& alloc = allocator_type())
		: mTreeSet(TreeTraits(lessComp), MemManager(alloc))
	{
	}

	template<typename Iterator>
	set(Iterator first, Iterator last, const allocator_type& alloc = allocator_type())
		: set(alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	set(Iterator first, Iterator last, const key_compare& lessComp,
		const allocator_type& alloc = allocator_type())
		: set(lessComp, alloc)
	{
		insert(first, last);
	}

	set(std::initializer_list<momo::internal::Identity<value_type>> values,
		const allocator_type& alloc = allocator_type())
		: mTreeSet(values, TreeTraits(), MemManager(alloc))
	{
	}

	set(std::initializer_list<momo::internal::Identity<value_type>> values,
		const key_compare& lessComp, const allocator_type& alloc = allocator_type())
		: mTreeSet(values, TreeTraits(lessComp), MemManager(alloc))
	{
	}

#ifdef MOMO_HAS_CONTAINERS_RANGES
	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	set(std::from_range_t, Range&& values, const allocator_type& alloc = allocator_type())
		: set(alloc)
	{
		insert_range(std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	set(std::from_range_t, Range&& values, const key_compare& lessComp,
		const allocator_type& alloc = allocator_type())
		: set(lessComp, alloc)
	{
		insert_range(std::forward<Range>(values));
	}
#endif // MOMO_HAS_CONTAINERS_RANGES

	set(set&& right)
		: set(std::move(right), right.get_allocator())
	{
	}

	set(set&& right, const momo::internal::Identity<allocator_type>& alloc)
		: mTreeSet(right.mTreeSet.GetTreeTraits(), MemManager(alloc))
	{
		if (right.get_allocator() == alloc)
		{
			mTreeSet.Swap(right.mTreeSet);
		}
		else
		{
			mTreeSet.MergeFrom(right.mTreeSet);
			right.clear();
		}
	}

	set(const set& right)
		: mTreeSet(right.mTreeSet)
	{
	}

	set(const set& right, const momo::internal::Identity<allocator_type>& alloc)
		: mTreeSet(right.mTreeSet, MemManager(alloc))
	{
	}

	~set() = default;

	set& operator=(set&& right)
		noexcept(momo::internal::ContainerAssignerStd::IsNothrowMoveAssignable<set>::value)
	{
		return momo::internal::ContainerAssignerStd::Move(std::move(right), *this);
	}

	set& operator=(const set& right)
	{
		return momo::internal::ContainerAssignerStd::Copy(right, *this);
	}

	set& operator=(std::initializer_list<value_type> values)
	{
		mTreeSet = TreeSet(values, mTreeSet.GetTreeTraits(), MemManager(get_allocator()));
		return *this;
	}

	void swap(set& right) noexcept
	{
		momo::internal::ContainerAssignerStd::Swap(*this, right);
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
		return mTreeSet.GetTreeTraits().GetLessComparer();
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

	MOMO_NODISCARD bool empty() const noexcept
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
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	const_iterator> find(const KeyArg& key) const
	{
		return mTreeSet.Find(key);
	}

	//template<typename KeyArg>
	//momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	//iterator> find(const KeyArg& key)

	size_type count(const key_type& key) const
	{
		return mTreeSet.GetKeyCount(key);
	}

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	size_type> count(const KeyArg& key) const
	{
		return mTreeSet.GetKeyCount(key);
	}

	bool contains(const key_type& key) const
	{
		return mTreeSet.ContainsKey(key);
	}

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	bool> contains(const KeyArg& key) const
	{
		return mTreeSet.ContainsKey(key);
	}

	const_iterator lower_bound(const key_type& key) const
	{
		return mTreeSet.GetLowerBound(key);
	}

	//iterator lower_bound(const key_type& key)

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	const_iterator> lower_bound(const KeyArg& key) const
	{
		return mTreeSet.GetLowerBound(key);
	}

	//template<typename KeyArg>
	//momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	//iterator> lower_bound(const KeyArg& key)

	const_iterator upper_bound(const key_type& key) const
	{
		return mTreeSet.GetUpperBound(key);
	}

	//iterator upper_bound(const key_type& key)

	template<typename KeyArg>
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	const_iterator> upper_bound(const KeyArg& key) const
	{
		return mTreeSet.GetUpperBound(key);
	}

	//template<typename KeyArg>
	//momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	//iterator> upper_bound(const KeyArg& key)

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
	momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	std::pair<const_iterator, const_iterator>> equal_range(const KeyArg& key) const
	{
		return { lower_bound(key), upper_bound(key) };
	}

	//template<typename KeyArg>
	//momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
	//std::pair<iterator, iterator>> equal_range(const KeyArg& key)

	std::pair<iterator, bool> insert(value_type&& value)
	{
		typename TreeSet::InsertResult res = mTreeSet.Insert(std::move(value));
		return { res.position, res.inserted };
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

	template<typename Iterator>
	void insert(Iterator first, Iterator last)
	{
		pvInsertRange(first, last);
	}

	void insert(std::initializer_list<value_type> values)
	{
		mTreeSet.Insert(values);
	}

#ifdef MOMO_HAS_CONTAINERS_RANGES
	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	void insert_range(Range&& values)
	{
		pvInsertRange(std::ranges::begin(values), std::ranges::end(values));
	}
#endif // MOMO_HAS_CONTAINERS_RANGES

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
	momo::internal::EnableIf<std::is_same<key_type, typename std::decay<ValueArg>::type>::value,
	std::pair<iterator, bool>> emplace(ValueArg&& valueArg)
	{
		typename TreeSet::InsertResult res = mTreeSet.InsertVar(
			static_cast<const key_type&>(valueArg), std::forward<ValueArg>(valueArg));
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
	momo::internal::EnableIf<std::is_same<key_type, typename std::decay<ValueArg>::type>::value,
	iterator> emplace_hint(const_iterator hint, ValueArg&& valueArg)
	{
		if (!pvCheckHint(hint, static_cast<const key_type&>(valueArg)))
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

	template<typename ValueFilter>
	friend size_type erase_if(set& cont, const ValueFilter& valueFilter)
	{
		return cont.mTreeSet.Remove(valueFilter);
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
		mTreeSet.MergeFrom(set.get_nested_container());
	}

	friend bool operator==(const set& left, const set& right)
	{
		return left.size() == right.size() && std::equal(left.begin(), left.end(), right.begin());
	}

#ifdef MOMO_HAS_THREE_WAY_COMPARISON
	friend auto operator<=>(const set& left, const set& right)
		requires requires (const_reference ref) { std::tie(ref) <=> std::tie(ref); }
	{
		auto valueThreeComp = [] (const value_type& value1, const value_type& value2)
			{ return std::tie(value1) <=> std::tie(value2); };
		return std::lexicographical_compare_three_way(left.begin(), left.end(),
			right.begin(), right.end(), valueThreeComp);
	}
#else
	friend bool operator<(const set& left, const set& right)
	{
		return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
	}
#endif

	MOMO_MORE_COMPARISON_OPERATORS(const set&)

private:
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
			if (TreeTraits::multiKey)
				hint = lower_bound(key);
			return TreeTraits::multiKey;
		}
		return true;
	}

	template<typename Iterator, typename Sentinel>
	momo::internal::EnableIf<momo::internal::IsSetArgIterator<Iterator, value_type>::value,
	void> pvInsertRange(Iterator begin, Sentinel end)
	{
		mTreeSet.Insert(std::move(begin), std::move(end));
	}

	template<typename Iterator, typename Sentinel>
	momo::internal::EnableIf<!momo::internal::IsSetArgIterator<Iterator, value_type>::value,
	void> pvInsertRange(Iterator begin, Sentinel end)
	{
		for (Iterator iter = std::move(begin); iter != end; ++iter)
			emplace(*iter);
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
	typename TLessComparer = std::less<TKey>,
	typename TAllocator = std::allocator<TKey>,
	typename TTreeSet = TreeSet<TKey, TreeTraitsStd<TKey, TLessComparer, true>,
		MemManagerStd<TAllocator>>>
class multiset : public set<TKey, TLessComparer, TAllocator, TTreeSet>
{
private:
	typedef set<TKey, TLessComparer, TAllocator, TTreeSet> Set;

public:
	using typename Set::size_type;
	using typename Set::value_type;
	using typename Set::iterator;
	using typename Set::node_type;

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

	template<typename ValueFilter>
	friend size_type erase_if(multiset& cont, const ValueFilter& valueFilter)
	{
		return cont.get_nested_container().Remove(valueFilter);
	}
};

#ifdef MOMO_HAS_DEDUCTION_GUIDES

#define MOMO_DECLARE_DEDUCTION_GUIDES(set) \
template<typename Iterator, \
	typename Key = typename std::iterator_traits<Iterator>::value_type, \
	typename Allocator = std::allocator<Key>, \
	typename = internal::ordered_checker<Key, Allocator>> \
set(Iterator, Iterator, Allocator = Allocator()) \
	-> set<Key, std::less<Key>, Allocator>; \
template<typename Iterator, typename LessComparer, \
	typename Key = typename std::iterator_traits<Iterator>::value_type, \
	typename Allocator = std::allocator<Key>, \
	typename = internal::ordered_checker<Key, Allocator, LessComparer>> \
set(Iterator, Iterator, LessComparer, Allocator = Allocator()) \
	-> set<Key, LessComparer, Allocator>; \
template<typename Key, \
	typename Allocator = std::allocator<Key>, \
	typename = internal::ordered_checker<Key, Allocator>> \
set(std::initializer_list<Key>, Allocator = Allocator()) \
	-> set<Key, std::less<Key>, Allocator>; \
template<typename Key, typename LessComparer, \
	typename Allocator = std::allocator<Key>, \
	typename = internal::ordered_checker<Key, Allocator, LessComparer>> \
set(std::initializer_list<Key>, LessComparer, Allocator = Allocator()) \
	-> set<Key, LessComparer, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES(set)
MOMO_DECLARE_DEDUCTION_GUIDES(multiset)

#undef MOMO_DECLARE_DEDUCTION_GUIDES

#ifdef MOMO_HAS_CONTAINERS_RANGES

#define MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(set) \
template<std::ranges::input_range Range, \
	typename Key = std::ranges::range_value_t<Range>, \
	typename Allocator = std::allocator<Key>, \
	typename = internal::ordered_checker<Key, Allocator>> \
set(std::from_range_t, Range&&, Allocator = Allocator()) \
	-> set<Key, std::less<Key>, Allocator>; \
template<std::ranges::input_range Range, typename LessComparer, \
	typename Key = std::ranges::range_value_t<Range>, \
	typename Allocator = std::allocator<Key>, \
	typename = internal::ordered_checker<Key, Allocator, LessComparer>> \
set(std::from_range_t, Range&&, LessComparer, Allocator = Allocator()) \
	-> set<Key, LessComparer, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(set)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(multiset)

#undef MOMO_DECLARE_DEDUCTION_GUIDES_RANGES

#endif // MOMO_HAS_CONTAINERS_RANGES

#endif // MOMO_HAS_DEDUCTION_GUIDES

} // namespace stdish

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_STDISH_SET
