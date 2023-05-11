/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/stdish/map.h

  namespace momo::stdish:
    class map
    class multimap

\**********************************************************/

#pragma once

#include "../TreeMap.h"
#include "node_handle.h"

namespace momo::stdish
{

namespace internal
{
	template<typename TKey, typename TLessFunc>
	class map_value_compare
	{
	protected:
		typedef TKey Key;
		typedef TLessFunc LessFunc;

	public:
		template<typename Value>
		requires std::is_same_v<Key, std::decay_t<decltype(std::declval<Value>().first)>>
		bool operator()(const Value& value1, const Value& value2) const
		{
			return comp(value1.first, value2.first);
		}

	protected:
		map_value_compare(const LessFunc& lessFunc)
			: comp(lessFunc)
		{
		}

	protected:
		LessFunc comp;
	};

	template<typename TKey, typename TMapped, typename TLessFunc, typename TAllocator,
		typename TTreeMap>
	class map_base
	{
	private:
		typedef TTreeMap TreeMap;
		typedef typename TreeMap::TreeTraits TreeTraits;
		typedef typename TreeMap::MemManager MemManager;

		typedef typename TreeMap::Iterator TreeMapIterator;

	public:
		typedef TKey key_type;
		typedef TMapped mapped_type;
		typedef TLessFunc key_compare;
		typedef TAllocator allocator_type;

		typedef TreeMap nested_container_type;

		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		typedef std::pair<const key_type, mapped_type> value_type;

		typedef map_value_compare<key_type, key_compare> value_compare;

		typedef momo::internal::DerivedBidirectionalIterator<TreeMapIterator,
			momo::internal::MapReferenceStd> iterator;
		typedef typename iterator::ConstIterator const_iterator;

		typedef typename iterator::Reference reference;
		typedef typename const_iterator::Reference const_reference;

		typedef typename iterator::Pointer pointer;
		typedef typename const_iterator::Pointer const_pointer;
		//typedef typename std::allocator_traits<allocator_type>::pointer pointer;
		//typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;

		typedef std::reverse_iterator<iterator> reverse_iterator;
		typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

		typedef internal::map_node_handle<typename TreeMap::ExtractedPair> node_type;
		typedef internal::insert_return_type<iterator, node_type> insert_return_type;

	private:
		template<typename KeyArg>
		struct IsValidKeyArg : public TreeTraits::template IsValidKeyArg<KeyArg>
		{
		};

		struct ConstIteratorProxy : public const_iterator
		{
			typedef const_iterator ConstIterator;
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
			MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBaseIterator)
		};

		struct IteratorProxy : public iterator
		{
			typedef iterator Iterator;
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
			MOMO_DECLARE_PROXY_FUNCTION(Iterator, GetBaseIterator)
		};

		struct NodeTypeProxy : private node_type
		{
			typedef node_type NodeType;
			MOMO_DECLARE_PROXY_FUNCTION(NodeType, GetExtractedPair)
		};

	public:
		map_base()
		{
		}

		explicit map_base(const allocator_type& alloc)
			: mTreeMap(TreeTraits(), MemManager(alloc))
		{
		}

		explicit map_base(const key_compare& lessFunc, const allocator_type& alloc = allocator_type())
			: mTreeMap(TreeTraits(lessFunc), MemManager(alloc))
		{
		}

		template<momo::internal::conceptInputIterator Iterator>
		map_base(Iterator first, Iterator last, const allocator_type& alloc = allocator_type())
			: map_base(alloc)
		{
			insert(first, last);
		}

		template<momo::internal::conceptInputIterator Iterator>
		map_base(Iterator first, Iterator last, const key_compare& lessFunc,
			const allocator_type& alloc = allocator_type())
			: map_base(lessFunc, alloc)
		{
			insert(first, last);
		}

		map_base(std::initializer_list<value_type> values,
			const allocator_type& alloc = allocator_type())
			: map_base(values.begin(), values.end(), alloc)
		{
		}

		map_base(std::initializer_list<value_type> values, const key_compare& lessFunc,
			const allocator_type& alloc = allocator_type())
			: map_base(values.begin(), values.end(), lessFunc, alloc)
		{
		}

