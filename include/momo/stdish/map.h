/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/stdish/map.h

  namespace momo::stdish:
    class map
    class multimap

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_STDISH_MAP
#define MOMO_INCLUDE_GUARD_STDISH_MAP

#ifdef __has_include
# if __has_include(<momo/Utility.h>)
#  include <momo/Utility.h>
# endif
#endif
#ifndef MOMO_PARENT_HEADER
# include "../Utility.h"
#endif

#include MOMO_PARENT_HEADER(TreeMap)
#include "set_map_utility.h"

namespace momo
{

namespace stdish
{

namespace internal
{
	template<typename TKey, typename TLessComparer>
	class map_value_compare
	{
	protected:
		typedef TKey Key;
		typedef TLessComparer LessComparer;

	public:
		template<typename Value>
		bool operator()(const Value& value1, const Value& value2) const
		{
			MOMO_STATIC_ASSERT(std::is_same<Key,
				typename std::decay<decltype(value1.first)>::type>::value);
			return comp(value1.first, value2.first);
		}

	protected:
		map_value_compare(const LessComparer& lessComp)
			: comp(lessComp)
		{
		}

	protected:
		LessComparer comp;
	};

	template<typename TKey, typename TMapped, typename TLessComparer, typename TAllocator,
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
		typedef TLessComparer key_compare;
		typedef TAllocator allocator_type;

		typedef TreeMap nested_container_type;

		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		typedef std::pair<const key_type, mapped_type> value_type;

		typedef map_value_compare<key_type, key_compare> value_compare;

		typedef momo::internal::TreeDerivedIterator<TreeMapIterator,
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

		explicit map_base(const key_compare& lessComp, const allocator_type& alloc = allocator_type())
			: mTreeMap(TreeTraits(lessComp), MemManager(alloc))
		{
		}

		template<typename Iterator>
		map_base(Iterator first, Iterator last, const allocator_type& alloc = allocator_type())
			: map_base(alloc)
		{
			insert(first, last);
		}

		template<typename Iterator>
		map_base(Iterator first, Iterator last, const key_compare& lessComp,
			const allocator_type& alloc = allocator_type())
			: map_base(lessComp, alloc)
		{
			insert(first, last);
		}

		map_base(std::initializer_list<momo::internal::Identity<value_type>> values,
			const allocator_type& alloc = allocator_type())
			: map_base(values.begin(), values.end(), alloc)
		{
		}

		map_base(std::initializer_list<momo::internal::Identity<value_type>> values,
			const key_compare& lessComp, const allocator_type& alloc = allocator_type())
			: map_base(values.begin(), values.end(), lessComp, alloc)
		{
		}

#ifdef MOMO_HAS_CONTAINERS_RANGES
		template<std::ranges::input_range Range>
		requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
		map_base(std::from_range_t, Range&& values, const allocator_type& alloc = allocator_type())
			: map_base(alloc)
		{
			insert_range(std::forward<Range>(values));
		}

		template<std::ranges::input_range Range>
		requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
		map_base(std::from_range_t, Range&& values, const key_compare& lessComp,
			const allocator_type& alloc = allocator_type())
			: map_base(lessComp, alloc)
		{
			insert_range(std::forward<Range>(values));
		}
#endif // MOMO_HAS_CONTAINERS_RANGES

		map_base(map_base&& right)
			: map_base(std::move(right), right.get_allocator())
		{
		}

		map_base(map_base&& right, const momo::internal::Identity<allocator_type>& alloc)
			: mTreeMap(right.mTreeMap.GetTreeTraits(), MemManager(alloc))
		{
			if (right.get_allocator() == alloc)
			{
				mTreeMap.Swap(right.mTreeMap);
			}
			else
			{
				mTreeMap.MergeFrom(right.mTreeMap);
				right.clear();
			}
		}

		map_base(const map_base& right)
			: mTreeMap(right.mTreeMap)
		{
		}

		map_base(const map_base& right, const momo::internal::Identity<allocator_type>& alloc)
			: mTreeMap(right.mTreeMap, MemManager(alloc))
		{
		}

		~map_base() = default;

		map_base& operator=(map_base&& right)
			noexcept(momo::internal::ContainerAssignerStd::IsNothrowMoveAssignable<map_base>::value)
		{
			return momo::internal::ContainerAssignerStd::Move(std::move(right), *this);
		}

		map_base& operator=(const map_base& right)
		{
			return momo::internal::ContainerAssignerStd::Copy(right, *this);
		}

