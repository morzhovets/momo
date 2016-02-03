/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/stdish/map.h

  namespace momo::stdish:
    class map

  This class is similar to `std::map`, but much more efficient in
  memory usage and operation speed.
  The implementation is based on a B-tree.

  Deviations from the `std::map`:
  1. Container items must be movable (preferably without exceptions)
    or copyable, similar to items of `std::vector`.
  2. After each addition or removal of the item all iterators and
    references to items become invalid and should not be used.
  3. Type `reference` is not the same as `value_type&`, so
    `for (auto& p : map)` is illegal, but `for (auto p : map)` or
    `for (const auto& p : map)` or `for (auto&& p : map)` is allowed.
  4.1. Container move constructor, move assignment operator and swap
    function do not throw exceptions regardless of the allocator.
  4.2. Functions of the allocator `construct`, `destruct` and `address`
    are not used.
  4.3. It is expected that the allocator types `pointer`, `const_pointer`,
    `reference`, `const_reference`, `size_type` and `difference_type`
    have the standard definition (as in `std::allocator`).
  4.4. It is expected that the allocator types `propagate_on_container_swap`
    and `propagate_on_container_move_assignment` are the same as
    `std::true_type`.

  It is allowed to pass to functions `insert` and `emplace` references
  to items within the container.
  Class has functions `try_emplace` and `insert_or_assign` from C++ 17.

\**********************************************************/

#pragma once

#include "../TreeMap.h"

namespace momo
{

namespace stdish
{

template<typename TKey, typename TMapped,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>,
	typename TTreeMap = TreeMap<TKey, TMapped, TreeTraitsStd<TKey, TLessFunc>,
		MemManagerStd<TAllocator>>>
class map
{
private:
	typedef TTreeMap TreeMap;
	typedef typename TreeMap::TreeTraits TreeTraits;
	typedef typename TreeMap::MemManager MemManager;

public:
	typedef TKey key_type;
	typedef TMapped mapped_type;
	typedef TLessFunc key_compare;
	typedef TAllocator allocator_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef std::pair<const key_type, mapped_type> value_type;
	
	class value_compare : public std::binary_function<value_type, value_type, bool>
	{
	public:
		explicit value_compare(const key_compare& keyComp)
			: mKeyComp(keyComp)
		{
		}

		bool operator()(const value_type& value1, const value_type& value2) const
		{
			return mKeyComp(value1.first, value2.first);
		}

	private:
		key_compare mKeyComp;
	};

	typedef internal::MapReferenceStd<key_type, mapped_type,
		typename TreeMap::Iterator::Reference> reference;
	typedef typename reference::ConstReference const_reference;

	typedef internal::TreeDerivedIterator<typename TreeMap::Iterator, reference> iterator;
	typedef typename iterator::ConstIterator const_iterator;

	typedef typename iterator::Pointer pointer;
	typedef typename const_iterator::Pointer const_pointer;

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

private:
	typedef internal::ObjectBuffer<key_type, TreeMap::KeyValueTraits::keyAlignment> KeyBuffer;

public:
	map()
	{
	}

	explicit map(const allocator_type& alloc)
		: mTreeMap(TreeTraits(), MemManager(alloc))
	{
	}

	explicit map(const key_compare& lessFunc, const allocator_type& alloc = allocator_type())
		: mTreeMap(TreeTraits(lessFunc), MemManager(alloc))
	{
	}

	template<typename Iterator>
	map(Iterator first, Iterator last, const allocator_type& alloc = allocator_type())
		: map(alloc)
	{
		insert(first, last);
	}

	template<typename Iterator>
	map(Iterator first, Iterator last, const key_compare& lessFunc,
		const allocator_type& alloc = allocator_type())
		: map(lessFunc, alloc)
	{
		insert(first, last);
	}

	map(std::initializer_list<value_type> values, const allocator_type& alloc = allocator_type())
		: map(values.begin(), values.end(), alloc)
	{
	}

	map(std::initializer_list<value_type> values, const key_compare& lessFunc,
		const allocator_type& alloc = allocator_type())
		: map(values.begin(), values.end(), lessFunc, alloc)
	{
	}