		map_base(map_base&& right) noexcept
			: mTreeMap(std::move(right.mTreeMap))
		{
		}

		map_base(map_base&& right, const std::type_identity_t<allocator_type>& alloc)
			noexcept(std::allocator_traits<allocator_type>::is_always_equal::value)
			: mTreeMap(pvCreateMap(std::move(right), alloc))
		{
		}

		map_base(const map_base& right)
			: mTreeMap(right.mTreeMap)
		{
		}

		map_base(const map_base& right, const std::type_identity_t<allocator_type>& alloc)
			: mTreeMap(right.mTreeMap, MemManager(alloc))
		{
		}

		~map_base() noexcept = default;

		map_base& operator=(map_base&& right)
			noexcept(std::allocator_traits<allocator_type>::is_always_equal::value ||
				std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
		{
			if (this != &right)
			{
				bool propagate = std::allocator_traits<allocator_type>::is_always_equal::value ||
					std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value;
				allocator_type alloc = (propagate ? &right : this)->get_allocator();
				mTreeMap = pvCreateMap(std::move(right), alloc);
			}
			return *this;
		}

		map_base& operator=(const map_base& right)
		{
			if (this != &right)
			{
				bool propagate = std::allocator_traits<allocator_type>::is_always_equal::value ||
					std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value;
				allocator_type alloc = (propagate ? &right : this)->get_allocator();
				mTreeMap = TreeMap(right.mTreeMap, MemManager(alloc));
			}
			return *this;
		}

		void swap(map_base& right) noexcept
		{
			MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
				|| get_allocator() == right.get_allocator());
			mTreeMap.Swap(right.mTreeMap);
		}

		const nested_container_type& get_nested_container() const noexcept
		{
			return mTreeMap;
		}

		nested_container_type& get_nested_container() noexcept
		{
			return mTreeMap;
		}

		const_iterator begin() const noexcept
		{
			return ConstIteratorProxy(mTreeMap.GetBegin());
		}

		iterator begin() noexcept
		{
			return IteratorProxy(mTreeMap.GetBegin());
		}

		const_iterator end() const noexcept
		{
			return ConstIteratorProxy(mTreeMap.GetEnd());
		}

		iterator end() noexcept
		{
			return IteratorProxy(mTreeMap.GetEnd());
		}

		const_reverse_iterator rbegin() const noexcept
		{
			return const_reverse_iterator(end());
		}

		reverse_iterator rbegin() noexcept
		{
			return reverse_iterator(end());
		}

		const_reverse_iterator rend() const noexcept
		{
			return const_reverse_iterator(begin());
		}

		reverse_iterator rend() noexcept
		{
			return reverse_iterator(begin());
		}

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
			return mTreeMap.GetTreeTraits().GetLessFunc();
		}

		value_compare value_comp() const
		{
			struct ValueCompareProxy : public value_compare
			{
				explicit ValueCompareProxy(const key_compare& keyComp)
					: value_compare(keyComp)
				{
				}
			};
			return ValueCompareProxy(key_comp());
		}

		allocator_type get_allocator() const noexcept
		{
			return allocator_type(mTreeMap.GetMemManager().GetByteAllocator());
		}

		size_type max_size() const noexcept
		{
			return std::allocator_traits<allocator_type>::max_size(get_allocator());
		}

		size_type size() const noexcept
		{
			return mTreeMap.GetCount();
		}

		[[nodiscard]] bool empty() const noexcept
		{
			return mTreeMap.IsEmpty();
		}

		void clear() noexcept
		{
			mTreeMap.Clear();
		}

		const_iterator find(const key_type& key) const
		{
			return ConstIteratorProxy(mTreeMap.Find(key));
		}

		iterator find(const key_type& key)
		{
			return IteratorProxy(mTreeMap.Find(key));
		}

		template<typename KeyArg>
		requires IsValidKeyArg<KeyArg>::value
		const_iterator find(const KeyArg& key) const
		{
			return ConstIteratorProxy(mTreeMap.Find(key));
		}

		template<typename KeyArg>
		requires IsValidKeyArg<KeyArg>::value
		iterator find(const KeyArg& key)
		{
			return IteratorProxy(mTreeMap.Find(key));
		}

