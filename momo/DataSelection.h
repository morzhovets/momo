/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/DataSelection.h

\**********************************************************/

#pragma once

#include "IteratorUtility.h"
#include "Array.h"

namespace momo
{

namespace experimental
{

namespace internal
{
	template<typename TRowReference, typename TRawIterator>
	class DataRowIterator
	{
	public:
		typedef TRowReference RowReference;
		typedef TRawIterator RawIterator;
		typedef typename RowReference::ColumnList ColumnList;

		typedef const RowReference Reference;
		typedef momo::internal::IteratorPointer<Reference> Pointer;

		typedef DataRowIterator<typename RowReference::ConstReference,
			typename RawIterator::ConstIterator> ConstIterator;

	public:
		DataRowIterator() MOMO_NOEXCEPT
			: mColumnList(nullptr)
		{
		}

		DataRowIterator(const ColumnList* columnList, RawIterator rawIter) MOMO_NOEXCEPT
			: mColumnList(columnList),
			mRawIterator(rawIter)
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIterator(mColumnList, mRawIterator);
		}

		DataRowIterator& operator++()
		{
			++mRawIterator;
			return *this;
		}

		Reference operator*() const
		{
			return RowReference(mColumnList, *mRawIterator);
		}

		Pointer operator->() const
		{
			return Pointer(**this);
		}

		bool operator==(DataRowIterator iter) const MOMO_NOEXCEPT
		{
			return mRawIterator == iter.mRawIterator;
		}

		bool operator!=(DataRowIterator iter) const MOMO_NOEXCEPT
		{
			return !(*this == iter);
		}

	private:
		const ColumnList* mColumnList;
		RawIterator mRawIterator;
	};

	template<typename TRowReference, typename TMemManager>
	class DataSelection
	{
	public:
		typedef TRowReference RowReference;
		typedef TMemManager MemManager;
		typedef typename RowReference::ColumnList ColumnList;
		typedef typename ColumnList::Raw Raw;

		template<typename Type>
		using Column = typename ColumnList::template Column<Type>;

		typedef typename RowReference::ConstReference ConstRowReference;
		typedef DataSelection<ConstRowReference, MemManager> ConstSelection;

		typedef Array<Raw*, MemManager> Raws;

		typedef DataRowIterator<RowReference, typename Raws::ConstIterator> ConstIterator;
		typedef ConstIterator Iterator;

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

		ConstIterator GetBegin() const MOMO_NOEXCEPT
		{
			return ConstIterator(mColumnList, mRaws.GetBegin());
		}

		ConstIterator GetEnd() const MOMO_NOEXCEPT
		{
			return ConstIterator(mColumnList, mRaws.GetEnd());
		}

		MOMO_FRIEND_SWAP(DataSelection)
		MOMO_FRIENDS_BEGIN_END(const DataSelection&, ConstIterator)

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

		const RowReference operator[](size_t index) const
		{
			return RowReference(mColumnList, mRaws[index]);
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
				return rowComparer(ConstRowReference(mColumnList, raw1),
					ConstRowReference(mColumnList, raw2));
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

namespace std
{
	template<typename RR, typename RI>
	struct iterator_traits<momo::experimental::internal::DataRowIterator<RR, RI>>
	{
		typedef forward_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename momo::experimental::internal::DataRowIterator<RR, RI>::Pointer pointer;
		typedef typename momo::experimental::internal::DataRowIterator<RR, RI>::Reference reference;
		typedef typename momo::experimental::internal::DataRowIterator<RR, RI>::RowReference value_type;
	};
} // namespace std
