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
	class DataRowIterator : private momo::internal::VersionKeeper<typename TRowReference::Settings>
	{
	protected:
		typedef TRowReference RowReference;
		typedef TRawIterator RawIterator;
		typedef typename RowReference::ColumnList ColumnList;
		typedef typename ColumnList::Settings Settings;

		typedef momo::internal::VersionKeeper<Settings> VersionKeeper;

	public:
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
		explicit DataRowIterator() MOMO_NOEXCEPT
			: mColumnList(nullptr)
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIteratorProxy(mColumnList, mRawIterator, *this);
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
			return Pointer(RowReferenceProxy(mColumnList, *mRawIterator, *this));
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
		explicit DataRowIterator(const ColumnList* columnList, RawIterator rawIter,
			VersionKeeper version) MOMO_NOEXCEPT
			: VersionKeeper(version),
			mColumnList(columnList),
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
	class DataRowBounds : private momo::internal::VersionKeeper<typename TRowReference::Settings>
	{
	protected:
		typedef TRowReference RowReference;
		typedef TRawBounds RawBounds;
		typedef typename RowReference::ColumnList ColumnList;
		typedef typename ColumnList::Settings Settings;

		typedef momo::internal::VersionKeeper<Settings> VersionKeeper;

	public:
		typedef DataRowIterator<RowReference, typename RawBounds::Iterator> Iterator;

		typedef DataRowBounds<typename RowReference::ConstReference, RawBounds> ConstBounds;

		typedef RowReference Reference;

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
		explicit DataRowBounds() MOMO_NOEXCEPT
			: mColumnList(nullptr)
		{
		}

		operator ConstBounds() const MOMO_NOEXCEPT
		{
			return ConstBoundsProxy(mColumnList, mRawBounds, *this);
		}

		Iterator GetBegin() const MOMO_NOEXCEPT
		{
			return IteratorProxy(mColumnList, mRawBounds.GetBegin(), *this);
		}

		Iterator GetEnd() const MOMO_NOEXCEPT
		{
			return IteratorProxy(mColumnList, mRawBounds.GetEnd(), *this);
		}

		MOMO_FRIENDS_BEGIN_END(const DataRowBounds&, Iterator)

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mRawBounds.GetCount();
		}

		Reference operator[](size_t index) const
		{
			MOMO_CHECK(index < GetCount());
			return GetBegin()[index];
		}

	protected:
		explicit DataRowBounds(const ColumnList* columnList, RawBounds rawBounds,
			VersionKeeper version) MOMO_NOEXCEPT
			: VersionKeeper(version),
			mColumnList(columnList),
			mRawBounds(rawBounds)
		{
		}

	private:
		const ColumnList* mColumnList;
		RawBounds mRawBounds;
	};

	template<typename TItem, typename TRowIterator, typename TSettings>
	class DataItemIterator
	{
	public:
		typedef TItem Item;
		typedef TRowIterator RowIterator;

	protected:
		typedef TSettings Settings;

	public:
		typedef typename RowIterator::Reference::template ItemReference<Item> Reference;
		typedef momo::internal::IteratorPointer<Reference> Pointer;

		typedef DataItemIterator<Item, typename RowIterator::ConstIterator, Settings> ConstIterator;

	public:
		explicit DataItemIterator() MOMO_NOEXCEPT
			: mOffset(0)
		{
		}

		explicit DataItemIterator(size_t offset, RowIterator rowIter) MOMO_NOEXCEPT
			: mOffset(offset),
			mRowIterator(rowIter)
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIterator(mOffset, mRowIterator);
		}

		DataItemIterator& operator+=(ptrdiff_t diff)
		{
			mRowIterator += diff;
			return *this;
		}

		ptrdiff_t operator-(ConstIterator iter) const
		{
			MOMO_CHECK(mOffset == iter.GetOffset());
			return mRowIterator - iter.GetRowIterator();
		}

		Pointer operator->() const
		{
			return Pointer(mRowIterator->template GetByOffset<Item>(mOffset));
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mOffset == iter.GetOffset() && mRowIterator == iter.GetRowIterator();
		}

		bool operator<(ConstIterator iter) const
		{
			MOMO_CHECK(mOffset == iter.GetOffset());
			return mRowIterator < iter.GetRowIterator();
		}

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(DataItemIterator)

		size_t GetOffset() const MOMO_NOEXCEPT
		{
			return mOffset;
		}

		RowIterator GetRowIterator() const MOMO_NOEXCEPT
		{
			return mRowIterator;
		}

	private:
		size_t mOffset;
		RowIterator mRowIterator;
	};

	template<typename TItem, typename TRowBounds, typename TSettings>
	class DataItemBounds
	{
	public:
		typedef TItem Item;
		typedef TRowBounds RowBounds;

	protected:
		typedef TSettings Settings;

	public:
		typedef DataItemIterator<Item, typename RowBounds::Iterator, Settings> Iterator;

		typedef DataItemBounds<Item, typename RowBounds::ConstBounds, Settings> ConstBounds;

		typedef typename Iterator::Reference Reference;

	public:
		explicit DataItemBounds() MOMO_NOEXCEPT
			: mOffset(0)
		{
		}

		explicit DataItemBounds(size_t offset, RowBounds rowBounds) MOMO_NOEXCEPT
			: mOffset(offset),
			mRowBounds(rowBounds)
		{
		}

		operator ConstBounds() const MOMO_NOEXCEPT
		{
			return ConstBounds(mOffset, mRowBounds);
		}

		Iterator GetBegin() const MOMO_NOEXCEPT
		{
			return Iterator(mOffset, mRowBounds.GetBegin());
		}

		Iterator GetEnd() const MOMO_NOEXCEPT
		{
			return Iterator(mOffset, mRowBounds.GetEnd());
		}

		MOMO_FRIENDS_BEGIN_END(const DataItemBounds&, Iterator)

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mRowBounds.GetCount();
		}

		Reference operator[](size_t index) const
		{
			MOMO_CHECK(index < GetCount());
			return GetBegin()[index];
		}

	private:
		size_t mOffset;
		RowBounds mRowBounds;
	};

	template<typename TRowReference>
	class DataSelection
		: private momo::internal::VersionKeeper<typename TRowReference::ColumnList::Settings>
	{
	public:
		typedef TRowReference RowReference;
		typedef typename RowReference::ColumnList ColumnList;
		typedef typename ColumnList::MemManager MemManager;
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		template<typename Item>
		using Column = typename ColumnList::template Column<Item>;

		typedef DataSelection<typename RowReference::ConstReference> ConstSelection;

	protected:
		typedef momo::internal::VersionKeeper<Settings> VersionKeeper;

		typedef Array<Raw*, MemManager, ArrayItemTraits<Raw*, MemManager>,
			momo::internal::NestedArraySettings<typename Settings::SelectionRawsSettings>> Raws;

	private:
		//typedef typename Raws::ConstIterator RawIterator;
		typedef momo::internal::ArrayIndexIterator<const Raws, Raw* const, Settings> RawIterator;

		typedef momo::internal::ArrayBounds<RawIterator> RawBounds;
		typedef DataRowBounds<RowReference, RawBounds> RowBounds;

	public:
		typedef DataRowIterator<RowReference, RawIterator> ConstIterator;
		typedef ConstIterator Iterator;

		template<typename Item>
		using ItemBounds = DataItemBounds<Item, RowBounds, Settings>;

	private:
		struct RowReferenceProxy : public RowReference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(RowReference)
			MOMO_DECLARE_PROXY_FUNCTION(RowReference, GetRaw, Raw*)
		};

		struct RawIteratorProxy : public RawIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(RawIterator)
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

		struct RowBoundsProxy : public RowBounds
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(RowBounds)
		};

	public:
		DataSelection() = delete;

		DataSelection(DataSelection&& selection) MOMO_NOEXCEPT
			: VersionKeeper(selection),
			mColumnList(selection.mColumnList),
			mRaws(std::move(selection.mRaws))
		{
		}

		DataSelection(const DataSelection& selection)
			: VersionKeeper(selection),
			mColumnList(selection.mColumnList),
			mRaws(selection.mRaws)
		{
		}

		template<typename RowFilter>
		DataSelection(const DataSelection& selection, const RowFilter& rowFilter)
			: VersionKeeper(selection),
			mColumnList(selection.mColumnList),
			mRaws(MemManager(selection.GetMemManager()))
		{
			for (Raw* raw : selection.mRaws)
			{
				if (rowFilter(pvMakeRowReference(raw)))
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
			return ConstSelectionProxy(mColumnList, std::move(mRaws), *this);
		}

		operator ConstSelection() const&
		{
			return ConstSelectionProxy(mColumnList, Raws(mRaws), *this);
		}

		void Swap(DataSelection& selection) MOMO_NOEXCEPT
		{
			std::swap(static_cast<VersionKeeper&>(*this), static_cast<VersionKeeper&>(selection));
			std::swap(mColumnList, selection.mColumnList);
			mRaws.Swap(selection.mRaws);
		}

		ConstIterator GetBegin() const MOMO_NOEXCEPT
		{
			return ConstIteratorProxy(mColumnList, pvMakeRawIterator(0), *this);
		}

		ConstIterator GetEnd() const MOMO_NOEXCEPT
		{
			return ConstIteratorProxy(mColumnList, pvMakeRawIterator(GetCount()), *this);
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
			MOMO_CHECK(index < GetCount());
			return pvMakeRowReference(mRaws[index]);
		}

		void Set(size_t index, RowReference rowRef)
		{
			rowRef.GetRaw();	// check
			MOMO_CHECK(index < GetCount());
			MOMO_CHECK(mColumnList == &rowRef.GetColumnList());
			mRaws[index] = RowReferenceProxy::GetRaw(rowRef);
		}

		template<typename Item>
		ItemBounds<Item> GetColumnItems(const Column<Item>& column) const
		{
			size_t offset = mColumnList->GetOffset(column);
			RawBounds rawBounds(pvMakeRawIterator(0), GetCount());
			return ItemBounds<Item>(offset, RowBoundsProxy(mColumnList, rawBounds, *this));
		}

		template<typename RowIterator>
		void Assign(RowIterator begin, RowIterator end)
		{
			size_t count = pvGetCount(begin, end);
			mRaws.Reserve(count);
			mRaws.Clear(false);
			for (RowIterator iter = begin; iter != end; ++iter)
				mRaws.AddBackNogrow(RowReferenceProxy::GetRaw(*iter));
		}

		void Add(RowReference rowRef)
		{
			rowRef.GetRaw();	// check
			MOMO_CHECK(mColumnList == &rowRef.GetColumnList());
			mRaws.AddBack(RowReferenceProxy::GetRaw(rowRef));
		}

		template<typename RowIterator>
		void Add(RowIterator begin, RowIterator end)
		{
			size_t count = pvGetCount(begin, end);
			mRaws.Reserve(mRaws.GetCount() + count);
			for (RowIterator iter = begin; iter != end; ++iter)
				mRaws.AddBackNogrow(RowReferenceProxy::GetRaw(*iter));
		}

		void Insert(size_t index, RowReference rowRef)
		{
			rowRef.GetRaw();	// check
			MOMO_CHECK(index <= GetCount());
			MOMO_CHECK(mColumnList == &rowRef.GetColumnList());
			mRaws.Insert(index, RowReferenceProxy::GetRaw(rowRef));
		}

		template<typename RowIterator>
		void Insert(size_t index, RowIterator begin, RowIterator end)
		{
			MOMO_CHECK(index <= GetCount());
			size_t count = pvGetCount(begin, end);
			mRaws.Reserve(mRaws.GetCount() + count);
			mRaws.Insert(index, count, nullptr);
			for (RowIterator iter = begin; iter != end; ++iter, ++index)
				mRaws[index] = RowReferenceProxy::GetRaw(*iter);
		}

		void Remove(size_t index, size_t count)
		{
			MOMO_CHECK(index + count <= GetCount());
			mRaws.Remove(index, count);
		}

		template<typename RowFilter>
		void Remove(const RowFilter& rowFilter)
		{
			auto newRowFilter = [&rowFilter] (RowReference rowRef)
				{ return !rowFilter(rowRef); };
			Filter(newRowFilter);
		}

		template<typename RowFilter>
		void Filter(const RowFilter& rowFilter)	//?
		{
			size_t index = 0;
			for (Raw* raw : mRaws)
			{
				if (rowFilter(pvMakeRowReference(raw)))
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
				{ return rowPred(pvMakeRowReference(raw)); };	//?
			return std::upper_bound(mRaws.GetBegin(), mRaws.GetEnd(), nullptr, rawPred) - mRaws.GetBegin();
		}

	protected:
		explicit DataSelection(const ColumnList* columnList, Raws&& raws,
			VersionKeeper version) MOMO_NOEXCEPT
			: VersionKeeper(version),
			mColumnList(columnList),
			mRaws(std::move(raws))
		{
		}

	private:
		RowReference pvMakeRowReference(Raw* raw) const MOMO_NOEXCEPT
		{
			return RowReferenceProxy(mColumnList, raw, *this);
		}

		RawIterator pvMakeRawIterator(size_t index) const MOMO_NOEXCEPT
		{
			return RawIteratorProxy(&mRaws, index);
		}

		template<typename RowIterator>
		size_t pvGetCount(RowIterator begin, RowIterator end)
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
				{ return rowComparer(pvMakeRowReference(raw1), pvMakeRowReference(raw2)); };	//?
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

	template<typename I, typename RI, typename S>
	struct iterator_traits<momo::experimental::internal::DataItemIterator<I, RI, S>>
	{
		typedef random_access_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename momo::experimental::internal::DataItemIterator<I, RI, S>::Pointer pointer;
		typedef typename momo::experimental::internal::DataItemIterator<I, RI, S>::Reference reference;
		typedef typename momo::experimental::internal::DataItemIterator<I, RI, S>::Item value_type;
	};
} // namespace std
