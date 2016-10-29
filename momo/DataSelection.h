/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/DataSelection.h

\**********************************************************/

#pragma once

#include "Array.h"

namespace momo
{

namespace experimental
{

namespace internal
{
	template<typename TRowRef, typename TMemManager>
	class DataSelection
	{
	public:
		typedef TRowRef RowRef;
		typedef TMemManager MemManager;
		typedef typename RowRef::ColumnList ColumnList;
		typedef typename ColumnList::Raw Raw;

		template<typename Type>
		using Column = typename ColumnList::template Column<Type>;

		typedef typename RowRef::ConstRowRef ConstRowRef;
		typedef DataSelection<ConstRowRef, MemManager> ConstSelection;

		typedef Array<Raw*, MemManager> Raws;

	public:
		DataSelection(const ColumnList* columnList, Raws&& raws) MOMO_NOEXCEPT
			: mColumnList(columnList),
			mRaws(std::move(raws))
		{
		}

		DataSelection(DataSelection&& selection) MOMO_NOEXCEPT
			: mColumnList(selection.mColumnList),
			mRaws(std::move(selection.mRaws))
		{
		}

		DataSelection(const DataSelection& selection)
			: mColumnList(selection.mColumnList),
			mRaws(selection.mRaws)
		{
		}

		~DataSelection() MOMO_NOEXCEPT
		{
		}

		DataSelection& operator=(DataSelection&& selection) MOMO_NOEXCEPT
		{
			DataSelection(std::move(selection)).Swap(*this);
			return *this;
		}

		DataSelection& operator=(const DataSelection& selection)
		{
			if (this != &selection)
				DataSelection(selection).Swap(*this);
			return *this;
		}

		operator ConstSelection() && MOMO_NOEXCEPT
		{
			return ConstSelection(mColumnList, std::move(mRaws));
		}

		operator ConstSelection() const&
		{
			return ConstSelection(mColumnList, Raws(mRaws));
		}

		void Swap(DataSelection& selection) MOMO_NOEXCEPT
		{
			std::swap(mColumnList, selection.mColumnList);
			mRaws.Swap(selection.mRaws);
		}

		MOMO_FRIEND_SWAP(DataSelection)

		const ColumnList& GetColumnList() const MOMO_NOEXCEPT
		{
			return *mColumnList;
		}

		const MemManager& GetMemManager() const MOMO_NOEXCEPT
		{
			return mRaws.GetMemManager();
		}

		MemManager& GetMemManager() MOMO_NOEXCEPT
		{
			return mRaws.GetMemManager();
		}

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mRaws.GetCount();
		}

		bool IsEmpty() const MOMO_NOEXCEPT
		{
			return mRaws.IsEmpty();
		}

		void Clear() MOMO_NOEXCEPT
		{
			mRaws.Clear();
		}

		const ConstRowRef operator[](size_t index) const
		{
			return ConstRowRef(mRaws[index], mColumnList);
		}

		DataSelection&& Reverse() && MOMO_NOEXCEPT
		{
			pvReverse();
			return std::move(*this);
		}

		DataSelection& Reverse() & MOMO_NOEXCEPT
		{
			pvReverse();
			return *this;
		}

		template<typename RowComparer>
		DataSelection&& Sort(const RowComparer& rowComparer) &&
		{
			pvSort(rowComparer);
			return std::move(*this);
		}

		template<typename RowComparer>
		DataSelection& Sort(const RowComparer& rowComparer) &
		{
			pvSort(rowComparer);
			return *this;
		}

	private:
		void pvReverse() MOMO_NOEXCEPT
		{
			std::reverse(mRaws.GetBegin(), mRaws.GetEnd());
		}

		template<typename RowComparer>
		void pvSort(const RowComparer& rowComparer)
		{
			auto rawComparer = [this, &rowComparer] (const Raw* raw1, const Raw* raw2)
			{
				return rowComparer(ConstRowRef(mColumnList, raw1),
					ConstRowRef(mColumnList, raw2));
			};
			std::sort(mRaws.GetBegin(), mRaws.GetEnd(), rawComparer);
		}

	private:
		const ColumnList* mColumnList;
		Raws mRaws;
	};
}

} // namespace experimental

} // namespace momo