		size_type count(const key_type& key) const
		{
			return mTreeMap.GetKeyCount(key);
		}

		template<typename KeyArg>
		requires IsValidKeyArg<KeyArg>::value
		size_type count(const KeyArg& key) const
		{
			return mTreeMap.GetKeyCount(key);
		}

		bool contains(const key_type& key) const
		{
			return mTreeMap.ContainsKey(key);
		}

		template<typename KeyArg>
		requires IsValidKeyArg<KeyArg>::value
		bool contains(const KeyArg& key) const
		{
			return mTreeMap.ContainsKey(key);
		}

		const_iterator lower_bound(const key_type& key) const
		{
			return ConstIteratorProxy(mTreeMap.GetLowerBound(key));
		}

		iterator lower_bound(const key_type& key)
		{
			return IteratorProxy(mTreeMap.GetLowerBound(key));
		}

		template<typename KeyArg>
		requires IsValidKeyArg<KeyArg>::value
		const_iterator lower_bound(const KeyArg& key) const
		{
			return ConstIteratorProxy(mTreeMap.GetLowerBound(key));
		}

		template<typename KeyArg>
		requires IsValidKeyArg<KeyArg>::value
		iterator lower_bound(const KeyArg& key)
		{
			return IteratorProxy(mTreeMap.GetLowerBound(key));
		}

		const_iterator upper_bound(const key_type& key) const
		{
			return ConstIteratorProxy(mTreeMap.GetUpperBound(key));
		}

		iterator upper_bound(const key_type& key)
		{
			return IteratorProxy(mTreeMap.GetUpperBound(key));
		}

		template<typename KeyArg>
		requires IsValidKeyArg<KeyArg>::value
		const_iterator upper_bound(const KeyArg& key) const
		{
			return ConstIteratorProxy(mTreeMap.GetUpperBound(key));
		}

		template<typename KeyArg>
		requires IsValidKeyArg<KeyArg>::value
		iterator upper_bound(const KeyArg& key)
		{
			return IteratorProxy(mTreeMap.GetUpperBound(key));
		}

		std::pair<const_iterator, const_iterator> equal_range(const key_type& key) const
		{
			const_iterator iter = lower_bound(key);
			if (TreeTraits::multiKey)
				return { iter, upper_bound(key) };
			if (iter == end() || mTreeMap.GetTreeTraits().IsLess(key, iter->first))
				return { iter, iter };
			return { iter, std::next(iter) };
		}

		std::pair<iterator, iterator> equal_range(const key_type& key)
		{
			iterator iter = lower_bound(key);
			if (TreeTraits::multiKey)
				return { iter, upper_bound(key) };
			if (iter == end() || mTreeMap.GetTreeTraits().IsLess(key, iter->first))
				return { iter, iter };
			return { iter, std::next(iter) };
		}

		template<typename KeyArg>
		requires IsValidKeyArg<KeyArg>::value
		std::pair<const_iterator, const_iterator> equal_range(const KeyArg& key) const
		{
			return { lower_bound(key), upper_bound(key) };
		}

		template<typename KeyArg>
		requires IsValidKeyArg<KeyArg>::value
		std::pair<iterator, iterator> equal_range(const KeyArg& key)
		{
			return { lower_bound(key), upper_bound(key) };
		}

		//template<typename Value>
		//requires std::is_constructible_v<value_type, Value>
		//std::pair<iterator, bool> insert(Value&& value)

		//template<typename Value>
		//requires std::is_constructible_v<value_type, Value>
		//iterator insert(const_iterator hint, Value&& value)

		//std::pair<iterator, bool> insert(value_type&& value)

		//iterator insert(const_iterator hint, value_type&& value)

		//std::pair<iterator, bool> insert(const value_type& value)

		//iterator insert(const_iterator hint, const value_type& value)

		std::pair<iterator, bool> insert(std::pair<key_type, mapped_type>&& value)
		{
			return ptEmplace(nullptr, std::forward_as_tuple(std::move(value.first)),
				std::forward_as_tuple(std::move(value.second)));
		}