		void swap(map_base& right) noexcept
		{
			momo::internal::ContainerAssignerStd::Swap(*this, right);
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
			return mTreeMap.GetTreeTraits().GetLessComparer();
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

		MOMO_NODISCARD bool empty() const noexcept
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
		momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
		const_iterator> find(const KeyArg& key) const
		{
			return ConstIteratorProxy(mTreeMap.Find(key));
		}

		template<typename KeyArg>
		momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
		iterator> find(const KeyArg& key)
		{
			return IteratorProxy(mTreeMap.Find(key));
		}

		size_type count(const key_type& key) const
		{
			return mTreeMap.GetKeyCount(key);
		}

		template<typename KeyArg>
		momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
		size_type> count(const KeyArg& key) const
		{
			return mTreeMap.GetKeyCount(key);
		}

		bool contains(const key_type& key) const
		{
			return mTreeMap.ContainsKey(key);
		}

		template<typename KeyArg>
		momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
		bool> contains(const KeyArg& key) const
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
		momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
		const_iterator> lower_bound(const KeyArg& key) const
		{
			return ConstIteratorProxy(mTreeMap.GetLowerBound(key));
		}

		template<typename KeyArg>
		momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
		iterator> lower_bound(const KeyArg& key)
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
		momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
		const_iterator> upper_bound(const KeyArg& key) const
		{
			return ConstIteratorProxy(mTreeMap.GetUpperBound(key));
		}

		template<typename KeyArg>
		momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
		iterator> upper_bound(const KeyArg& key)
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
		momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
		std::pair<const_iterator, const_iterator>> equal_range(const KeyArg& key) const
		{
			return { lower_bound(key), upper_bound(key) };
		}

		template<typename KeyArg>
		momo::internal::EnableIf<IsValidKeyArg<KeyArg>::value,
		std::pair<iterator, iterator>> equal_range(const KeyArg& key)
		{
			return { lower_bound(key), upper_bound(key) };
		}

		template<typename ValueArg = std::pair<key_type, mapped_type>>
		momo::internal::EnableIf<std::is_constructible<value_type, ValueArg&&>::value,
		std::pair<iterator, bool>> insert(ValueArg&& valueArg)
		{
			return emplace(std::forward<ValueArg>(valueArg));
		}

		template<typename ValueArg = std::pair<key_type, mapped_type>>
		momo::internal::EnableIf<std::is_constructible<value_type, ValueArg&&>::value,
		iterator> insert(const_iterator hint, ValueArg&& valueArg)
		{
			return emplace_hint(hint, std::forward<ValueArg>(valueArg));
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

		template<typename Iterator>
		void insert(Iterator first, Iterator last)
		{
			pvInsertRange(first, last);
		}

		void insert(std::initializer_list<value_type> values)
		{
			mTreeMap.Insert(values.begin(), values.end());
		}

#ifdef MOMO_HAS_CONTAINERS_RANGES
		template<std::ranges::input_range Range>
		requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
		void insert_range(Range&& values)
		{
			pvInsertRange(std::ranges::begin(values), std::ranges::end(values));
		}
#endif // MOMO_HAS_CONTAINERS_RANGES

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
			return ptEmplace(nullptr,
				std::forward_as_tuple(std::get<0>(std::forward<ValueArg>(valueArg))),
				std::forward_as_tuple(std::get<1>(std::forward<ValueArg>(valueArg))));
		}

		template<typename ValueArg>
		iterator emplace_hint(const_iterator hint, ValueArg&& valueArg)
		{
			return ptEmplace(hint,
				std::forward_as_tuple(std::get<0>(std::forward<ValueArg>(valueArg))),
				std::forward_as_tuple(std::get<1>(std::forward<ValueArg>(valueArg)))).first;
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

		node_type extract(const_iterator where)
		{
			return node_type(*this, where);	// need RVO for exception safety
		}

		node_type extract(const key_type& key)
		{
			const_iterator iter = find(key);
			return (iter != end()) ? extract(iter) : node_type();
		}

		template<typename Map>
		void merge(Map&& map)
		{
			mTreeMap.MergeFrom(map.get_nested_container());
		}

		friend bool operator==(const map_base& left, const map_base& right)
		{
			return left.size() == right.size()
				&& std::equal(left.begin(), left.end(), right.begin());
		}

#ifdef MOMO_HAS_THREE_WAY_COMPARISON
		friend auto operator<=>(const map_base& left, const map_base& right)
			requires requires (const_reference ref) { ref <=> ref; }
		{
			return std::lexicographical_compare_three_way(left.begin(), left.end(),
				right.begin(), right.end());
		}
#else
		friend bool operator<(const map_base& left, const map_base& right)
		{
			return std::lexicographical_compare(left.begin(), left.end(),
				right.begin(), right.end());
		}
#endif

		MOMO_MORE_COMPARISON_OPERATORS(const map_base&)

	protected:	//?
		void ptAssign(std::initializer_list<value_type> values)
		{
			mTreeMap = TreeMap(values, mTreeMap.GetTreeTraits(), MemManager(get_allocator()));
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
			{
				if (TreeTraits::multiKey)
					return { lower_bound(key), true };
				else
					return pvFind(nullptr, key);
			}
			return { IteratorProxy(mTreeMap.MakeMutableIterator(
				ConstIteratorProxy::GetBaseIterator(hint))), true };
		}

		template<typename Hint, typename... KeyArgs, typename MappedCreator>
		std::pair<iterator, bool> pvInsert(Hint hint, std::tuple<KeyArgs...>&& keyArgs,
			MappedCreator&& mappedCreator)
		{
			MemManager& memManager = mTreeMap.GetMemManager();
			typedef momo::internal::ObjectBuffer<key_type,
				TreeMap::KeyValueTraits::keyAlignment> KeyBuffer;
			typedef momo::internal::ObjectManager<key_type, MemManager> KeyManager;
			typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
			KeyBuffer keyBuffer;
			KeyCreator(memManager, std::move(keyArgs))(keyBuffer.GetPtr());
			key_type* keyPtr = keyBuffer.template GetPtr<true>();
			try
			{
				std::pair<iterator, bool> res = pvFind(hint, *keyPtr);
				if (!res.second)
				{
					KeyManager::Destroy(memManager, *keyPtr);
					keyPtr = nullptr;
					return res;
				}
				auto valueCreator = [&memManager, &keyPtr, &mappedCreator]
					(key_type* newKey, mapped_type* newMapped)
				{
					KeyManager::Relocate(memManager, *keyPtr, newKey);
					keyPtr = nullptr;
					try
					{
						std::forward<MappedCreator>(mappedCreator)(newMapped);
					}
					catch (...)
					{
						KeyManager::Destroy(memManager, *newKey);
						throw;
					}
				};
				TreeMapIterator resIter = mTreeMap.AddCrt(
					IteratorProxy::GetBaseIterator(res.first), valueCreator);
				return { IteratorProxy(resIter), true };
			}
			catch (...)
			{
				if (keyPtr != nullptr)
					KeyManager::Destroy(memManager, *keyPtr);
				throw;
			}
		}

		template<typename Hint, typename RKey, typename MappedCreator,
			typename Key = typename std::decay<RKey>::type>
		momo::internal::EnableIf<std::is_same<key_type, Key>::value,
		std::pair<iterator, bool>> pvInsert(Hint hint, std::tuple<RKey>&& key,
			MappedCreator&& mappedCreator)
		{
			std::pair<iterator, bool> res = pvFind(hint,
				static_cast<const key_type&>(std::get<0>(key)));
			if (!res.second)
				return res;
			TreeMapIterator resIter = mTreeMap.AddCrt(IteratorProxy::GetBaseIterator(res.first),
				std::forward<RKey>(std::get<0>(key)), std::forward<MappedCreator>(mappedCreator));
			return { IteratorProxy(resIter), true };
		}

		template<typename Iterator, typename Sentinel>
		momo::internal::EnableIf<momo::internal::IsMapArgIteratorStd<Iterator, key_type>::value,
		void> pvInsertRange(Iterator begin, Sentinel end)
		{
			mTreeMap.Insert(std::move(begin), std::move(end));
		}

		template<typename Iterator, typename Sentinel>
		momo::internal::EnableIf<!momo::internal::IsMapArgIteratorStd<Iterator, key_type>::value,
		void> pvInsertRange(Iterator begin, Sentinel end)
		{
			for (Iterator iter = std::move(begin); iter != end; ++iter)
				insert(*iter);
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
	typename TLessComparer = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>,
	typename TTreeMap = TreeMap<TKey, TMapped, TreeTraitsStd<TKey, TLessComparer>,
		MemManagerStd<TAllocator>>>
class map : public internal::map_base<TKey, TMapped, TLessComparer, TAllocator, TTreeMap>
{
private:
	typedef internal::map_base<TKey, TMapped, TLessComparer, TAllocator, TTreeMap> MapBase;
	typedef TTreeMap TreeMap;

public:
	using typename MapBase::key_type;
	using typename MapBase::mapped_type;
	using typename MapBase::size_type;
	using typename MapBase::value_type;
	using typename MapBase::const_reference;
	using typename MapBase::const_iterator;
	using typename MapBase::iterator;

public:
	using MapBase::MapBase;

	map& operator=(std::initializer_list<value_type> values)
	{
		MapBase::ptAssign(values);
		return *this;
	}

	friend void swap(map& left, map& right) noexcept
	{
		left.swap(right);
	}

	typename TreeMap::ValueReferenceRKey operator[](key_type&& key)
	{
		return MapBase::get_nested_container()[std::move(key)];
	}

	typename TreeMap::ValueReferenceCKey operator[](const key_type& key)
	{
		return MapBase::get_nested_container()[key];
	}

	const mapped_type& at(const key_type& key) const
	{
		const_iterator iter = MapBase::find(key);
		if (iter == MapBase::end())
			MOMO_THROW(std::out_of_range("invalid map key"));
		return iter->second;
	}

	mapped_type& at(const key_type& key)
	{
		iterator iter = MapBase::find(key);
		if (iter == MapBase::end())
			MOMO_THROW(std::out_of_range("invalid map key"));
		return iter->second;
	}

	template<typename... MappedArgs>
	std::pair<iterator, bool> try_emplace(key_type&& key, MappedArgs&&... mappedArgs)
	{
		return MapBase::ptEmplace(nullptr, std::forward_as_tuple(std::move(key)),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...));
	}

	template<typename... MappedArgs>
	iterator try_emplace(const_iterator hint, key_type&& key, MappedArgs&&... mappedArgs)
	{
		return MapBase::ptEmplace(hint, std::forward_as_tuple(std::move(key)),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...)).first;
	}

	template<typename... MappedArgs>
	std::pair<iterator, bool> try_emplace(const key_type& key, MappedArgs&&... mappedArgs)
	{
		return MapBase::ptEmplace(nullptr, std::forward_as_tuple(key),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...));
	}

	template<typename... MappedArgs>
	iterator try_emplace(const_iterator hint, const key_type& key, MappedArgs&&... mappedArgs)
	{
		return MapBase::ptEmplace(hint, std::forward_as_tuple(key),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...)).first;
	}