	map(map&& right) MOMO_NOEXCEPT
		: mTreeMap(std::move(right.mTreeMap))
	{
	}

	map(map&& right, const allocator_type& alloc)
		: mTreeMap(right.mTreeMap.GetTreeTraits(), MemManager(alloc))
	{
		if (right.get_allocator() == alloc)
			*this = std::move(right);
		else
			insert(right.begin(), right.end());
		right.clear();
	}

	map(const map& right)
		: mTreeMap(right.mTreeMap)
	{
	}

	map(const map& right, const allocator_type& alloc)
		: mTreeMap(right.mTreeMap.GetTreeTraits(), MemManager(alloc))
	{
		insert(right.begin(), right.end());
	}

	~map() MOMO_NOEXCEPT
	{
	}

	map& operator=(map&& right) MOMO_NOEXCEPT
	{
		mTreeMap = std::move(right.mTreeMap);
		return *this;
	}

	map& operator=(const map& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_copy_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			map(right, alloc).swap(*this);
		}
		return *this;
	}

	map& operator=(std::initializer_list<value_type> values)
	{
		clear();	//?
		insert(values);
		return *this;
	}

	void swap(map& right) MOMO_NOEXCEPT
	{
		mTreeMap.Swap(right.mTreeMap);
	}

	friend void swap(map& left, map& right) MOMO_NOEXCEPT
	{
		left.swap(right);
	}

	iterator begin() MOMO_NOEXCEPT
	{
		return iterator(mTreeMap.GetBegin());
	}

	const_iterator begin() const MOMO_NOEXCEPT
	{
		return const_iterator(mTreeMap.GetBegin());
	}

	iterator end() MOMO_NOEXCEPT
	{
		return iterator(mTreeMap.GetEnd());
	}

	const_iterator end() const MOMO_NOEXCEPT
	{
		return const_iterator(mTreeMap.GetEnd());
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
		return mTreeMap.GetTreeTraits().GetLessFunc();
	}

	value_compare value_comp() const
	{
		return value_compare(key_comp());
	}

	allocator_type get_allocator() const //MOMO_NOEXCEPT
	{
		return mTreeMap.GetMemManager().GetAllocator();
	}

	size_type max_size() const //MOMO_NOEXCEPT
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const MOMO_NOEXCEPT
	{
		return mTreeMap.GetCount();
	}

	bool empty() const MOMO_NOEXCEPT
	{
		return mTreeMap.IsEmpty();
	}

	void clear() MOMO_NOEXCEPT
	{
		mTreeMap.Clear();
	}

	const_iterator find(const key_type& key) const
	{
		return const_iterator(mTreeMap.Find(key));
	}

	iterator find(const key_type& key)
	{
		return iterator(mTreeMap.Find(key));
	}

	template<typename KeyArg, typename = typename key_compare::is_transparent>
	const_iterator find(const KeyArg& key) const
	{
		return const_iterator(mTreeMap.Find(key));
	}

	template<typename KeyArg, typename = typename key_compare::is_transparent>
	iterator find(const KeyArg& key)
	{
		return iterator(mTreeMap.Find(key));
	}

	size_type count(const key_type& key) const
	{
		return mTreeMap.HasKey(key) ? 1 : 0;
	}

	template<typename KeyArg, typename = typename key_compare::is_transparent>
	size_type count(const KeyArg& key) const
	{
		return mTreeMap.HasKey(key) ? 1 : 0;
	}

	const_iterator lower_bound(const key_type& key) const
	{
		return const_iterator(mTreeMap.LowerBound(key));
	}

	iterator lower_bound(const key_type& key)
	{
		return iterator(mTreeMap.LowerBound(key));
	}

	template<typename KeyArg, typename = typename key_compare::is_transparent>
	const_iterator lower_bound(const KeyArg& key) const
	{
		return const_iterator(mTreeMap.LowerBound(key));
	}

	template<typename KeyArg, typename = typename key_compare::is_transparent>
	iterator lower_bound(const KeyArg& key)
	{
		return iterator(mTreeMap.LowerBound(key));
	}