		iterator insert(const_iterator hint, std::pair<key_type, mapped_type>&& value)
		{
			return ptEmplace(hint, std::forward_as_tuple(std::move(value.first)),
				std::forward_as_tuple(std::move(value.second))).first;
		}

		template<typename First, typename Second>
		requires std::is_constructible_v<key_type, const First&>
			&& std::is_constructible_v<mapped_type, const Second&>
		std::pair<iterator, bool> insert(const std::pair<First, Second>& value)
		{
			return ptEmplace(nullptr, std::forward_as_tuple(value.first),
				std::forward_as_tuple(value.second));
		}

		template<typename First, typename Second>
		requires std::is_constructible_v<key_type, const First&>
			&& std::is_constructible_v<mapped_type, const Second&>
		iterator insert(const_iterator hint, const std::pair<First, Second>& value)
		{
			return ptEmplace(hint, std::forward_as_tuple(value.first),
				std::forward_as_tuple(value.second)).first;
		}

		template<typename First, typename Second>
		requires std::is_constructible_v<key_type, First&&>
			&& std::is_constructible_v<mapped_type, Second&&>
		std::pair<iterator, bool> insert(std::pair<First, Second>&& value)
		{
			return ptEmplace(nullptr, std::forward_as_tuple(std::forward<First>(value.first)),
				std::forward_as_tuple(std::forward<Second>(value.second)));
		}

		template<typename First, typename Second>
		requires std::is_constructible_v<key_type, First&&>
			&& std::is_constructible_v<mapped_type, Second&&>
		iterator insert(const_iterator hint, std::pair<First, Second>&& value)
		{
			return ptEmplace(hint, std::forward_as_tuple(std::forward<First>(value.first)),
				std::forward_as_tuple(std::forward<Second>(value.second))).first;
		}

		insert_return_type insert(node_type&& node)
		{
			if (node.empty())
				return { end(), false, node_type() };
			typename TreeMap::InsertResult res = mTreeMap.Insert(
				std::move(NodeTypeProxy::GetExtractedPair(node)));
			return { IteratorProxy(res.position), res.inserted,
				res.inserted ? node_type() : std::move(node) };
		}

		iterator insert(const_iterator hint, node_type&& node)
		{
			if (node.empty())
				return end();
			std::pair<iterator, bool> res = pvFind(hint, node.key());
			if (!res.second)
				return res.first;
			return IteratorProxy(mTreeMap.Add(IteratorProxy::GetBaseIterator(res.first),
				std::move(NodeTypeProxy::GetExtractedPair(node))));
		}

		template<momo::internal::conceptInputIterator Iterator>
		void insert(Iterator first, Iterator last)
		{
			if constexpr (momo::internal::conceptMapArgIterator<Iterator, key_type, false>)
			{
				mTreeMap.Insert(first, last);
			}
			else
			{
				for (Iterator iter = first; iter != last; ++iter)
					insert(*iter);
			}
		}

		void insert(std::initializer_list<value_type> values)
		{
			mTreeMap.Insert(values.begin(), values.end());
		}

		std::pair<iterator, bool> emplace()
		{
			return ptEmplace(nullptr, std::tuple<>(), std::tuple<>());
		}

		iterator emplace_hint(const_iterator hint)
		{
			return ptEmplace(hint, std::tuple<>(), std::tuple<>()).first;
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
			return ptEmplace(nullptr, std::forward_as_tuple(std::forward<KeyArg>(keyArg)),
				std::forward_as_tuple(std::forward<MappedArg>(mappedArg)));
		}

		template<typename KeyArg, typename MappedArg>
		iterator emplace_hint(const_iterator hint, KeyArg&& keyArg, MappedArg&& mappedArg)
		{
			return ptEmplace(hint, std::forward_as_tuple(std::forward<KeyArg>(keyArg)),
				std::forward_as_tuple(std::forward<MappedArg>(mappedArg))).first;
		}

		template<typename... KeyArgs, typename... MappedArgs>
		std::pair<iterator, bool> emplace(std::piecewise_construct_t,
			std::tuple<KeyArgs...> keyArgs, std::tuple<MappedArgs...> mappedArgs)
		{
			return ptEmplace(nullptr, std::move(keyArgs), std::move(mappedArgs));
		}

