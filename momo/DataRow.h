/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataRow.h

\**********************************************************/

#pragma once

#include "Utility.h"

#include <atomic>

namespace momo
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

		DataRow(DataRow&& row) noexcept
			: mColumnList(row.mColumnList),
			mRaw(row.mRaw),
			mFreeRaws(row.mFreeRaws)
		{
			row.mRaw = nullptr;
			row.mFreeRaws = nullptr;
		}

		DataRow(const DataRow&) = delete;

		~DataRow() noexcept
		{
			if (mRaw == nullptr)
				return;
			mColumnList->DestroyRaw(mRaw);
			while (true)
			{
				Raw* headRaw = *mFreeRaws;
				*BitCaster::PtrToPtr<Raw*>(mRaw, 0) = headRaw;	//?
				if (mFreeRaws->compare_exchange_weak(headRaw, mRaw))
					break;
			}
		}

		DataRow& operator=(DataRow&& row) noexcept
		{
			DataRow(std::move(row)).Swap(*this);
			return *this;
		}

		DataRow& operator=(const DataRow&) = delete;

		void Swap(DataRow& row) noexcept
		{
			std::swap(mColumnList, row.mColumnList);
			std::swap(mRaw, row.mRaw);
			std::swap(mFreeRaws, row.mFreeRaws);
		}

		MOMO_FRIEND_SWAP(DataRow)

		const ColumnList& GetColumnList() const noexcept
		{
			return *mColumnList;
		}

		template<typename Item>
		const Item& GetByOffset(size_t offset) const
		{
			return ColumnList::template GetByOffset<const Item>(mRaw, offset);
		}

		template<typename Item>
		Item& GetByOffset(size_t offset)
		{
			return ColumnList::template GetByOffset<Item>(mRaw, offset);
		}

		template<typename Item>
		const Item& Get(const Column<Item>& column) const
		{
			return GetByOffset<Item>(mColumnList->GetOffset(column));
		}

		template<typename Item>
		Item& Get(const Column<Item>& column)
		{
			return GetByOffset<Item>(mColumnList->GetOffset(column));
		}

		template<typename Item>
		const Item& operator[](const Column<Item>& column) const
		{
			return Get(column);
		}

		template<typename Item>
		Item& operator[](const Column<Item>& column)
		{
			return Get(column);
		}

		const Raw* GetRaw() const noexcept
		{
			return mRaw;
		}

		Raw* GetRaw() noexcept
		{
			return mRaw;
		}

		const Raw* operator->() const noexcept
		{
			return GetRaw();
		}

		Raw* operator->() noexcept
		{
			return GetRaw();
		}

	protected:
		explicit DataRow(const ColumnList* columnList, Raw* raw, FreeRaws* freeRaws) noexcept
			: mColumnList(columnList),
			mRaw(raw),
			mFreeRaws(freeRaws)
		{
		}

		Raw* ptGetRaw() const noexcept
		{
			return mRaw;
		}

		Raw* ptExtractRaw() noexcept
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
	class DataConstRowReference : private VersionKeeper<typename TColumnList::Settings>
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		template<typename Item>
		using Column = typename ColumnList::template Column<Item>;

		typedef DataConstRowReference ConstReference;

	protected:
		typedef internal::VersionKeeper<Settings> VersionKeeper;

	public:
		DataConstRowReference() = delete;

		const ColumnList& GetColumnList() const noexcept
		{
			return *mColumnList;
		}

		template<typename Item>
		const Item& GetByOffset(size_t offset) const
		{
			VersionKeeper::Check();
			return ColumnList::template GetByOffset<const Item>(mRaw, offset);
		}

		template<typename Item>
		const Item& Get(const Column<Item>& column) const
		{
			return GetByOffset<Item>(mColumnList->GetOffset(column));
		}

		template<typename Item>
		const Item& operator[](const Column<Item>& column) const
		{
			return Get(column);
		}

		size_t GetNumber() const
		{
			return mColumnList->GetNumber(GetRaw());
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
		explicit DataConstRowReference(const ColumnList* columnList, Raw* raw,
			VersionKeeper version) noexcept
			: VersionKeeper(version),
			mColumnList(columnList),
			mRaw(raw)
		{
		}

		Raw* ptGetRaw() const noexcept
		{
			return mRaw;
		}

	private:
		const ColumnList* mColumnList;
		Raw* mRaw;
	};

	template<typename TColumnList>
	class DataRowReference : public DataConstRowReference<TColumnList>
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		template<typename Item>
		using Column = typename ColumnList::template Column<Item>;

		typedef DataConstRowReference<ColumnList> ConstReference;

	public:
		using ConstReference::ConstReference;

		template<typename Item>
		Item& GetMutableByOffset(size_t offset) const
		{
			ConstReference::GetRaw();	// check
			MOMO_CHECK(ConstReference::GetColumnList().IsMutable(offset));
			return ColumnList::template GetByOffset<Item>(ConstReference::ptGetRaw(), offset);
		}

		template<typename Item>
		Item& GetMutable(const Column<Item>& column) const
		{
			return GetMutableByOffset<Item>(ConstReference::GetColumnList().GetOffset(column));
		}

		template<typename Item, typename ItemArg>
		void SetByOffset(size_t offset, ItemArg&& itemArg) const
		{
			ConstReference::GetRaw();	// check
			const ColumnList& columnList = ConstReference::GetColumnList();
			MOMO_CHECK(columnList.IsMutable(offset));
			columnList.template Assign<Item>(ConstReference::ptGetRaw(), offset,
				std::forward<ItemArg>(itemArg));
		}

		template<typename Item, typename ItemArg>
		void Set(const Column<Item>& column, ItemArg&& itemArg) const
		{
			SetByOffset<Item>(ConstReference::GetColumnList().GetOffset(column),
				std::forward<ItemArg>(itemArg));
		}
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

		explicit DataRowPointer(RowBounds rowBounds) noexcept
			: RowBounds(rowBounds)
		{
		}

		operator ConstPointer() const noexcept
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

		bool operator!() const noexcept
		{
			return RowBounds::GetCount() == 0;
		}

		explicit operator bool() const noexcept
		{
			return !!*this;
		}
	};
}

} // namespace momo