	const_iterator upper_bound(const key_type& key) const
	{
		return const_iterator(mTreeMap.UpperBound(key));
	}

	iterator upper_bound(const key_type& key)
	{
		return iterator(mTreeMap.UpperBound(key));
	}

	template<typename KeyArg, typename = typename key_compare::is_transparent>
	const_iterator upper_bound(const KeyArg& key) const
	{
		return const_iterator(mTreeMap.UpperBound(key));
	}

	template<typename KeyArg, typename = typename key_compare::is_transparent>
	iterator upper_bound(const KeyArg& key)
	{
		return iterator(mTreeMap.UpperBound(key));
	}

	std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
	{
		return equal_range<key_type, void>(key);
	}

	std::pair<iterator, iterator> equal_range(const key_type& key)
	{
		return equal_range<key_type, void>(key);
	}

	template<typename KeyArg, typename = typename key_compare::is_transparent>
	std::pair<const_iterator, const_iterator> equal_range(const KeyArg& key) const
	{
		const_iterator iter = lower_bound(key);
		if (iter == end() || mTreeMap.GetTreeTraits().IsLess(key, iter->first))
			return std::pair<const_iterator, const_iterator>(iter, iter);
		return std::pair<const_iterator, const_iterator>(iter, std::next(iter));
	}

	template<typename KeyArg, typename = typename key_compare::is_transparent>
	std::pair<iterator, iterator> equal_range(const KeyArg& key)
	{
		iterator iter = lower_bound(key);
		if (iter == end() || mTreeMap.GetTreeTraits().IsLess(key, iter->first))
			return std::pair<iterator, iterator>(iter, iter);
		return std::pair<iterator, iterator>(iter, std::next(iter));
	}

	//template<typename Value>
	//typename std::enable_if<std::is_convertible<Value, value_type>::value, iterator>::type
	//insert(Value&& value)

	//template<typename Value>
	//typename std::enable_if<std::is_convertible<Value, value_type>::value, iterator>::type
	//insert(const_iterator hint, Value&& value)

	//iterator insert(const value_type& value)

	//iterator insert(const_iterator hint, const value_type& value)

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<const First&, key_type>::value
		&& std::is_convertible<const Second&, mapped_type>::value,
		std::pair<iterator, bool>>::type
	insert(const std::pair<First, Second>& value)
	{
		return _insert(nullptr, std::forward_as_tuple(value.first),
			std::forward_as_tuple(value.second));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<const First&, key_type>::value
		&& std::is_convertible<const Second&, mapped_type>::value, iterator>::type
	insert(const_iterator hint, const std::pair<First, Second>& value)
	{
		return _insert(hint, std::forward_as_tuple(value.first),
			std::forward_as_tuple(value.second)).first;
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<First, key_type>::value
		&& std::is_convertible<Second, mapped_type>::value,
		std::pair<iterator, bool>>::type
	insert(std::pair<First, Second>&& value)
	{
		return _insert(nullptr, std::forward_as_tuple(std::forward<First>(value.first)),
			std::forward_as_tuple(std::forward<Second>(value.second)));
	}

	template<typename First, typename Second>
	typename std::enable_if<std::is_convertible<First, key_type>::value
		&& std::is_convertible<Second, mapped_type>::value, iterator>::type
	insert(const_iterator hint, std::pair<First, Second>&& value)
	{
		return _insert(hint, std::forward_as_tuple(std::forward<First>(value.first)),
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
		return _insert(nullptr, std::tuple<>(), std::tuple<>());
	}

	iterator emplace_hint(const_iterator hint)
	{
		return _insert(hint, std::tuple<>(), std::tuple<>()).first;
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
		return _insert(nullptr, std::forward_as_tuple(std::forward<KeyArg>(keyArg)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg)));
	}

	template<typename KeyArg, typename MappedArg>
	iterator emplace_hint(const_iterator hint, KeyArg&& keyArg, MappedArg&& mappedArg)
	{
		return _insert(hint, std::forward_as_tuple(std::forward<KeyArg>(keyArg)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg))).first;
	}

	template<typename... KeyArgs, typename... MappedArgs>
	std::pair<iterator, bool> emplace(std::piecewise_construct_t,
		std::tuple<KeyArgs...> keyArgs, std::tuple<MappedArgs...> mappedArgs)
	{
		return _insert(nullptr, std::move(keyArgs), std::move(mappedArgs));
	}

	template<typename... KeyArgs, typename... MappedArgs>
	iterator emplace_hint(const_iterator hint, std::piecewise_construct_t,
		std::tuple<KeyArgs...> keyArgs, std::tuple<MappedArgs...> mappedArgs)
	{
		return _insert(hint, std::move(keyArgs), std::move(mappedArgs)).first;
	}

	iterator erase(const_iterator where)
	{
		return iterator(mTreeMap.Remove(where.GetBaseIterator()));
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		if (first == begin() && last == end())
		{
			clear();
			return end();
		}
		if (first == last)
			return (first != end()) ? find(first->first) : end();	//?
		size_t count = std::distance(first, last);
		iterator iter = erase(first);
		for (size_t i = 1; i < count; ++i)
			iter = erase(iter);
		return iter;
	}

	size_type erase(const key_type& key)
	{
		return mTreeMap.Remove(key) ? 1 : 0;
	}

	typename TreeMap::ValueReferenceRKey operator[](key_type&& key)
	{
		return mTreeMap[std::move(key)];
	}

	typename TreeMap::ValueReferenceCKey operator[](const key_type& key)
	{
		return mTreeMap[key];
	}

	const mapped_type& at(const key_type& key) const
	{
		const_iterator iter = find(key);
		if (iter == end())
			throw std::out_of_range("invalid map key");
		return iter->second;
	}

	mapped_type& at(const key_type& key)
	{
		iterator iter = find(key);
		if (iter == end())
			throw std::out_of_range("invalid map key");
		return iter->second;
	}

	template<typename... MappedArgs>
	std::pair<iterator, bool> try_emplace(key_type&& key, MappedArgs&&... mappedArgs)
	{
		return _insert(nullptr, std::forward_as_tuple(std::move(key)),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...));
	}