		template<typename... KeyArgs, typename... MappedArgs>
		iterator emplace_hint(const_iterator hint, std::piecewise_construct_t,
			std::tuple<KeyArgs...> keyArgs, std::tuple<MappedArgs...> mappedArgs)
		{
			return ptEmplace(hint, std::move(keyArgs), std::move(mappedArgs)).first;
		}

		iterator erase(const_iterator where)
		{
			return IteratorProxy(mTreeMap.Remove(ConstIteratorProxy::GetBaseIterator(where)));
		}

		iterator erase(iterator where)
		{
			return erase(static_cast<const_iterator>(where));
		}

		iterator erase(const_iterator first, const_iterator last)
		{
			return IteratorProxy(mTreeMap.Remove(ConstIteratorProxy::GetBaseIterator(first),
				ConstIteratorProxy::GetBaseIterator(last)));
		}

		size_type erase(const key_type& key)
		{
			return mTreeMap.Remove(key);
		}

		template<typename KeyArg>
		requires (IsValidKeyArg<KeyArg>::value &&
			!std::is_convertible_v<KeyArg&&, const_iterator> && !std::is_convertible_v<KeyArg&&, iterator>)
		size_type erase(KeyArg&& key)
		{
			std::pair<iterator, iterator> range = equal_range(std::forward<KeyArg>(key));
			size_t count = momo::internal::UIntMath<>::Dist(range.first, range.second);
			erase(range.first, range.second);
			return count;
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
			mTreeMap.MergeFrom(map.get_nested_container());
		}

		bool operator==(const map_base& right) const
		{
			return size() == right.size() && std::equal(begin(), end(), right.begin());
		}

		auto operator<=>(const map_base& right) const
		{
			return std::lexicographical_compare_three_way(begin(), end(),
				right.begin(), right.end());
		}

	protected:	//?
		void ptAssign(std::initializer_list<value_type> values)
		{
			TreeMap treeMap(mTreeMap.GetTreeTraits(), MemManager(get_allocator()));
			treeMap.Insert(values.begin(), values.end());
			mTreeMap = std::move(treeMap);
		}

		template<typename Hint, typename... KeyArgs, typename... MappedArgs>
		std::pair<iterator, bool> ptEmplace(Hint hint, std::tuple<KeyArgs...>&& keyArgs,
			std::tuple<MappedArgs...>&& mappedArgs)
		{
			typedef typename TreeMap::KeyValueTraits
				::template ValueCreator<MappedArgs...> MappedCreator;
			return pvInsert(hint, std::move(keyArgs),
				MappedCreator(mTreeMap.GetMemManager(), std::move(mappedArgs)));
		}

	private:
		static TreeMap pvCreateMap(map_base&& right, const allocator_type& alloc)
		{
			if (right.get_allocator() == alloc)
				return std::move(right.mTreeMap);
			TreeMap treeMap(right.mTreeMap.GetTreeTraits(), MemManager(alloc));
			treeMap.MergeFrom(right.mTreeMap);
			return treeMap;
		}

		bool pvIsOrdered(const key_type& key1, const key_type& key2) const
		{
			const TreeTraits& treeTraits = mTreeMap.GetTreeTraits();
			return TreeTraits::multiKey ? !treeTraits.IsLess(key2, key1)
				: treeTraits.IsLess(key1, key2);
		}

		std::pair<iterator, bool> pvFind(std::nullptr_t /*hint*/, const key_type& key)
		{
			iterator iter = upper_bound(key);
			if (!TreeTraits::multiKey && iter != begin())
			{
				iterator prevIter = std::prev(iter);
				if (!mTreeMap.GetTreeTraits().IsLess(prevIter->first, key))
					return { prevIter, false };
			}
			return { iter, true };
		}

		std::pair<iterator, bool> pvFind(const_iterator hint, const key_type& key)
		{
			if (hint != begin() && !pvIsOrdered(std::prev(hint)->first, key))
				return pvFind(nullptr, key);
			if (hint != end() && !pvIsOrdered(key, hint->first))
				return pvFind(nullptr, key);
			return { IteratorProxy(mTreeMap.MakeMutableIterator(
				ConstIteratorProxy::GetBaseIterator(hint))), true };
		}