	template<typename MappedArg>
	std::pair<iterator, bool> insert_or_assign(key_type&& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(nullptr, std::move(key), std::forward<MappedArg>(mappedArg));
	}

	template<typename MappedArg>
	iterator insert_or_assign(const_iterator hint, key_type&& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(hint, std::move(key), std::forward<MappedArg>(mappedArg)).first;
	}

	template<typename MappedArg>
	std::pair<iterator, bool> insert_or_assign(const key_type& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(nullptr, key, std::forward<MappedArg>(mappedArg));
	}

	template<typename MappedArg>
	iterator insert_or_assign(const_iterator hint, const key_type& key, MappedArg&& mappedArg)
	{
		return pvInsertOrAssign(hint, key, std::forward<MappedArg>(mappedArg)).first;
	}

	template<typename ValueFilter>
	friend size_type erase_if(map& cont, const ValueFilter& valueFilter)
	{
		auto pairFilter = [&valueFilter] (const key_type& key, const mapped_type& mapped)
			{ return valueFilter(const_reference(key, mapped)); };
		return cont.get_nested_container().Remove(pairFilter);
	}

private:
	template<typename Hint, typename RKey, typename MappedArg>
	std::pair<iterator, bool> pvInsertOrAssign(Hint hint, RKey&& key, MappedArg&& mappedArg)
	{
		std::pair<iterator, bool> res = MapBase::ptEmplace(hint,
			std::forward_as_tuple(std::forward<RKey>(key)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg)));
		if (!res.second)
			res.first->second = std::forward<MappedArg>(mappedArg);
		return res;
	}
};

/*!
	\brief
	`momo::stdish::multimap` is similar to `std::multimap`, but much more
	efficient in memory usage. The implementation is based on a B-tree.

	\copydetails momo::stdish::map
*/

template<typename TKey, typename TMapped,
	typename TLessComparer = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>,
	typename TTreeMap = TreeMap<TKey, TMapped, TreeTraitsStd<TKey, TLessComparer, true>,
		MemManagerStd<TAllocator>>>
class multimap : public internal::map_base<TKey, TMapped, TLessComparer, TAllocator, TTreeMap>
{
private:
	typedef internal::map_base<TKey, TMapped, TLessComparer, TAllocator, TTreeMap> MapBase;

public:
	using typename MapBase::key_type;
	using typename MapBase::mapped_type;
	using typename MapBase::size_type;
	using typename MapBase::value_type;
	using typename MapBase::iterator;
	using typename MapBase::const_reference;
	using typename MapBase::const_iterator;
	using typename MapBase::node_type;

