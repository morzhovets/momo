/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/DataSelection.h

\**********************************************************/

#pragma once

#include "DataRow.h"
#include "Array.h"

namespace momo
{

namespace experimental
{

namespace internal
{
	template<typename TColumnList, typename TMemManager>
	class DataConstSelection
	{
	public:
		typedef TColumnList ColumnList;
		typedef TMemManager MemManager;
		typedef typename ColumnList::Raw Raw;

		template<typename Type>
		using Column = typename ColumnList::template Column<Type>;

		typedef DataConstRowRef<ColumnList> ConstRowRef;

	private:
		struct RawsSettings : public ArraySettings<2>
		{
			static const CheckMode checkMode = CheckMode::assertion;
		};

	public:
		typedef Array<Raw*, MemManager, ArrayItemTraits<Raw*>, RawsSettings> Raws;

	public:
		DataConstSelection(const ColumnList* columnList, Raws&& raws) noexcept
			: mColumnList(columnList),
			mRaws(std::move(raws))
		{
		}

		DataConstSelection(DataConstSelection&& selection) noexcept
			: mColumnList(selection.mColumnList),
			mRaws(std::move(selection.mRaws))
		{
		}

		DataConstSelection(const DataConstSelection& selection)
			: mColumnList(selection.mColumnList),
			mRaws(selection.mRaws)
		{
		}

		~DataConstSelection() noexcept
		{
		}

		DataConstSelection& operator=(DataConstSelection&& selection) noexcept
		{
			DataConstSelection(std::move(selection)).Swap(*this);
			return *this;
		}

		DataConstSelection& operator=(const DataConstSelection& selection)
		{
			if (this != &selection)
				DataConstSelection(selection).Swap(*this);
			return *this;
		}

		void Swap(DataConstSelection& selection) noexcept
		{
			std::swap(mColumnList, selection.mColumnList);
			mRaws.Swap(selection.mRaws);
		}

		MOMO_FRIEND_SWAP(DataConstSelection)

		const ColumnList& GetColumnList() const noexcept
		{
			return *mColumnList;
		}

		const MemManager& GetMemManager() const noexcept
		{
			return mRaws.GetMemManager();
		}

		MemManager& GetMemManager() noexcept
		{
			return mRaws.GetMemManager();
		}

		size_t GetCount() const noexcept
		{
			return mRaws.GetCount();
		}

		bool IsEmpty() const noexcept
		{
			return mRaws.IsEmpty();
		}

		void Clear() noexcept
		{
			mRaws.Clear();
		}

		const ConstRowRef operator[](size_t index) const
		{
			return ConstRowRef(mRaws[index], mColumnList);
		}

		DataConstSelection& Reverse() noexcept
		{
			std::reverse(mRaws.GetBegin(), mRaws.GetEnd());
			return *this;
		}

		template<typename Comparer>
		DataConstSelection& Sort(const Comparer& comparer)
		{
			auto rawComparer = [this, &comparer] (const Raw* raw1, const Raw* raw2)
			{
				return comparer(ConstRowRef(mColumnList, raw1),
					ConstRowRef(mColumnList, raw2));
			};
			std::sort(mRaws.GetBegin(), mRaws.GetEnd(), rawComparer);
			return *this;
		}

	private:
		const ColumnList* mColumnList;
		Raws mRaws;
	};
}

} // namespace experimental

} // namespace momo
