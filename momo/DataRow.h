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

		template<typename Type>
		using Column = typename ColumnList::template Column<Type>;

	public:
		DataRow(const ColumnList* columnList, Raw* raw, std::atomic<Raw*>* freeRaws) noexcept
			: mColumnList(columnList),
			mRaw(raw),
			mFreeRaws(freeRaws)
		{
		}

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
				*reinterpret_cast<Raw**>(mRaw) = headRaw;
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

		template<typename Type>
		const Type& GetByOffset(size_t offset) const
		{
			return mColumnList->template GetByOffset<Type>(
				static_cast<const Raw*>(mRaw), offset);
		}

		template<typename Type>
		Type& GetByOffset(size_t offset)
		{
			return mColumnList->template GetByOffset<Type>(mRaw, offset);
		}

		template<typename Type>
		const Type& GetByColumn(const Column<Type>& column) const
		{
			return GetByOffset<Type>(mColumnList->GetOffset(column));
		}

		template<typename Type>
		Type& GetByColumn(const Column<Type>& column)
		{
			return GetByOffset<Type>(mColumnList->GetOffset(column));
		}

		template<typename Type>
		const Type& operator[](const Column<Type>& column) const
		{
			return GetByColumn(column);
		}

		template<typename Type>
		Type& operator[](const Column<Type>& column)
		{
			return GetByColumn(column);
		}

		const Raw* GetRaw() const noexcept
		{
			return mRaw;
		}

		Raw* GetRaw() noexcept
		{
			return mRaw;
		}

		Raw* ExtractRaw() noexcept
		{
			Raw* raw = mRaw;
			mRaw = nullptr;
			return raw;
		}

		const Raw* operator->() const noexcept
		{
			return mRaw;
		}

		Raw* operator->() noexcept
		{
			return mRaw;
		}

	private:
		const ColumnList* mColumnList;
		Raw* mRaw;
		std::atomic<Raw*>* mFreeRaws;
	};

	template<typename TColumnList>
	class DataConstRowRef
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Raw Raw;

		template<typename Type>
		using Column = typename ColumnList::template Column<Type>;

		typedef DataConstRowRef ConstRowRef;

	public:
		DataConstRowRef(const ColumnList* columnList, const Raw* raw) noexcept
			: mColumnList(columnList),
			mRaw(raw)
		{
		}

		const ColumnList& GetColumnList() const noexcept
		{
			return *mColumnList;
		}

		template<typename Type>
		const Type& GetByOffset(size_t offset) const
		{
			return mColumnList->template GetByOffset<Type>(mRaw, offset);
		}

		template<typename Type>
		const Type& GetByColumn(const Column<Type>& column) const
		{
			return GetByOffset<Type>(mColumnList->GetOffset(column));
		}

		template<typename Type>
		const Type& operator[](const Column<Type>& column) const
		{
			return GetByOffset<Type>(mColumnList->GetOffset(column));
		}

		const Raw* GetRaw() const noexcept
		{
			return mRaw;
		}

		const Raw* operator->() const noexcept
		{
			return mRaw;
		}

	private:
		const ColumnList* mColumnList;
		const Raw* mRaw;
	};

	template<typename TColumnList>
	class DataRowRef
	{
	public:
		typedef TColumnList ColumnList;
		typedef typename ColumnList::Raw Raw;

		template<typename Type>
		using Column = typename ColumnList::template Column<Type>;

		typedef DataConstRowRef<ColumnList> ConstRowRef;

		template<typename Type>
		class ItemRef
		{
		public:
			ItemRef(const ColumnList* columnList, Raw* raw, size_t offset) noexcept
				: mColumnList(columnList),
				mRaw(raw),
				mOffset(offset)
			{
			}

			ItemRef(ItemRef&& itemRef) noexcept
				: mColumnList(itemRef.mColumnList),
				mRaw(itemRef.mRaw),
				mOffset(itemRef.mOffset)
			{
			}

			ItemRef(const ItemRef&) = delete;

			~ItemRef() noexcept
			{
			}

			ItemRef& operator=(const ItemRef& itemRef)
			{
				return _Assign(itemRef.Get());
			}

			template<typename TypeArg>
			ItemRef& operator=(TypeArg&& itemArg)
			{
				return _Assign(std::forward<TypeArg>(itemArg));
			}

			operator const Type&() const
			{
				return Get();
			}

			const Type& Get() const
			{
				return mColumnList->template GetByOffset<Type>(
					static_cast<const Raw*>(mRaw), mOffset);
			}

		private:
			template<typename TypeArg>
			ItemRef& _Assign(TypeArg&& itemArg)
			{
				if (!mColumnList->IsMutable(mOffset))
					throw std::runtime_error("Item is read only");
				mColumnList->template Assign<Type>(mRaw, mOffset, std::forward<TypeArg>(itemArg));
				return *this;
			}

		private:
			const ColumnList* mColumnList;
			Raw* mRaw;
			size_t mOffset;
		};

	public:
		DataRowRef(const ColumnList* columnList, Raw* raw) noexcept
			: mColumnList(columnList),
			mRaw(raw)
		{
		}

		operator ConstRowRef() const noexcept
		{
			return ConstRowRew(mColumnList, mRaw);
		}

		const ColumnList& GetColumnList() const noexcept
		{
			return *mColumnList;
		}

		template<typename Type>
		ItemRef<Type> GetByOffset(size_t offset) const
		{
			return ItemRef<Type>(mColumnList, mRaw, offset);
		}

		template<typename Type>
		ItemRef<Type> GetByColumn(const Column<Type>& column) const
		{
			return GetByOffset<Type>(mColumnList->GetOffset(column));
		}

		template<typename Type>
		ItemRef<Type> operator[](const Column<Type>& column) const
		{
			return GetByOffset<Type>(mColumnList->GetOffset(column));
		}

		const Raw* GetRaw() const noexcept
		{
			return mRaw;
		}

		const Raw* operator->() const noexcept
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