	typedef iterator insert_return_type;

public:
	using MapBase::MapBase;

	multimap& operator=(std::initializer_list<value_type> values)
	{
		MapBase::ptAssign(values);
		return *this;
	}

	friend void swap(multimap& left, multimap& right) noexcept
	{
		left.swap(right);
	}

	//using MapBase::insert;	// gcc 5 & 6

	template<typename ValueArg = std::pair<key_type, mapped_type>>
	momo::internal::EnableIf<std::is_constructible<value_type, ValueArg&&>::value,
	iterator> insert(ValueArg&& valueArg)
	{
		return MapBase::insert(std::forward<ValueArg>(valueArg)).first;
	}

	template<typename ValueArg = std::pair<key_type, mapped_type>>
	momo::internal::EnableIf<std::is_constructible<value_type, ValueArg&&>::value,
	iterator> insert(const_iterator hint, ValueArg&& valueArg)
	{
		return MapBase::insert(hint, std::forward<ValueArg>(valueArg));
	}

	iterator insert(node_type&& node)
	{
		return MapBase::insert(std::move(node)).position;
	}

	iterator insert(const_iterator hint, node_type&& node)
	{
		return MapBase::insert(hint, std::move(node));
	}

	template<typename Iterator>
	void insert(Iterator first, Iterator last)
	{
		MapBase::insert(first, last);
	}