	template<typename... MappedArgs>
	iterator try_emplace(const_iterator hint, key_type&& key, MappedArgs&&... mappedArgs)
	{
		return _insert(hint, std::forward_as_tuple(std::move(key)),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...)).first;
	}

	template<typename... MappedArgs>
	std::pair<iterator, bool> try_emplace(const key_type& key, MappedArgs&&... mappedArgs)
	{
		return _insert(nullptr, std::forward_as_tuple(key),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...));
	}

	template<typename... MappedArgs>
	iterator try_emplace(const_iterator hint, const key_type& key, MappedArgs&&... mappedArgs)
	{
		return _insert(hint, std::forward_as_tuple(key),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...)).first;
	}

	template<typename MappedArg>
	std::pair<iterator, bool> insert_or_assign(key_type&& key, MappedArg&& mappedArg)
	{
		return _insert_or_assign(nullptr, std::move(key), std::forward<MappedArg>(mappedArg));
	}

	template<typename MappedArg>
	iterator insert_or_assign(const_iterator hint, key_type&& key, MappedArg&& mappedArg)
	{
		return _insert_or_assign(hint, std::move(key), std::forward<MappedArg>(mappedArg)).first;
	}

	template<typename MappedArg>
	std::pair<iterator, bool> insert_or_assign(const key_type& key, MappedArg&& mappedArg)
	{
		return _insert_or_assign(nullptr, key, std::forward<MappedArg>(mappedArg));
	}

	template<typename MappedArg>
	iterator insert_or_assign(const_iterator hint, const key_type& key, MappedArg&& mappedArg)
	{
		return _insert_or_assign(hint, key, std::forward<MappedArg>(mappedArg)).first;
	}

	bool operator==(const map& right) const
	{
		return size() == right.size() && std::equal(begin(), end(), right.begin());
	}

	bool operator!=(const map& right) const
	{
		return !(*this == right);
	}

	bool operator<(const map& right) const
	{
		return std::lexicographical_compare(begin(), end(), right.begin(), right.end());
	}

	bool operator>(const map& right) const
	{
		return right < *this;
	}

	bool operator<=(const map& right) const
	{
		return !(right < *this);
	}

	bool operator>=(const map& right) const
	{
		return right <= *this;
	}