		template<typename Hint, typename... KeyArgs, typename MappedCreator>
		std::pair<iterator, bool> pvInsert(Hint hint, std::tuple<KeyArgs...>&& keyArgs,
			MappedCreator mappedCreator)
		{
			MemManager& memManager = mTreeMap.GetMemManager();
			typedef momo::internal::ObjectBuffer<key_type,
				TreeMap::KeyValueTraits::keyAlignment> KeyBuffer;
			typedef momo::internal::ObjectManager<key_type, MemManager> KeyManager;
			typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
			KeyBuffer keyBuffer;
			KeyCreator(memManager, std::move(keyArgs))(&keyBuffer);
			bool keyDestroyed = false;
			try
			{
				std::pair<iterator, bool> res = pvFind(hint, *&keyBuffer);
				if (!res.second)
				{
					KeyManager::Destroy(memManager, *&keyBuffer);
					keyDestroyed = true;
					return res;
				}
				auto valueCreator = [&memManager, &keyBuffer, &keyDestroyed,
					mappedCreator = std::move(mappedCreator)]
					(key_type* newKey, mapped_type* newMapped) mutable
				{
					KeyManager::Relocate(memManager, *&keyBuffer, newKey);
					keyDestroyed = true;
					try
					{
						std::move(mappedCreator)(newMapped);
					}
					catch (...)
					{
						KeyManager::Destroy(memManager, *newKey);
						throw;
					}
				};
				TreeMapIterator resIter = mTreeMap.AddCrt(
					IteratorProxy::GetBaseIterator(res.first), std::move(valueCreator));
				return { IteratorProxy(resIter), true };
			}
			catch (...)
			{
				if (!keyDestroyed)
					KeyManager::Destroy(memManager, *&keyBuffer);
				throw;
			}
		}

		template<typename Hint, typename RKey, typename MappedCreator,
			typename Key = std::decay_t<RKey>>
		requires std::is_same_v<key_type, Key>
		std::pair<iterator, bool> pvInsert(Hint hint, std::tuple<RKey>&& key,
			MappedCreator mappedCreator)
		{
			std::pair<iterator, bool> res = pvFind(hint, std::as_const(std::get<0>(key)));
			if (!res.second)
				return res;
			TreeMapIterator resIter = mTreeMap.AddCrt(IteratorProxy::GetBaseIterator(res.first),
				std::forward<RKey>(std::get<0>(key)), std::move(mappedCreator));
			return { IteratorProxy(resIter), true };
		}

	private:
		TreeMap mTreeMap;
	};
}

/*!
	\brief
	`momo::stdish::map` is similar to `std::map`, but much more
	efficient in memory usage. The implementation is based on a B-tree.

	\details
	Deviations from standard class:
	1. Container items must be movable (preferably without exceptions)
	or copyable, similar to items of `std::vector`.
	2. After each addition or removal of the item all iterators and
	references to items become invalid and should not be used.
	3. Type `reference` is not the same as `value_type&`, so
	`for (auto& p : map)` is illegal, but `for (auto p : map)` or
	`for (const auto& p : map)` or `for (auto&& p : map)` is allowed.
	4. Functions `begin`, `cbegin`, `rend` and `crend` have logarithmic
	complexity.
	5. If `ObjectManager<key_type>::isNothrowAnywayAssignable` is false
	or `ObjectManager<mapped_type>::isNothrowAnywayAssignable` is false,
	functions `erase` can throw exceptions.
	6. Functions `merge`, `extract` and `insert(node_type&&)` move items.

	It is allowed to pass to functions `insert` and `emplace` references
	to items within the container.

	Function `merge` can work fast, if container types are same and each
	key from one container is less than each key from other container.
*/

template<typename TKey, typename TMapped,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>,
	typename TTreeMap = TreeMap<TKey, TMapped, TreeTraitsStd<TKey, TLessFunc>,
		MemManagerStd<TAllocator>>>
