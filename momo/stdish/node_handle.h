/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/stdish/node_handle.h

\**********************************************************/

#pragma once

#include "../SetUtility.h"
#include "../MapUtility.h"

namespace momo::stdish
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
			noexcept(std::is_nothrow_move_constructible_v<SetExtractedItem>)
			: mSetExtractedItem(std::move(node.mSetExtractedItem))
		{
		}

		set_node_handle(const set_node_handle&) = delete;

		template<typename Set>
		explicit set_node_handle(Set& set, typename Set::const_iterator iter)
			: mSetExtractedItem(set.get_nested_container(), iter)
		{
		}

		~set_node_handle() noexcept = default;

		//set_node_handle& operator=(set_node_handle&&)

		set_node_handle& operator=(const set_node_handle&) = delete;

		[[nodiscard]] bool empty() const noexcept
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
			MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBaseIterator)
		};

	public:
		map_node_handle() noexcept
		{
		}

		map_node_handle(map_node_handle&& node)
			noexcept(std::is_nothrow_move_constructible_v<MapExtractedPair>)
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

		~map_node_handle() noexcept = default;

		//map_node_handle& operator=(map_node_handle&&)

		map_node_handle& operator=(const map_node_handle&) = delete;

		[[nodiscard]] bool empty() const noexcept
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
}

} // namespace momo::stdish
