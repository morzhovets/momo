/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/stdish/map.h

  namespace momo::stdish:
    class map_adaptor
    class multimap_adaptor
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
	template<typename TKey, typename TLessComparer>
	class map_value_compare
	{
	protected:
		typedef TKey Key;
		typedef TLessComparer LessComparer;

	public:
		template<typename Value>
		requires std::is_same_v<Key, std::decay_t<decltype(std::declval<Value>().first)>>
		bool operator()(const Value& value1, const Value& value2) const
		{
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

	template<typename TTreeMap>
	class map_adaptor_base
	{
	private:
		typedef TTreeMap TreeMap;
		typedef typename TreeMap::TreeTraits TreeTraits;
		typedef typename TreeMap::MemManager MemManager;

		typedef typename TreeMap::Iterator TreeMapIterator;

	public:
		typedef typename TreeMap::Key key_type;
		typedef typename TreeMap::Value mapped_type;
		typedef typename TreeTraits::LessComparer key_compare;

		typedef TreeMap nested_container_type;

		typedef size_t size_type;
		typedef ptrdiff_t difference_type;

		typedef std::pair<const key_type, mapped_type> value_type;
		typedef typename std::allocator_traits<typename MemManager::ByteAllocator>
			::template rebind_alloc<value_type> allocator_type;

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
		using IsValidKeyArg = TreeTraits::template IsValidKeyArg<KeyArg>;

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
		map_adaptor_base()
		{
		}

		explicit map_adaptor_base(const allocator_type& alloc)
			: mTreeMap(TreeTraits(), MemManager(alloc))
		{
		}

		explicit map_adaptor_base(const key_compare& lessComp,
			const allocator_type& alloc = allocator_type())
			: mTreeMap(TreeTraits(lessComp), MemManager(alloc))
		{
		}

		template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
		map_adaptor_base(Iterator first, Iterator last, const allocator_type& alloc = allocator_type())
			: map_adaptor_base(alloc)
		{
			insert(first, last);
		}

		template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
		map_adaptor_base(Iterator first, Iterator last, const key_compare& lessComp,
			const allocator_type& alloc = allocator_type())
			: map_adaptor_base(lessComp, alloc)
		{
			insert(first, last);
		}

		map_adaptor_base(std::initializer_list<value_type> values,
			const allocator_type& alloc = allocator_type())
			: map_adaptor_base(values.begin(), values.end(), alloc)
		{
		}

		map_adaptor_base(std::initializer_list<value_type> values, const key_compare& lessComp,
			const allocator_type& alloc = allocator_type())
			: map_adaptor_base(values.begin(), values.end(), lessComp, alloc)
		{
		}

#if defined(__cpp_lib_containers_ranges)
		template<std::ranges::input_range Range>
		requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
		map_adaptor_base(std::from_range_t, Range&& values, const allocator_type& alloc = allocator_type())
			: map_adaptor_base(alloc)
		{
			insert_range(std::forward<Range>(values));
		}

		template<std::ranges::input_range Range>
		requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
		map_adaptor_base(std::from_range_t, Range&& values, const key_compare& lessComp,
			const allocator_type& alloc = allocator_type())
			: map_adaptor_base(lessComp, alloc)
		{
			insert_range(std::forward<Range>(values));
		}
#endif // __cpp_lib_containers_ranges

		map_adaptor_base(map_adaptor_base&& right)
			: map_adaptor_base(std::move(right), right.get_allocator())
		{
		}

		map_adaptor_base(map_adaptor_base&& right, const allocator_type& alloc)
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

		map_adaptor_base(const map_adaptor_base& right)
			: mTreeMap(right.mTreeMap)
		{
		}

		map_adaptor_base(const map_adaptor_base& right, const allocator_type& alloc)
			: mTreeMap(right.mTreeMap, MemManager(alloc))
		{
		}

		~map_adaptor_base() noexcept = default;

		map_adaptor_base& operator=(map_adaptor_base&& right)
			noexcept(momo::internal::ContainerAssignerStd::isNothrowMoveAssignable<map_adaptor_base>)
		{
			return momo::internal::ContainerAssignerStd::Move(std::move(right), *this);
		}

		map_adaptor_base& operator=(const map_adaptor_base& right)
		{
			return momo::internal::ContainerAssignerStd::Copy(right, *this);
		}

		template<momo::internal::conceptMutableThis RMap>
		std::remove_reference_t<RMap>& operator=(this RMap&& left,
			std::initializer_list<value_type> values)
		{
			left.mTreeMap = TreeMap(values, left.mTreeMap.GetTreeTraits(), MemManager(left.get_allocator()));
			return left;
		}

		void swap(map_adaptor_base& right) noexcept
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

		template<typename ValueArg = std::pair<key_type, mapped_type>>
		requires std::is_constructible_v<value_type, ValueArg&&>
		std::pair<iterator, bool> insert(ValueArg&& valueArg)
		{
			return emplace(std::forward<ValueArg>(valueArg));
		}

		template<typename ValueArg = std::pair<key_type, mapped_type>>
		requires std::is_constructible_v<value_type, ValueArg&&>
		iterator insert(const_iterator hint, ValueArg&& valueArg)
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

		template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
		void insert(Iterator first, Iterator last)
		{
			pvInsertRange(first, last);
		}

		void insert(std::initializer_list<value_type> values)
		{
			mTreeMap.Insert(values.begin(), values.end());
		}

		template<std::ranges::input_range Range>
		requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
		void insert_range(Range&& values)
		{
			pvInsertRange(std::ranges::begin(values), std::ranges::end(values));
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

		bool operator==(const map_adaptor_base& right) const
		{
			return size() == right.size() && std::equal(begin(), end(), right.begin());
		}

		auto operator<=>(const map_adaptor_base& right) const
			requires (std::three_way_comparable<const_reference>)
		{
			return std::lexicographical_compare_three_way(begin(), end(),
				right.begin(), right.end());
		}

		template<momo::internal::conceptPredicate<const_reference> ValueFilter>
		friend size_type erase_if(map_adaptor_base& cont, ValueFilter valueFilter)
		{
			momo::FastCopyableFunctor fastValueFilter(valueFilter);
			auto pairFilter = [fastValueFilter] (const key_type& key, const mapped_type& mapped)
				{ return fastValueFilter(const_reference(key, mapped)); };
			return cont.mTreeMap.Remove(pairFilter);
		}

	protected:	//?
		template<typename Hint, typename... KeyArgs, typename... MappedArgs>
		std::pair<iterator, bool> ptEmplace(Hint hint, std::tuple<KeyArgs...>&& keyArgs,
			std::tuple<MappedArgs...>&& mappedArgs)
		{
			typedef typename TreeMap::KeyValueTraits
				::template ValueCreator<MappedArgs...> MappedCreator;
			return pvInsert(hint, std::move(keyArgs),
				FastMovableFunctor(MappedCreator(mTreeMap.GetMemManager(), std::move(mappedArgs))));
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
				if constexpr (TreeTraits::multiKey)
					return { lower_bound(key), true };
				else
					return pvFind(nullptr, key);
			}
			return { IteratorProxy(mTreeMap.MakeMutableIterator(
				ConstIteratorProxy::GetBaseIterator(hint))), true };
		}

		template<typename Hint, typename... KeyArgs,
			momo::internal::conceptObjectCreator<mapped_type> MappedCreator>
		std::pair<iterator, bool> pvInsert(Hint hint, std::tuple<KeyArgs...>&& keyArgs,
			FastMovableFunctor<MappedCreator> mappedCreator)
		{
			MemManager& memManager = mTreeMap.GetMemManager();
			typedef momo::internal::ObjectBuffer<key_type,
				TreeMap::KeyValueTraits::keyAlignment> KeyBuffer;
			typedef momo::internal::ObjectManager<key_type, MemManager> KeyManager;
			typedef typename KeyManager::template Creator<KeyArgs...> KeyCreator;
			typedef typename KeyManager::template DestroyFinalizer<> KeyDestroyFinalizer;
			KeyBuffer keyBuffer;
			KeyCreator(memManager, std::move(keyArgs))(keyBuffer.GetPtr());
			KeyDestroyFinalizer keyFin(&memManager, keyBuffer.template GetPtr<true>());
			std::pair<iterator, bool> res = pvFind(hint, std::as_const(keyBuffer.Get()));
			if (!res.second)
				return res;
			auto valueCreator = [mappedCreator = std::move(mappedCreator), keyFin = std::move(keyFin)]
				(key_type* newKey, mapped_type* newMapped) mutable
			{
				KeyManager::Relocate(*keyFin.GetMemManager(), *keyFin.GetPtr(), newKey);
				keyFin.ResetPtr(newKey);
				std::move(mappedCreator)(newMapped);
				keyFin.ResetPtr();
			};
			TreeMapIterator resIter = mTreeMap.AddCrt(
				IteratorProxy::GetBaseIterator(res.first), std::move(valueCreator));
			return { IteratorProxy(resIter), true };
		}

		template<typename Hint, typename RKey,
			momo::internal::conceptObjectCreator<mapped_type> MappedCreator,
			typename Key = std::decay_t<RKey>>
		requires std::is_same_v<key_type, Key>
		std::pair<iterator, bool> pvInsert(Hint hint, std::tuple<RKey>&& key,
			FastMovableFunctor<MappedCreator> mappedCreator)
		{
			std::pair<iterator, bool> res = pvFind(hint, std::as_const(std::get<0>(key)));
			if (!res.second)
				return res;
			TreeMapIterator resIter = mTreeMap.AddCrt(IteratorProxy::GetBaseIterator(res.first),
				std::forward<RKey>(std::get<0>(key)), std::move(mappedCreator));
			return { IteratorProxy(resIter), true };
		}

		template<std::input_iterator Iterator,
			momo::internal::conceptSentinel<Iterator> Sentinel>
		void pvInsertRange(Iterator begin, Sentinel end)
		{
			if constexpr (momo::internal::conceptMapArgIterator<Iterator, key_type, false>)
			{
				mTreeMap.Insert(std::move(begin), std::move(end));
			}
			else
			{
				for (Iterator iter = std::move(begin); iter != end; ++iter)
					insert(*iter);
			}
		}

	private:
		TreeMap mTreeMap;
	};
}

template<typename TTreeMap>
class MOMO_EMPTY_BASES map_adaptor
	: public internal::map_adaptor_base<TTreeMap>,
	public momo::internal::Swappable<map_adaptor>
{
private:
	typedef TTreeMap TreeMap;
	typedef internal::map_adaptor_base<TreeMap> MapAdaptorBase;

public:
	using typename MapAdaptorBase::key_type;
	using typename MapAdaptorBase::mapped_type;
	using typename MapAdaptorBase::const_iterator;
	using typename MapAdaptorBase::iterator;

private:
	struct IteratorProxy : public MapAdaptorBase::iterator
	{
		typedef iterator Iterator;
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

public:
	using MapAdaptorBase::MapAdaptorBase;

	using MapAdaptorBase::operator=;

	decltype(auto) operator[](key_type&& key)
	{
		return MapAdaptorBase::get_nested_container()[std::move(key)];
	}

	decltype(auto) operator[](const key_type& key)
	{
		return MapAdaptorBase::get_nested_container()[key];
	}

	const mapped_type& at(const key_type& key) const
	{
		const_iterator iter = MapAdaptorBase::find(key);
		if (iter == MapAdaptorBase::end())
			MOMO_THROW(std::out_of_range("invalid map key"));
		return iter->second;
	}

	mapped_type& at(const key_type& key)
	{
		iterator iter = MapAdaptorBase::find(key);
		if (iter == MapAdaptorBase::end())
			MOMO_THROW(std::out_of_range("invalid map key"));
		return iter->second;
	}

	template<typename... MappedArgs>
	std::pair<iterator, bool> try_emplace(key_type&& key, MappedArgs&&... mappedArgs)
	{
		return MapAdaptorBase::ptEmplace(nullptr, std::forward_as_tuple(std::move(key)),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...));
	}

	template<typename... MappedArgs>
	iterator try_emplace(const_iterator hint, key_type&& key, MappedArgs&&... mappedArgs)
	{
		return MapAdaptorBase::ptEmplace(hint, std::forward_as_tuple(std::move(key)),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...)).first;
	}

	template<typename... MappedArgs>
	std::pair<iterator, bool> try_emplace(const key_type& key, MappedArgs&&... mappedArgs)
	{
		return MapAdaptorBase::ptEmplace(nullptr, std::forward_as_tuple(key),
			std::forward_as_tuple(std::forward<MappedArgs>(mappedArgs)...));
	}

	template<typename... MappedArgs>
	iterator try_emplace(const_iterator hint, const key_type& key, MappedArgs&&... mappedArgs)
	{
		return MapAdaptorBase::ptEmplace(hint, std::forward_as_tuple(key),
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

private:
	template<typename RKey, typename MappedArg>
	std::pair<iterator, bool> pvInsertOrAssign(RKey&& key, MappedArg&& mappedArg)
	{
		typename TreeMap::InsertResult res = MapAdaptorBase::get_nested_container().InsertOrAssign(
			std::forward<RKey>(key), std::forward<MappedArg>(mappedArg));
		return { IteratorProxy(res.position), res.inserted };
	}

	template<typename RKey, typename MappedArg>
	iterator pvInsertOrAssign(const_iterator hint, RKey&& key, MappedArg&& mappedArg)
	{
		std::pair<iterator, bool> res = MapAdaptorBase::ptEmplace(hint,
			std::forward_as_tuple(std::forward<RKey>(key)),
			std::forward_as_tuple(std::forward<MappedArg>(mappedArg)));
		if (!res.second)
		{
			TreeMap::KeyValueTraits::AssignValue(MapAdaptorBase::get_nested_container().GetMemManager(),
				std::forward<MappedArg>(mappedArg), res.first->second);
		}
		return res.first;
	}
};

template<typename TTreeMap>
class MOMO_EMPTY_BASES multimap_adaptor
	: public internal::map_adaptor_base<TTreeMap>,
	public momo::internal::Swappable<multimap_adaptor>
{
private:
	typedef internal::map_adaptor_base<TTreeMap> MapAdaptorBase;

public:
	using typename MapAdaptorBase::key_type;
	using typename MapAdaptorBase::mapped_type;
	using typename MapAdaptorBase::value_type;
	using typename MapAdaptorBase::iterator;
	using typename MapAdaptorBase::const_iterator;
	using typename MapAdaptorBase::node_type;

	typedef iterator insert_return_type;

public:
	using MapAdaptorBase::MapAdaptorBase;

	using MapAdaptorBase::operator=;

	//using MapAdaptorBase::insert;	// vs clang

	template<typename ValueArg = std::pair<key_type, mapped_type>>
	requires std::is_constructible_v<value_type, ValueArg&&>
	iterator insert(ValueArg&& valueArg)
	{
		return MapAdaptorBase::insert(std::forward<ValueArg>(valueArg)).first;
	}

	template<typename ValueArg = std::pair<key_type, mapped_type>>
	requires std::is_constructible_v<value_type, ValueArg&&>
	iterator insert(const_iterator hint, ValueArg&& valueArg)
	{
		return MapAdaptorBase::insert(hint, std::forward<ValueArg>(valueArg));
	}

	iterator insert(node_type&& node)
	{
		return MapAdaptorBase::insert(std::move(node)).position;
	}

	iterator insert(const_iterator hint, node_type&& node)
	{
		return MapAdaptorBase::insert(hint, std::move(node));
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	void insert(Iterator first, Iterator last)
	{
		MapAdaptorBase::insert(first, last);
	}

	void insert(std::initializer_list<value_type> values)
	{
		MapAdaptorBase::insert(values);
	}

	template<typename... ValueArgs>
	iterator emplace(ValueArgs&&... valueArgs)
	{
		return MapAdaptorBase::emplace(std::forward<ValueArgs>(valueArgs)...).first;
	}
};

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
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
class MOMO_EMPTY_BASES map
	: public map_adaptor<TreeMap<TKey, TMapped,
		TreeTraitsStd<TKey, TLessComparer>, MemManagerStd<TAllocator>>>,
	public momo::internal::Swappable<map>
{
private:
	typedef map_adaptor<TreeMap<TKey, TMapped,
		TreeTraitsStd<TKey, TLessComparer>, MemManagerStd<TAllocator>>> MapAdaptor;

public:
	using MapAdaptor::MapAdaptor;

	using MapAdaptor::operator=;
};

/*!
	\brief
	`momo::stdish::multimap` is similar to `std::multimap`, but much more
	efficient in memory usage. The implementation is based on a B-tree.

	\copydetails momo::stdish::map
*/

template<typename TKey, typename TMapped,
	typename TLessComparer = std::less<TKey>,
	typename TAllocator = std::allocator<std::pair<const TKey, TMapped>>>
class MOMO_EMPTY_BASES multimap
	: public multimap_adaptor<TreeMap<TKey, TMapped,
		TreeTraitsStd<TKey, TLessComparer, true>, MemManagerStd<TAllocator>>>,
	public momo::internal::Swappable<multimap>
{
private:
	typedef multimap_adaptor<TreeMap<TKey, TMapped,
		TreeTraitsStd<TKey, TLessComparer, true>, MemManagerStd<TAllocator>>> MultiMapAdaptor;

public:
	using MultiMapAdaptor::MultiMapAdaptor;

	using MultiMapAdaptor::operator=;
};

#define MOMO_DECLARE_DEDUCTION_GUIDES(map) \
template<typename Iterator, \
	typename Value = std::iter_value_t<Iterator>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
map(Iterator, Iterator, Allocator = Allocator()) \
	-> map<Key, Mapped, std::less<Key>, Allocator>; \
template<typename Iterator, \
	typename Value = std::iter_value_t<Iterator>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	momo::internal::conceptCopyableLessComparer<Key> LessComparer, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
map(Iterator, Iterator, LessComparer, Allocator = Allocator()) \
	-> map<Key, Mapped, LessComparer, Allocator>; \
template<typename QKey, typename Mapped, \
	typename Key = std::remove_const_t<QKey>, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
map(std::initializer_list<std::pair<QKey, Mapped>>, Allocator = Allocator()) \
	-> map<Key, Mapped, std::less<Key>, Allocator>; \
template<typename QKey, typename Mapped, \
	typename Key = std::remove_const_t<QKey>, \
	momo::internal::conceptCopyableLessComparer<Key> LessComparer, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
map(std::initializer_list<std::pair<QKey, Mapped>>, LessComparer, Allocator = Allocator()) \
	-> map<Key, Mapped, LessComparer, Allocator>; \
template<typename Key, typename Mapped, typename LessComparer, typename Allocator> \
map(map<Key, Mapped, LessComparer, Allocator>, std::type_identity_t<Allocator>) \
	-> map<Key, Mapped, LessComparer, Allocator>;

#define MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(map) \
template<std::ranges::input_range Range, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
map(std::from_range_t, Range&&, Allocator = Allocator()) \
	-> map<Key, Mapped, std::less<Key>, Allocator>; \
template<std::ranges::input_range Range, \
	typename Value = std::ranges::range_value_t<Range>, \
	typename Key = std::decay_t<typename Value::first_type>, \
	typename Mapped = std::decay_t<typename Value::second_type>, \
	momo::internal::conceptCopyableLessComparer<Key> LessComparer, \
	momo::internal::conceptAllocator Allocator = std::allocator<std::pair<const Key, Mapped>>> \
map(std::from_range_t, Range&&, LessComparer, Allocator = Allocator()) \
	-> map<Key, Mapped, LessComparer, Allocator>;

MOMO_DECLARE_DEDUCTION_GUIDES(map)
MOMO_DECLARE_DEDUCTION_GUIDES(multimap)

#if defined(__cpp_lib_containers_ranges)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(map)
MOMO_DECLARE_DEDUCTION_GUIDES_RANGES(multimap)
#endif

#undef MOMO_DECLARE_DEDUCTION_GUIDES
#undef MOMO_DECLARE_DEDUCTION_GUIDES_RANGES

} // namespace momo::stdish
