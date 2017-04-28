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
		typedef typename ColumnList::Settings Settings;

		typedef const RowReference Reference;
		typedef momo::internal::IteratorPointer<Reference> Pointer;

		typedef DataRowIterator<typename RowReference::ConstReference, RawIterator> ConstIterator;

	private:
		struct RowReferenceProxy : public RowReference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(RowReference)
		};

		struct ConstIteratorProxy : public ConstIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
			MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetColumnList, const ColumnList*)
			MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetRawIterator, RawIterator)
		};

	public:
		DataRowIterator() MOMO_NOEXCEPT
			: mColumnList(nullptr)
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIteratorProxy(mColumnList, mRawIterator);
		}

		DataRowIterator& operator+=(ptrdiff_t diff)
		{
			mRawIterator += diff;
			return *this;
		}

		ptrdiff_t operator-(ConstIterator iter) const
		{
			MOMO_CHECK(mColumnList == ConstIteratorProxy::GetColumnList(iter));
			return mRawIterator - ConstIteratorProxy::GetRawIterator(iter);
		}

		Pointer operator->() const
		{
			MOMO_CHECK(mColumnList != nullptr);
			return Pointer(RowReferenceProxy(mColumnList, *mRawIterator));
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mColumnList == ConstIteratorProxy::GetColumnList(iter)
				&& mRawIterator == ConstIteratorProxy::GetRawIterator(iter);
		}

		bool operator<(ConstIterator iter) const
		{
			MOMO_CHECK(mColumnList == ConstIteratorProxy::GetColumnList(iter));
			return mRawIterator < ConstIteratorProxy::GetRawIterator(iter);
		}

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(DataRowIterator)

	protected:
		DataRowIterator(const ColumnList* columnList, RawIterator rawIter) MOMO_NOEXCEPT
			: mColumnList(columnList),
			mRawIterator(rawIter)
		{
		}

		const ColumnList* ptGetColumnList() const MOMO_NOEXCEPT
		{
			return mColumnList;
		}

		RawIterator ptGetRawIterator() const MOMO_NOEXCEPT
		{
			return mRawIterator;
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
	
	private:
		struct IteratorProxy : public Iterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
		};

		struct ConstBoundsProxy : public ConstBounds
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstBounds)
		};

	public:
		DataRowBounds() MOMO_NOEXCEPT
			: mColumnList(nullptr)
		{
		}

		operator ConstBounds() const MOMO_NOEXCEPT
		{
			return ConstBoundsProxy(mColumnList, mRawBounds);
		}

		Iterator GetBegin() const MOMO_NOEXCEPT
		{
			return IteratorProxy(mColumnList, mRawBounds.GetBegin());
		}

		Iterator GetEnd() const MOMO_NOEXCEPT
		{
			return IteratorProxy(mColumnList, mRawBounds.GetEnd());
		}

		MOMO_FRIENDS_BEGIN_END(const DataRowBounds&, Iterator)

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mRawBounds.GetCount();
		}

		RowReference& operator[](size_t index) const
		{
			return GetBegin()[index];
		}

	protected:
		DataRowBounds(const ColumnList* columnList, RawBounds rawBounds) MOMO_NOEXCEPT
			: mColumnList(columnList),
			mRawBounds(rawBounds)
		{
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

		struct ConstIteratorProxy : public ConstIterator
		{
			typedef typename DataSelection::ConstIterator ConstIterator;	//? vs
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
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
			return ConstIteratorProxy(mColumnList, mRaws.GetBegin());
		}

		ConstIterator GetEnd() const MOMO_NOEXCEPT
		{
			return ConstIteratorProxy(mColumnList, mRaws.GetEnd());
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

		template<typename RowIterator>
		void Assign(RowIterator begin, RowIterator end)
		{
			size_t count = pvCheck(begin, end);
			mRaws.Reserve(count);
			mRaws.Clear(false);
			for (RowIterator iter = begin; iter != end; ++iter)
				mRaws.AddBackNogrow(RowReferenceProxy::GetRaw(*iter));
		}

		void Add(RowReference rowRef)
		{
			MOMO_CHECK(mColumnList == &rowRef.GetColumnList());
			mRaws.AddBack(RowReferenceProxy::GetRaw(rowRef));
		}

		template<typename RowIterator>
		void Add(RowIterator begin, RowIterator end)
		{
			size_t count = pvCheck(begin, end);
			mRaws.Reserve(mRaws.GetCount() + count);
			for (RowIterator iter = begin; iter != end; ++iter)
				mRaws.AddBackNogrow(RowReferenceProxy::GetRaw(*iter));
		}

		void Insert(size_t index, RowReference rowRef)
		{
			MOMO_CHECK(mColumnList == &rowRef.GetColumnList());
			mRaws.Insert(index, RowReferenceProxy::GetRaw(rowRef));
		}

		template<typename RowIterator>
		void Insert(size_t index, RowIterator begin, RowIterator end)
		{
			size_t count = pvCheck(begin, end);
			mRaws.Reserve(mRaws.GetCount() + count);
			mRaws.Insert(index, count, nullptr);
			for (RowIterator iter = begin; iter != end; ++iter, ++index)
				mRaws[index] = RowReferenceProxy::GetRaw(*iter);
		}

		void Remove(size_t index, size_t count = 1)
		{
			mRaws.Remove(index, count);
		}

		void Update(size_t index, RowReference rowRef)
		{
			MOMO_CHECK(mColumnList == &rowRef.GetColumnList());
			mRaws[index] = RowReferenceProxy::GetRaw(rowRef);
		}

		template<typename Filter>
		void Erase(const Filter& filter)	//?
		{
			size_t index = 0;
			for (Raw* raw : mRaws)
			{
				if (!filter(pvMakeRowReference(raw)))
					mRaws[index++] = raw;
			}
			mRaws.RemoveBack(mRaws.GetCount() - index);
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

		template<typename RowIterator>
		size_t pvCheck(RowIterator begin, RowIterator end)
		{
			MOMO_STATIC_ASSERT(momo::internal::IsForwardIterator<RowIterator>::value);
			size_t count = 0;
			for (RowIterator iter = begin; iter != end; ++iter, ++count)
				MOMO_CHECK(&iter->GetColumnList() == mColumnList);
			return count;
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
		typedef random_access_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename momo::experimental::internal::DataRowIterator<RR, RI>::Pointer pointer;
		typedef typename momo::experimental::internal::DataRowIterator<RR, RI>::Reference reference;
		typedef typename momo::experimental::internal::DataRowIterator<RR, RI>::RowReference value_type;
	};
} // namespace std
