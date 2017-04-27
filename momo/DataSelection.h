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

		typedef DataRowIterator<typename RowReference::ConstReference, RawIterator> ConstIterator;

	private:
		struct RowReferenceProxy : public RowReference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(RowReference)
		};

	public:
		DataRowIterator() MOMO_NOEXCEPT
			: mColumnList(nullptr)
		{
		}

		DataRowIterator(const ColumnList* columnList, RawIterator rawIter) MOMO_NOEXCEPT	//pt
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

		DataRowIterator operator++(int)
		{
			DataRowIterator tempIter = *this;
			++*this;
			return tempIter;
		}

		Reference operator*() const
		{
			return RowReferenceProxy(mColumnList, *mRawIterator);
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

	template<typename TRowReference, typename TRawBounds>
	class DataRowBounds
	{
	public:
		typedef TRowReference RowReference;
		typedef TRawBounds RawBounds;
		typedef typename RowReference::ColumnList ColumnList;

		typedef DataRowIterator<RowReference, typename RawBounds::Iterator> Iterator;

		typedef DataRowBounds<typename RowReference::ConstReference, RawBounds> ConstBounds;

	public:
		DataRowBounds() MOMO_NOEXCEPT
			: mColumnList(nullptr)
		{
		}

		DataRowBounds(const ColumnList* columnList, RawBounds rawBounds) MOMO_NOEXCEPT	//pt
			: mColumnList(columnList),
			mRawBounds(rawBounds)
		{
		}

		operator ConstBounds() const MOMO_NOEXCEPT
		{
			return ConstBounds(mColumnList, mRawBounds);
		}

		Iterator GetBegin() const MOMO_NOEXCEPT
		{
			return Iterator(mRawBounds.GetBegin());
		}

		Iterator GetEnd() const MOMO_NOEXCEPT
		{
			return Iterator(mRawBounds.GetEnd());
		}

		MOMO_FRIENDS_BEGIN_END(const DataRowBounds&, Iterator)

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mRawBounds.GetCount();
		}

	private:
		const ColumnList* mColumnList;
		RawBounds mRawBounds;
	};

	template<typename TDataSettings>
	struct DataSelectionRawsSettings : TDataSettings::RawsSettings
	{
	protected:
		typedef TDataSettings DataSettings;

	public:
		static const CheckMode checkMode = DataSettings::checkMode;	//?
	};

	template<typename TRowReference>
	class DataSelection
	{
	public:
		typedef TRowReference RowReference;
		typedef typename RowReference::ColumnList ColumnList;
		typedef typename ColumnList::MemManager MemManager;
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		template<typename Type>
		using Column = typename ColumnList::template Column<Type>;

		typedef typename RowReference::ConstReference ConstRowReference;
		typedef DataSelection<ConstRowReference> ConstSelection;

	protected:
		typedef Array<Raw*, MemManager, ArrayItemTraits<Raw*, MemManager>,
			DataSelectionRawsSettings<Settings>> Raws;

	public:
		typedef DataRowIterator<RowReference, typename Raws::ConstIterator> ConstIterator;
		typedef ConstIterator Iterator;

	private:
		struct RowReferenceProxy : public RowReference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(RowReference)
			MOMO_DECLARE_PROXY_FUNCTION(RowReference, GetRaw, Raw*)
		};

		struct ConstSelectionProxy : public ConstSelection
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstSelection)
		};

	public:
		DataSelection() = delete;

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

		template<typename Filter>
		DataSelection(const DataSelection& selection, const Filter& filter)
			: mColumnList(selection.mColumnList),
			mRaws(MemManager(selection.GetMemManager()))
		{
			for (Raw* raw : selection.mRaws)
			{
				if (filter(pvMakeRowReference(raw)))
					mRaws.AddBack(raw);
			}
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
			return ConstSelectionProxy(mColumnList, std::move(mRaws));
		}

		operator ConstSelection() const&
		{
			return ConstSelectionProxy(mColumnList, Raws(mRaws));
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

		void Reserve(size_t capacity)
		{
			mRaws.Reserve(capacity);
		}

		const RowReference operator[](size_t index) const
		{
			return pvMakeRowReference(mRaws[index]);
		}

		void Add(RowReference rowRef)
		{
			MOMO_CHECK(mColumnList == &rowRef.GetColumnList());
			mRaws.AddBack(RowReferenceProxy::GetRaw(rowRef));
		}

		void Insert(size_t index, RowReference rowRef)
		{
			MOMO_CHECK(mColumnList == &rowRef.GetColumnList());
			mRaws.Insert(index, RowReferenceProxy::GetRaw(rowRef));
		}

		void Remove(size_t index, size_t count)
		{
			mRaws.Remove(index, count);
		}

		void Update(size_t index, RowReference rowRef)
		{
			MOMO_CHECK(mColumnList == &rowRef.GetColumnList());
			mRaws[index] = RowReferenceProxy::GetRaw(rowRef);
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

		template<typename RowPredicate>
		size_t BinarySearch(const RowPredicate& rowPred) const
		{
			auto rawPred = [this, &rowPred] (Raw*, Raw* raw)
				{ return rowPred(pvMakeRowReference(raw)); };
			return std::upper_bound(mRaws.GetBegin(), mRaws.GetEnd(), nullptr, rawPred) - mRaws.GetBegin();
		}

	protected:
		DataSelection(const ColumnList* columnList, Raws&& raws) MOMO_NOEXCEPT
			: mColumnList(columnList),
			mRaws(std::move(raws))
		{
		}

	private:
		RowReference pvMakeRowReference(Raw* raw) const MOMO_NOEXCEPT
		{
			return RowReferenceProxy(mColumnList, raw);
		}

		void pvReverse() MOMO_NOEXCEPT
		{
			std::reverse(mRaws.GetBegin(), mRaws.GetEnd());
		}

		template<typename RowComparer>
		void pvSort(const RowComparer& rowComparer)
		{
			auto rawComparer = [this, &rowComparer] (Raw* raw1, Raw* raw2)
				{ return rowComparer(pvMakeRowReference(raw1), pvMakeRowReference(raw2)); };
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
