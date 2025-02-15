/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/stdish/set_map_utility.h

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_STDISH_SET_MAP_UTILITY
#define MOMO_INCLUDE_GUARD_STDISH_SET_MAP_UTILITY

#include "../SetUtility.h"
#include "../MapUtility.h"

#ifdef MOMO_HAS_CONTAINERS_RANGES
# include <ranges>
#endif

namespace momo
{

namespace stdish
{

namespace internal
{
	template<typename TSetExtractedItem>
	class set_node_handle
	{
	protected:
		typedef TSetExtractedItem SetExtractedItem;

	public:
		typedef typename SetExtractedItem::Item value_type;
		//allocator_type;

	public:
		set_node_handle() noexcept
		{
		}

		set_node_handle(set_node_handle&& node)
			noexcept(std::is_nothrow_move_constructible<SetExtractedItem>::value)
			: mSetExtractedItem(std::move(node.mSetExtractedItem))
		{
		}

		set_node_handle(const set_node_handle&) = delete;

		template<typename Set>
		explicit set_node_handle(Set& set, typename Set::const_iterator iter)
			: mSetExtractedItem(set.get_nested_container(), iter)
		{
		}

		~set_node_handle() = default;

		//set_node_handle& operator=(set_node_handle&&)

		set_node_handle& operator=(const set_node_handle&) = delete;

		MOMO_NODISCARD bool empty() const noexcept
		{
			return mSetExtractedItem.IsEmpty();
		}

		explicit operator bool() const noexcept
		{
			return !empty();
		}

		const value_type& value() const
		{
			return mSetExtractedItem.GetItem();
		}

		value_type& value()
		{
			return mSetExtractedItem.GetItem();
		}

		//allocator_type get_allocator() const

		//void swap(set_node_handle&)
		//friend void swap(set_node_handle&, set_node_handle&)

	protected:
		SetExtractedItem& ptGetExtractedItem() noexcept
		{
			return mSetExtractedItem;
		}

	private:
		SetExtractedItem mSetExtractedItem;
	};

	template<typename TMapExtractedPair>
	class map_node_handle
	{
	protected:
		typedef TMapExtractedPair MapExtractedPair;

	public:
		typedef typename MapExtractedPair::Key key_type;
		typedef typename MapExtractedPair::Value mapped_type;
		//allocator_type;

	private:
		template<typename Map>
		struct ConstIteratorProxy : private Map::const_iterator
		{
			typedef typename Map::const_iterator ConstIterator;
			MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBaseIterator,
				typename ConstIterator::BaseIterator)
		};

	public:
		map_node_handle() noexcept
		{
		}

		map_node_handle(map_node_handle&& node)
			noexcept(std::is_nothrow_move_constructible<MapExtractedPair>::value)
			: mMapExtractedPair(std::move(node.mMapExtractedPair))
		{
		}

		map_node_handle(const map_node_handle&) = delete;

		template<typename Map>
		explicit map_node_handle(Map& map, typename Map::const_iterator iter)
			: mMapExtractedPair(map.get_nested_container(),
				ConstIteratorProxy<Map>::GetBaseIterator(iter))
		{
		}

		~map_node_handle() = default;

		//map_node_handle& operator=(map_node_handle&&)

		map_node_handle& operator=(const map_node_handle&) = delete;

		MOMO_NODISCARD bool empty() const noexcept
		{
			return mMapExtractedPair.IsEmpty();
		}

		explicit operator bool() const noexcept
		{
			return !empty();
		}

		const key_type& key() const
		{
			return mMapExtractedPair.GetKey();
		}

		key_type& key()
		{
			return mMapExtractedPair.GetKey();
		}

		const mapped_type& mapped() const
		{
			return mMapExtractedPair.GetValue();
		}

		mapped_type& mapped()
		{
			return mMapExtractedPair.GetValue();
		}

		//allocator_type get_allocator() const

		//void swap(map_node_handle&)
		//friend void swap(map_node_handle&, map_node_handle&)

	protected:
		MapExtractedPair& ptGetExtractedPair() noexcept
		{
			return mMapExtractedPair;
		}

	private:
		MapExtractedPair mMapExtractedPair;
	};

	template<typename Iterator, typename NodeHandle>
	struct insert_return_type
	{
		Iterator position;
		bool inserted;
		NodeHandle node;
	};

#ifdef MOMO_HAS_DEDUCTION_GUIDES
	template<typename Key, typename Allocator, typename HashFunc,
		typename EqualFunc = std::equal_to<Key>,
		typename = decltype(std::declval<Allocator&>().allocate(size_t{})),
		typename = decltype(std::declval<HashFunc&>()(std::declval<const Key&>())),
		typename = decltype(std::declval<EqualFunc&>()(std::declval<const Key&>(),
			std::declval<const Key&>()))>
	class unordered_checker
	{
	};

	template<typename Key, typename Allocator,
		typename LessComparer = std::less<Key>,
		typename = decltype(std::declval<Allocator&>().allocate(size_t{})),
		typename = decltype(std::declval<LessComparer&>()(std::declval<const Key&>(),
			std::declval<const Key&>()))>
	class ordered_checker
	{
	};
#endif // MOMO_HAS_DEDUCTION_GUIDES
}

} // namespace stdish

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_STDISH_SET_MAP_UTILITY
