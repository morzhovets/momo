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
	protected:
		typedef TSetExtractedItem SetExtractedItem;

	public:
		typedef typename SetExtractedItem::Item value_type;
		//allocator_type;

	public:
		set_node_handle() MOMO_NOEXCEPT
		{
		}

		set_node_handle(set_node_handle&& node)
			MOMO_NOEXCEPT_IF(noexcept(SetExtractedItem(std::move(node.mSetExtractedItem))))
			: mSetExtractedItem(std::move(node.mSetExtractedItem))
		{
		}

		set_node_handle(const set_node_handle&) = delete;

		template<typename Set>
		set_node_handle(Set& set, typename Set::ConstIterator iter)
			: mSetExtractedItem(set, iter)
		{
		}

		~set_node_handle() MOMO_NOEXCEPT
		{
		}

		//set_node_handle& operator=(set_node_handle&&)

		set_node_handle& operator=(const set_node_handle&) = delete;

		bool empty() const MOMO_NOEXCEPT
		{
			return mSetExtractedItem.IsEmpty();
		}

		explicit operator bool() const MOMO_NOEXCEPT
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
		SetExtractedItem& ptGetExtractedItem() MOMO_NOEXCEPT
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

	public:
		map_node_handle() MOMO_NOEXCEPT
		{
		}

		map_node_handle(map_node_handle&& node)
			MOMO_NOEXCEPT_IF(noexcept(MapExtractedPair(std::move(node.mMapExtractedPair))))
			: mMapExtractedPair(std::move(node.mMapExtractedPair))
		{
		}

		map_node_handle(const map_node_handle&) = delete;

		template<typename Map>
		map_node_handle(Map& map, typename Map::ConstIterator iter)
			: mMapExtractedPair(map, iter)
		{
		}

		~map_node_handle() MOMO_NOEXCEPT
		{
		}

		//map_node_handle& operator=(map_node_handle&&)

		map_node_handle& operator=(const map_node_handle&) = delete;

		bool empty() const MOMO_NOEXCEPT
		{
			return mMapExtractedPair.IsEmpty();
		}

		explicit operator bool() const MOMO_NOEXCEPT
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
		MapExtractedPair& ptGetExtractedPair() MOMO_NOEXCEPT
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

} // namespace stdish

} // namespace momo
