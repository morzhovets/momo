/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/DataRow.h

\**********************************************************/

#pragma once

#include "Utility.h"

#include <atomic>

namespace momo
{

namespace experimental
{

namespace internal
{
	template<typename TColumnList>
	class DataRow
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		template<typename Item>
		using Column = typename ColumnList::template Column<Item>;

	protected:
		typedef std::atomic<Raw*> FreeRaws;

	public:
		DataRow() = delete;

		DataRow(DataRow&& row) MOMO_NOEXCEPT
			: mColumnList(row.mColumnList),
			mRaw(row.mRaw),
			mFreeRaws(row.mFreeRaws)
		{
			row.mRaw = nullptr;
			row.mFreeRaws = nullptr;
		}

		DataRow(const DataRow&) = delete;

		~DataRow() MOMO_NOEXCEPT
		{
			if (mRaw == nullptr)
				return;
			mColumnList->DestroyRaw(mRaw);
			while (true)
			{
				Raw* headRaw = *mFreeRaws;
				*reinterpret_cast<Raw**>(mRaw) = headRaw;
				if (mFreeRaws->compare_exchange_weak(headRaw, mRaw))
					break;
			}
		}

		DataRow& operator=(DataRow&& row) MOMO_NOEXCEPT
		{
			DataRow(std::move(row)).Swap(*this);
			return *this;
		}

		DataRow& operator=(const DataRow&) = delete;

		void Swap(DataRow& row) MOMO_NOEXCEPT
		{
			std::swap(mColumnList, row.mColumnList);
			std::swap(mRaw, row.mRaw);
			std::swap(mFreeRaws, row.mFreeRaws);
		}

		MOMO_FRIEND_SWAP(DataRow)

		const ColumnList& GetColumnList() const MOMO_NOEXCEPT
		{
			return *mColumnList;
		}

		template<typename Item>
		const Item& GetByOffset(size_t offset) const
		{
			return mColumnList->template GetByOffset<const Item>(mRaw, offset);
		}

		template<typename Item>
		Item& GetByOffset(size_t offset)
		{
			return mColumnList->template GetByOffset<Item>(mRaw, offset);
		}

		template<typename Item>
		const Item& GetByColumn(const Column<Item>& column) const
		{
			return GetByOffset<Item>(mColumnList->GetOffset(column));
		}

		template<typename Item>
		Item& GetByColumn(const Column<Item>& column)
		{
			return GetByOffset<Item>(mColumnList->GetOffset(column));
		}

		template<typename Item>
		const Item& operator[](const Column<Item>& column) const
		{
			return GetByColumn(column);
		}

		template<typename Item>
		Item& operator[](const Column<Item>& column)
		{
			return GetByColumn(column);
		}

		const Raw* GetRaw() const MOMO_NOEXCEPT
		{
			return mRaw;
		}

		Raw* GetRaw() MOMO_NOEXCEPT
		{
			return mRaw;
		}

		Raw* ExtractRaw() MOMO_NOEXCEPT
		{
			Raw* raw = mRaw;
			mRaw = nullptr;
			return raw;
		}

		const Raw* operator->() const MOMO_NOEXCEPT
		{
			return mRaw;
		}

		Raw* operator->() MOMO_NOEXCEPT
		{
			return mRaw;
		}

	protected:
		DataRow(const ColumnList* columnList, Raw* raw, FreeRaws* freeRaws) MOMO_NOEXCEPT
			: mColumnList(columnList),
			mRaw(raw),
			mFreeRaws(freeRaws)
		{
		}

		Raw* ptGetRaw() const MOMO_NOEXCEPT
		{
			return mRaw;
		}

	private:
		const ColumnList* mColumnList;
		Raw* mRaw;
		FreeRaws* mFreeRaws;
	};

	template<typename TColumnList>
	class DataConstRowReference
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		template<typename Item>
		using Column = typename ColumnList::template Column<Item>;

		typedef DataConstRowReference ConstReference;

	public:
		DataConstRowReference() = delete;

		const ColumnList& GetColumnList() const MOMO_NOEXCEPT
		{
			return *mColumnList;
		}

		template<typename Item>
		const Item& GetByOffset(size_t offset) const
		{
			return mColumnList->template GetByOffset<const Item>(mRaw, offset);
		}