class map : public internal::map_base<TKey, TMapped, TLessFunc, TAllocator, TTreeMap>
{
private:
	typedef internal::map_base<TKey, TMapped, TLessFunc, TAllocator, TTreeMap> BaseMap;
	typedef TTreeMap TreeMap;

public:
	using typename BaseMap::key_type;
	using typename BaseMap::mapped_type;
	using typename BaseMap::size_type;
	using typename BaseMap::value_type;
	using typename BaseMap::const_reference;
	using typename BaseMap::const_iterator;
	using typename BaseMap::iterator;

private:
	struct IteratorProxy : public BaseMap::iterator
	{
		typedef iterator Iterator;
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

public:
	using BaseMap::BaseMap;

	map& operator=(std::initializer_list<value_type> values)
	{
		BaseMap::ptAssign(values);
		return *this;
	}

	friend void swap(map& left, map& right) noexcept
	{
		left.swap(right);
	}

	typename TreeMap::ValueReferenceRKey operator[](key_type&& key)
	{
		return BaseMap::get_nested_container()[std::move(key)];
	}

	typename TreeMap::ValueReferenceCKey operator[](const key_type& key)
	{
		return BaseMap::get_nested_container()[key];
	}

	const mapped_type& at(const key_type& key) const
	{
		const_iterator iter = BaseMap::find(key);
		if (iter == BaseMap::end())
			throw std::out_of_range("invalid map key");
		return iter->second;
	}

	mapped_type& at(const key_type& key)
	{
		iterator iter = BaseMap::find(key);
		if (iter == BaseMap::end())
			throw std::out_of_range("invalid map key");
		return iter->second;
	}

	template<typename... MappedArgs>
	std::pair<iterator, bool> try_emplace(key_type&& key, MappedArgs&&... mappedArgs)
	{
		return BaseMap::ptEmplace(nullptr, std::forward_as_tuple(std::move(key)),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...));
	}

	template<typename... MappedArgs>
	iterator try_emplace(const_iterator hint, key_type&& key, MappedArgs&&... mappedArgs)
	{
		return BaseMap::ptEmplace(hint, std::forward_as_tuple(std::move(key)),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...)).first;
	}

	template<typename... MappedArgs>
	std::pair<iterator, bool> try_emplace(const key_type& key, MappedArgs&&... mappedArgs)
	{
		return BaseMap::ptEmplace(nullptr, std::forward_as_tuple(key),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...));
	}

	template<typename... MappedArgs>
	iterator try_emplace(const_iterator hint, const key_type& key, MappedArgs&&... mappedArgs)
	{
		return BaseMap::ptEmplace(hint, std::forward_as_tuple(key),
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

	template<typename Predicate>
	requires std::predicate<const Predicate&, const_reference>
	friend size_type erase_if(map& cont, const Predicate& pred)
	{
		auto pairPred = [&pred] (const key_type& key, const mapped_type& mapped)
			{ return pred(const_reference(key, mapped)); };
		return cont.get_nested_container().Remove(pairPred);
	}

private:
	template<typename RKey, typename MappedArg>
	std::pair<iterator, bool> pvInsertOrAssign(RKey&& key, MappedArg&& mappedArg)
	{
		typename TreeMap::InsertResult res = BaseMap::get_nested_container().InsertOrAssign(
			std::forward<RKey>(key), std::forward<MappedArg>(mappedArg));
		return { IteratorProxy(res.position), res.inserted };
	}

	template<typename RKey, typename MappedArg>
	iterator pvInsertOrAssign(const_iterator hint, RKey&& key, MappedArg&& mappedArg)
	{
		std::pair<iterator, bool> res = BaseMap::ptEmplace(hint,
			std::forward_as_tuple(std::forward<RKey>(key)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg)));
		if (!res.second)
		{
			TreeMap::KeyValueTraits::AssignValue(BaseMap::get_nested_container().GetMemManager(),
				std::forward<MappedArg>(mappedArg), res.first->second);
		}
		return res.first;
	}
};

/*!
	\brief
	`momo::stdish::multimap` is similar to `std::multimap`, but much more
	efficient in memory usage. The implementation is based on a B-tree.

	\copydetails momo::stdish::map
*/