private:
	bool _check_hint(const_iterator hint, const key_type& key) const
	{
		const TreeTraits& treeTraits = mTreeMap.GetTreeTraits();
		return (hint == begin() || treeTraits.IsLess(std::prev(hint)->first, key))
			&& (hint == end() || treeTraits.IsLess(key, hint->first));
	}

	template<typename Hint, typename... KeyArgs, typename... MappedArgs>
	std::pair<iterator, bool> _insert(Hint hint, std::tuple<KeyArgs...>&& keyArgs,
		std::tuple<MappedArgs...>&& mappedArgs)
	{
		typedef typename TreeMap::KeyValueTraits
			::template ValueCreator<MappedArgs...> MappedCreator;
		return _insert(hint, std::move(keyArgs), MappedCreator(std::move(mappedArgs)));
	}

	template<typename Hint, typename... KeyArgs, typename MappedCreator>
	std::pair<iterator, bool> _insert(Hint hint, std::tuple<KeyArgs...>&& keyArgs,
		const MappedCreator& mappedCreator)
	{
		typedef internal::ObjectManager<key_type> KeyManager;
		typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
		KeyBuffer keyBuffer;
		KeyCreator(std::move(keyArgs))(&keyBuffer);
		std::pair<iterator, bool> res;
		try
		{
			res = _insert(hint, std::forward_as_tuple(std::move(*&keyBuffer)), mappedCreator);
		}
		catch (...)
		{
			KeyManager::Destroy(*&keyBuffer);
			throw;
		}
		KeyManager::Destroy(*&keyBuffer);
		return res;
	}

	template<typename MappedCreator>
	std::pair<iterator, bool> _insert(std::nullptr_t, std::tuple<key_type&&>&& key,
		const MappedCreator& mappedCreator)
	{
		typename TreeMap::InsertResult res = mTreeMap.InsertCrt(std::move(std::get<0>(key)),
			mappedCreator);
		return std::pair<iterator, bool>(iterator(res.iterator), res.inserted);
	}

	template<typename MappedCreator>
	std::pair<iterator, bool> _insert(const_iterator hint, std::tuple<key_type&&>&& key,
		const MappedCreator& mappedCreator)
	{
		if (!_check_hint(hint, (const key_type&)std::get<0>(key)))
			return _insert(nullptr, std::move(key), mappedCreator);
		typename TreeMap::Iterator resIter = mTreeMap.AddCrt(hint.GetBaseIterator(),
			std::move(std::get<0>(key)), mappedCreator);
		return std::pair<iterator, bool>(iterator(resIter), true);
	}

	template<typename MappedCreator>
	std::pair<iterator, bool> _insert(std::nullptr_t, std::tuple<const key_type&> key,
		const MappedCreator& mappedCreator)
	{
		typename TreeMap::InsertResult res = mTreeMap.InsertCrt(std::get<0>(key), mappedCreator);
		return std::pair<iterator, bool>(iterator(res.iterator), res.inserted);
	}

	template<typename MappedCreator>
	std::pair<iterator, bool> _insert(const_iterator hint, std::tuple<const key_type&> key,
		const MappedCreator& mappedCreator)
	{
		if (!_check_hint(hint, std::get<0>(key)))
			return _insert(nullptr, key, mappedCreator);
		typename TreeMap::Iterator resIter = mTreeMap.AddCrt(hint.GetBaseIterator(),
			std::get<0>(key), mappedCreator);
		return std::pair<iterator, bool>(iterator(resIter), true);
	}
	
	template<typename Hint, typename RKey, typename MappedArg>
	std::pair<iterator, bool> _insert_or_assign(Hint hint, RKey&& key, MappedArg&& mappedArg)
	{
		std::pair<iterator, bool> res = _insert(hint,
			std::forward_as_tuple(std::forward<RKey>(key)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg)));
		if (!res.second)
			res.first->second = std::forward<MappedArg>(mappedArg);
		return res;
	}

private:
	TreeMap mTreeMap;
};

} // namespace stdish

} // namespace momo
