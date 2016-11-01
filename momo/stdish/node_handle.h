/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/stdish/node_handle.h

\**********************************************************/

#pragma once

#include "../SetUtility.h"
#include "../MapUtility.h"

namespace momo
{

namespace stdish
{

namespace internal
{
	template<typename TSetExtractedItem>
	class set_node_handle
	{
	private:
		typedef TSetExtractedItem SetExtractedItem;

	public:
		typedef typename SetExtractedItem::Item value_type;
		typedef typename SetExtractedItem::MemManager::Allocator allocator_type;

	public:
		set_node_handle() MOMO_NOEXCEPT
		{
		}

		set_node_handle(set_node_handle&& node) //MOMO_NOEXCEPT_IF
			: mSetExtractedItem(std::move(node.mSetExtractedItem))
		{
		}

		template<typename Set>
		set_node_handle(Set& set, typename Set::const_iterator iter)
		{
			set.get_nested_container().Remove(iter, mSetExtractedItem);
		}

		~set_node_handle() MOMO_NOEXCEPT
		{
		}

		set_node_handle& operator=(const set_node_handle&) = delete;

		bool empty() const MOMO_NOEXCEPT
		{
			return mSetExtractedItem.IsEmpty();
		}

		explicit operator bool() const MOMO_NOEXCEPT
		{
			return empty();
		}

		allocator_type get_allocator() const
		{
			return allocator_type(mSetExtractedItem.GetMemManager().GetCharAllocator());
		}

		value_type& value() const
		{
			return mSetExtractedItem.GetItem();
		}

	private:
		mutable SetExtractedItem mSetExtractedItem;
	};

	template<typename TMapExtractedPair>
	class map_node_handle
	{
	private:
		typedef TMapExtractedPair MapExtractedPair;

	public:
		typedef typename MapExtractedPair::Key key_type;
		typedef typename MapExtractedPair::Value mapped_type;
		typedef typename MapExtractedPair::MemManager::Allocator allocator_type;

	public:
		map_node_handle() MOMO_NOEXCEPT
		{
		}

		map_node_handle(map_node_handle&& node) //MOMO_NOEXCEPT_IF
			: mMapExtractedPair(std::move(node.mMapExtractedPair))
		{
		}

		template<typename Map>
		map_node_handle(Map& map, typename Map::const_iterator iter)
		{
			map.get_nested_container().Remove(iter.GetBaseIterator(), mMapExtractedPair);
		}

		~map_node_handle() MOMO_NOEXCEPT
		{
		}

		map_node_handle& operator=(const map_node_handle&) = delete;

		bool empty() const MOMO_NOEXCEPT
		{
			return mMapExtractedPair.IsEmpty();
		}

		explicit operator bool() const MOMO_NOEXCEPT
		{
			return empty();
		}

		allocator_type get_allocator() const
		{
			return allocator_type(mMapExtractedPair.GetMemManager().GetCharAllocator());
		}

		key_type& key() const
		{
			return mMapExtractedPair.GetKey();
		}

		mapped_type& mapped() const
		{
			return mMapExtractedPair.GetValue();
		}

	private:
		mutable MapExtractedPair mMapExtractedPair;
	};
}

} // namespace stdish

} // namespace momo