template<typename TKey, typename TMapped,
	typename TLessFunc = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>,
	typename TTreeMap = TreeMap<TKey, TMapped, TreeTraitsStd<TKey, TLessFunc, true>,
		MemManagerStd<TAllocator>>>
class multimap : public internal::map_base<TKey, TMapped, TLessFunc, TAllocator, TTreeMap>
{
private:
	typedef internal::map_base<TKey, TMapped, TLessFunc, TAllocator, TTreeMap> BaseMap;

public:
	using typename BaseMap::key_type;
	using typename BaseMap::mapped_type;
	using typename BaseMap::size_type;
	using typename BaseMap::value_type;
	using typename BaseMap::iterator;
	using typename BaseMap::const_reference;
	using typename BaseMap::node_type;

	typedef iterator insert_return_type;

public:
	using BaseMap::BaseMap;

	multimap& operator=(std::initializer_list<value_type> values)
	{
		BaseMap::ptAssign(values);
		return *this;
	}

	friend void swap(multimap& left, multimap& right) noexcept
	{
		left.swap(right);
	}

	using BaseMap::insert;

	iterator insert(std::pair<key_type, mapped_type>&& value)
	{
		return BaseMap::insert(std::move(value)).first;
	}

	template<typename First, typename Second>
	requires std::is_constructible_v<key_type, const First&>
		&& std::is_constructible_v<mapped_type, const Second&>
	iterator insert(const std::pair<First, Second>& value)
	{
		return BaseMap::insert(value).first;
	}

	template<typename First, typename Second>
	requires std::is_constructible_v<key_type, First&&>
		&& std::is_constructible_v<mapped_type, Second&&>
	iterator insert(std::pair<First, Second>&& value)
	{
		return BaseMap::insert(std::move(value)).first;
	}

	iterator insert(node_type&& node)
	{
		return BaseMap::insert(std::move(node)).position;
	}

	template<typename... ValueArgs>
	iterator emplace(ValueArgs&&... valueArgs)
	{
		return BaseMap::emplace(std::forward<ValueArgs>(valueArgs)...).first;
	}

	template<typename Predicate>
	requires std::predicate<const Predicate&, const_reference>
	friend size_type erase_if(multimap& cont, const Predicate& pred)
	{
		auto pairPred = [&pred] (const key_type& key, const mapped_type& mapped)
			{ return pred(const_reference(key, mapped)); };
		return cont.get_nested_container().Remove(pairPred);
	}
};

#define MOMO_DECLARE_DEDUCTION_GUIDES(map) \
template<typename Iterator, \
	typename Key = std::remove_const_t<typename std::iter_value_t<Iterator>::first_type>, \
	typename Mapped = typename std::iter_value_t<Iterator>::second_type, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
requires momo::internal::conceptAllocator<Allocator> \
map(Iterator, Iterator, Allocator = Allocator()) \
	-> map<Key, Mapped, std::less<Key>, Allocator>; \
template<typename Iterator, typename LessFunc, \
	typename Key = std::remove_const_t<typename std::iter_value_t<Iterator>::first_type>, \
	typename Mapped = typename std::iter_value_t<Iterator>::second_type, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
requires momo::internal::conceptLessFunc<LessFunc, Key> && \
	momo::internal::conceptAllocator<Allocator> \
map(Iterator, Iterator, LessFunc, Allocator = Allocator()) \
	-> map<Key, Mapped, LessFunc, Allocator>; \
template<typename Key, typename Mapped, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
requires momo::internal::conceptAllocator<Allocator> \
map(std::initializer_list<std::pair<Key, Mapped>>, Allocator = Allocator()) \
	-> map<Key, Mapped, std::less<Key>, Allocator>; \
template<typename Key, typename Mapped, typename LessFunc, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>> \
requires momo::internal::conceptLessFunc<LessFunc, Key> && \
	momo::internal::conceptAllocator<Allocator> \
map(std::initializer_list<std::pair<Key, Mapped>>, LessFunc, Allocator = Allocator()) \
	-> map<Key, Mapped, LessFunc, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES(map)
MOMO_DECLARE_DEDUCTION_GUIDES(multimap)

#undef MOMO_DECLARE_DEDUCTION_GUIDES

} // namespace momo::stdish