	void insert(std::initializer_list<value_type> values)
	{
		MapBase::insert(values);
	}

	template<typename... ValueArgs>
	iterator emplace(ValueArgs&&... valueArgs)
	{
		return MapBase::emplace(std::forward<ValueArgs>(valueArgs)...).first;
	}

	template<typename ValueFilter>
	friend size_type erase_if(multimap& cont, const ValueFilter& valueFilter)
	{
		auto pairFilter = [&valueFilter] (const key_type& key, const mapped_type& mapped)
			{ return valueFilter(const_reference(key, mapped)); };
		return cont.get_nested_container().Remove(pairFilter);
	}
};

#ifdef MOMO_HAS_DEDUCTION_GUIDES

#define MOMO_DECLARE_DEDUCTION_GUIDES(map) \
template<typename Iterator, \
	typename Value = typename std::iterator_traits<Iterator>::value_type, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::ordered_checker<Key, Allocator>> \
map(Iterator, Iterator, Allocator = Allocator()) \
	-> map<Key, Mapped, std::less<Key>, Allocator>; \
template<typename Iterator, typename LessComparer, \
	typename Value = typename std::iterator_traits<Iterator>::value_type, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::ordered_checker<Key, Allocator, LessComparer>> \
map(Iterator, Iterator, LessComparer, Allocator = Allocator()) \
	-> map<Key, Mapped, LessComparer, Allocator>; \
template<typename QKey, typename Mapped, \
	typename Key = std::remove_const_t<QKey>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::ordered_checker<Key, Allocator>> \
map(std::initializer_list<std::pair<QKey, Mapped>>, Allocator = Allocator()) \
	-> map<Key, Mapped, std::less<Key>, Allocator>; \
template<typename QKey, typename Mapped, typename LessComparer, \
	typename Key = std::remove_const_t<QKey>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::ordered_checker<Key, Allocator, LessComparer>> \
map(std::initializer_list<std::pair<QKey, Mapped>>, LessComparer, Allocator = Allocator()) \
	-> map<Key, Mapped, LessComparer, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES(map)
MOMO_DECLARE_DEDUCTION_GUIDES(multimap)

#undef MOMO_DECLARE_DEDUCTION_GUIDES

#ifdef MOMO_HAS_CONTAINERS_RANGES

#define MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(map) \
template<std::ranges::input_range Range, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::ordered_checker<Key, Allocator>> \
map(std::from_range_t, Range&&, Allocator = Allocator()) \
	-> map<Key, Mapped, std::less<Key>, Allocator>; \
template<std::ranges::input_range Range, typename LessComparer, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	typename Allocator = std::allocator<std::pair<const Key, Mapped>>, \
	typename = internal::ordered_checker<Key, Allocator, LessComparer>> \
map(std::from_range_t, Range&&, LessComparer, Allocator = Allocator()) \
	-> map<Key, Mapped, LessComparer, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(map)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(multimap)

#undef MOMO_DECLARE_DEDUCTION_GUIDES_RANGES

#endif // MOMO_HAS_CONTAINERS_RANGES

#endif // MOMO_HAS_DEDUCTION_GUIDES

} // namespace stdish

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_STDISH_MAP