		template<typename Item>
		const Item& GetByColumn(const Column<Item>& column) const
		{
			return GetByOffset<Item>(mColumnList->GetOffset(column));
		}

		template<typename Item>
		const Item& operator[](const Column<Item>& column) const
		{
			return GetByOffset<Item>(mColumnList->GetOffset(column));
		}

		size_t GetNumber() const MOMO_NOEXCEPT
		{
			return mColumnList->GetNumber(mRaw);
		}

		const Raw* GetRaw() const MOMO_NOEXCEPT
		{
			return mRaw;
		}

		const Raw* operator->() const MOMO_NOEXCEPT
		{
			return mRaw;
		}

	protected:
		DataConstRowReference(const ColumnList* columnList, Raw* raw) MOMO_NOEXCEPT
			: mColumnList(columnList),
			mRaw(raw)
		{
		}

		Raw* ptGetRaw() const MOMO_NOEXCEPT
		{
			return mRaw;
		}

	private:
		const ColumnList* mColumnList;
		Raw* mRaw;
	};

	template<typename TColumnList>
	class DataRowReference
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		template<typename Item>
		using Column = typename ColumnList::template Column<Item>;

		typedef DataConstRowReference<ColumnList> ConstReference;

		template<typename Item>
		class ItemReference
		{
		public:
			ItemReference(const ColumnList* columnList, Raw* raw, size_t offset) MOMO_NOEXCEPT	//pt
				: mColumnList(columnList),
				mRaw(raw),
				mOffset(offset)
			{
			}

			ItemReference(ItemReference&& itemRef) MOMO_NOEXCEPT
				: mColumnList(itemRef.mColumnList),
				mRaw(itemRef.mRaw),
				mOffset(itemRef.mOffset)
			{
			}

			ItemReference(const ItemReference&) = delete;

			~ItemReference() MOMO_NOEXCEPT
			{
			}

			ItemReference& operator=(const ItemReference& itemRef)
			{
				return pvAssign(itemRef.Get());
			}

			template<typename ItemArg>
			ItemReference& operator=(ItemArg&& itemArg)
			{
				return pvAssign(std::forward<ItemArg>(itemArg));
			}

			operator const Item&() const
			{
				return Get();
			}

			const Item& Get() const
			{
				return mColumnList->template GetByOffset<const Item>(mRaw, mOffset);
			}

		private:
			template<typename ItemArg>
			ItemReference& pvAssign(ItemArg&& itemArg)
			{
				MOMO_CHECK(mColumnList->IsMutable(mOffset));
				mColumnList->template Assign<Item>(mRaw, mOffset, std::forward<ItemArg>(itemArg));
				return *this;
			}

		private:
			const ColumnList* mColumnList;
			Raw* mRaw;
			size_t mOffset;
		};

	private:
		struct ConstReferenceProxy : public ConstReference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstReference)
		};

	public:
		DataRowReference() = delete;

		operator ConstReference() const MOMO_NOEXCEPT
		{
			return ConstReferenceProxy(mColumnList, mRaw);
		}

		const ColumnList& GetColumnList() const MOMO_NOEXCEPT
		{
			return *mColumnList;
		}

		template<typename Item>
		ItemReference<Item> GetByOffset(size_t offset) const
		{
			return ItemReference<Item>(mColumnList, mRaw, offset);
		}

		template<typename Item>
		ItemReference<Item> GetByColumn(const Column<Item>& column) const
		{
			return GetByOffset<Item>(mColumnList->GetOffset(column));
		}

		template<typename Item>
		ItemReference<Item> operator[](const Column<Item>& column) const
		{
			return GetByOffset<Item>(mColumnList->GetOffset(column));
		}

		size_t GetNumber() const MOMO_NOEXCEPT
		{
			return mColumnList->GetNumber(mRaw);
		}

		const Raw* GetRaw() const MOMO_NOEXCEPT
		{
			return mRaw;
		}

		const Raw* operator->() const MOMO_NOEXCEPT
		{
			return mRaw;
		}

	protected:
		DataRowReference(const ColumnList* columnList, Raw* raw) MOMO_NOEXCEPT
			: mColumnList(columnList),
			mRaw(raw)
		{
		}

		Raw* ptGetRaw() const MOMO_NOEXCEPT
		{
			return mRaw;
		}

	private:
		const ColumnList* mColumnList;
		Raw* mRaw;
	};
}

} // namespace experimental

} // namespace momo
