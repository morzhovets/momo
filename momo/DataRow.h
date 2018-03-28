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

		const Raw* operator->() const MOMO_NOEXCEPT
		{
			return GetRaw();
		}

		Raw* operator->() MOMO_NOEXCEPT
		{
			return GetRaw();
		}

	protected:
		explicit DataRow(const ColumnList* columnList, Raw* raw, FreeRaws* freeRaws) MOMO_NOEXCEPT
			: mColumnList(columnList),
			mRaw(raw),
			mFreeRaws(freeRaws)
		{
		}

		Raw* ptGetRaw() const MOMO_NOEXCEPT
		{
			return mRaw;
		}

		Raw* ptExtractRaw() MOMO_NOEXCEPT
		{
			Raw* raw = mRaw;
			mRaw = nullptr;
			return raw;
		}

	private:
		const ColumnList* mColumnList;
		Raw* mRaw;
		FreeRaws* mFreeRaws;
	};

	template<typename TColumnList>
	class DataConstItemReferencer
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Raw Raw;

		typedef DataConstItemReferencer ConstReferencer;

		template<typename Item>
		using ItemReference = const Item&;

	protected:
		typedef typename ColumnList::Settings Settings;

		typedef momo::internal::VersionKeeper<Settings> VersionKeeper;

	protected:
		template<typename Item>
		static ItemReference<Item> ptGetReference(const ColumnList* columnList, Raw* raw,
			size_t offset, VersionKeeper version)
		{
			version.Check();
			return columnList->template GetByOffset<const Item>(raw, offset);
		}
	};

	template<typename TColumnList>
	class DataItemReferencer
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Raw Raw;

		typedef DataConstItemReferencer<ColumnList> ConstReferencer;

	protected:
		typedef typename ColumnList::Settings Settings;

		typedef momo::internal::VersionKeeper<Settings> VersionKeeper;

	public:
		template<typename TItem>
		class ItemReference : private VersionKeeper
		{
		public:
			typedef TItem Item;

			typedef typename ConstReferencer::template ItemReference<Item> ConstReference;

		public:
			ItemReference() = delete;

			ItemReference(const ItemReference& itemRef) MOMO_NOEXCEPT
				: VersionKeeper(itemRef),
				mColumnList(itemRef.mColumnList),
				mRaw(itemRef.mRaw),
				mOffset(itemRef.mOffset)
			{
			}

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
				VersionKeeper::Check();
				return mColumnList->template GetByOffset<const Item>(mRaw, mOffset);
			}

		protected:
			explicit ItemReference(const ColumnList* columnList, Raw* raw, size_t offset,
				VersionKeeper version) MOMO_NOEXCEPT
				: VersionKeeper(version),
				mColumnList(columnList),
				mRaw(raw),
				mOffset(offset)
			{
			}

		private:
			template<typename ItemArg>
			ItemReference& pvAssign(ItemArg&& itemArg)
			{
				VersionKeeper::Check();
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
		template<typename Item>
		struct ItemReferenceProxy : public ItemReference<Item>	//?
		{
			typedef typename DataItemReferencer::template ItemReference<Item> ItemReference;	// gcc
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ItemReference)
		};

	protected:
		template<typename Item>
		static ItemReference<Item> ptGetReference(const ColumnList* columnList, Raw* raw,
			size_t offset, VersionKeeper version)
		{
			return ItemReferenceProxy<Item>(columnList, raw, offset, version);
		}
	};

	template<typename TItemReferencer>
	class DataRowReference : private TItemReferencer,
		private momo::internal::VersionKeeper<typename TItemReferencer::ColumnList::Settings>
	{
	protected:
		typedef TItemReferencer ItemReferencer;

	public:
		typedef typename ItemReferencer::ColumnList ColumnList;
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		template<typename Item>
		using Column = typename ColumnList::template Column<Item>;

		typedef DataRowReference<typename ItemReferencer::ConstReferencer> ConstReference;

		template<typename Item>
		using ItemReference = typename ItemReferencer::template ItemReference<Item>;

	protected:
		typedef momo::internal::VersionKeeper<Settings> VersionKeeper;

	private:
		struct ConstReferenceProxy : public ConstReference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstReference)
		};

	public:
		DataRowReference() = delete;

		operator ConstReference() const MOMO_NOEXCEPT
		{
			return ConstReferenceProxy(mColumnList, mRaw, *this);
		}

		const ColumnList& GetColumnList() const MOMO_NOEXCEPT
		{
			return *mColumnList;
		}

		template<typename Item>
		ItemReference<Item> GetByOffset(size_t offset) const
		{
			return ItemReferencer::template ptGetReference<Item>(mColumnList, mRaw, offset, *this);
		}

		template<typename Item>
		ItemReference<Item> GetByColumn(const Column<Item>& column) const
		{
			return GetByOffset<Item>(mColumnList->GetOffset(column));
		}

		template<typename Item>
		ItemReference<Item> operator[](const Column<Item>& column) const
		{
			return GetByColumn(column);
		}

		size_t GetNumber() const
		{
			VersionKeeper::Check();
			return mColumnList->GetNumber(mRaw);
		}

		const Raw* GetRaw() const
		{
			VersionKeeper::Check();
			return mRaw;
		}

		const Raw* operator->() const
		{
			return GetRaw();
		}

	protected:
		explicit DataRowReference(const ColumnList* columnList, Raw* raw,
			VersionKeeper version) MOMO_NOEXCEPT
			: VersionKeeper(version),
			mColumnList(columnList),
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

	template<typename TRowBounds>
	class DataRowPointer : public TRowBounds
	{
	protected:
		typedef TRowBounds RowBounds;

	public:
		typedef typename RowBounds::Reference Reference;

		typedef DataRowPointer<typename RowBounds::ConstBounds> ConstPointer;

	private:
		typedef typename Reference::ColumnList::Settings Settings;

	public:
		using RowBounds::RowBounds;

		explicit DataRowPointer(RowBounds rowBounds) MOMO_NOEXCEPT
			: RowBounds(rowBounds)
		{
		}

		operator ConstPointer() const MOMO_NOEXCEPT
		{
			return ConstPointer(static_cast<const RowBounds&>(*this));
		}

		typename RowBounds::Iterator operator->() const
		{
			MOMO_CHECK(RowBounds::GetCount() == 1);
			return RowBounds::GetBegin();
		}

		Reference operator*() const
		{
			MOMO_CHECK(RowBounds::GetCount() == 1);
			return *RowBounds::GetBegin();
		}

		bool operator!() const MOMO_NOEXCEPT
		{
			return RowBounds::GetCount() == 0;
		}

		explicit operator bool() const MOMO_NOEXCEPT
		{
			return !!*this;
		}
	};
}

} // namespace experimental

} // namespace momo
