/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataSelection.h

\**********************************************************/

#pragma once

#include "IteratorUtility.h"
#include "Array.h"

namespace momo
{

namespace internal
{
	template<typename TRaws, typename TSettings>
	class DataRawIterator : private ArrayIndexIterator<const TRaws, const typename TRaws::Item>
	{
	protected:
		typedef TRaws Raws;
		typedef TSettings Settings;

	private:
		typedef typename Raws::Item RawPtr;

		typedef internal::ArrayIndexIterator<const Raws, const RawPtr> ArrayIndexIterator;

	public:
		typedef const RawPtr& Reference;
		typedef const RawPtr* Pointer;

		typedef DataRawIterator ConstIterator;

	public:
		explicit DataRawIterator() noexcept
		{
		}

		explicit DataRawIterator(const Raws& raws, size_t index) noexcept
			: ArrayIndexIterator(&raws, index)
		{
		}

		//operator ConstIterator() const noexcept

		DataRawIterator& operator+=(ptrdiff_t diff)
		{
			const Raws* raws = pvGetRaws();
			size_t newIndex = pvGetIndex() + diff;
			(void)raws; (void)newIndex;
			MOMO_CHECK((raws != nullptr) ? newIndex <= raws->GetCount() : diff == 0);
			ArrayIndexIterator::operator+=(diff);
			return *this;
		}

		ptrdiff_t operator-(ConstIterator iter) const
		{
			MOMO_CHECK(pvGetRaws() == iter.pvGetRaws());
			return ArrayIndexIterator::operator-(iter);
		}

		Pointer operator->() const
		{
			const Raws* raws = pvGetRaws();
			size_t index = pvGetIndex();
			(void)raws; (void)index;
			MOMO_CHECK(raws != nullptr && index < raws->GetCount());
			return ArrayIndexIterator::operator->();
			//return raws->GetItems() + index;
		}

		bool operator==(ConstIterator iter) const noexcept
		{
			return ArrayIndexIterator::operator==(iter);
		}

		bool operator<(ConstIterator iter) const
		{
			MOMO_CHECK(pvGetRaws() == iter.pvGetRaws());
			return ArrayIndexIterator::operator<(iter);
		}

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(DataRawIterator)

	private:
		const Raws* pvGetRaws() const noexcept
		{
			return ArrayIndexIterator::ptGetArray();
		}

		size_t pvGetIndex() const noexcept
		{
			return ArrayIndexIterator::ptGetIndex();
		}
	};

	template<typename TRowReference, typename TRawIterator>
	class DataRowIterator : private VersionKeeper<typename TRowReference::Settings>
	{
	protected:
		typedef TRowReference RowReference;
		typedef TRawIterator RawIterator;
		typedef typename RowReference::ColumnList ColumnList;
		typedef typename ColumnList::Settings Settings;

		typedef internal::VersionKeeper<Settings> VersionKeeper;

	public:
		typedef const RowReference Reference;
		typedef IteratorPointer<Reference> Pointer;

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
		explicit DataRowIterator() noexcept
			: mColumnList(nullptr)
		{
		}

		operator ConstIterator() const noexcept
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

		bool operator==(ConstIterator iter) const noexcept
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
			VersionKeeper version) noexcept
			: VersionKeeper(version),
			mColumnList(columnList),
			mRawIterator(rawIter)
		{
		}

		const ColumnList* ptGetColumnList() const noexcept
		{
			return mColumnList;
		}

		RawIterator ptGetRawIterator() const noexcept
		{
			return mRawIterator;
		}

	private:
		const ColumnList* mColumnList;
		RawIterator mRawIterator;
	};

	template<typename TRowReference, typename TRawBounds>
	class DataRowBounds : private VersionKeeper<typename TRowReference::Settings>
	{
	protected:
		typedef TRowReference RowReference;
		typedef TRawBounds RawBounds;
		typedef typename RowReference::ColumnList ColumnList;
		typedef typename ColumnList::Settings Settings;

		typedef internal::VersionKeeper<Settings> VersionKeeper;

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
		explicit DataRowBounds() noexcept
			: mColumnList(nullptr)
		{
		}

		operator ConstBounds() const noexcept
		{
			return ConstBoundsProxy(mColumnList, mRawBounds, *this);
		}

		Iterator GetBegin() const noexcept
		{
			return IteratorProxy(mColumnList, mRawBounds.GetBegin(), *this);
		}

		Iterator GetEnd() const noexcept
		{
			return IteratorProxy(mColumnList, mRawBounds.GetEnd(), *this);
		}

		MOMO_FRIENDS_BEGIN_END(const DataRowBounds&, Iterator)

		size_t GetCount() const noexcept
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
			VersionKeeper version) noexcept
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
	class DataConstItemIterator
	{
	public:
		typedef TItem Item;
		typedef TRowIterator RowIterator;

	protected:
		typedef TSettings Settings;

	public:
		typedef const Item& Reference;
		typedef const Item* Pointer;

		typedef DataConstItemIterator ConstIterator;

	public:
		explicit DataConstItemIterator() noexcept
			: mOffset(0)
		{
		}

		explicit DataConstItemIterator(size_t offset, RowIterator rowIter) noexcept
			: mOffset(offset),
			mRowIterator(rowIter)
		{
		}

		operator ConstIterator() const noexcept
		{
			return ConstIterator(mOffset, mRowIterator);
		}

		DataConstItemIterator& operator+=(ptrdiff_t diff)
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
			return std::addressof(mRowIterator->template GetByOffset<Item>(mOffset));
		}

		bool operator==(ConstIterator iter) const noexcept
		{
			return mOffset == iter.GetOffset() && mRowIterator == iter.GetRowIterator();
		}

		bool operator<(ConstIterator iter) const
		{
			MOMO_CHECK(mOffset == iter.GetOffset());
			return mRowIterator < iter.GetRowIterator();
		}

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(DataConstItemIterator)

		size_t GetOffset() const noexcept
		{
			return mOffset;
		}

		RowIterator GetRowIterator() const noexcept
		{
			return mRowIterator;
		}

	private:
		size_t mOffset;
		RowIterator mRowIterator;
	};

	template<typename TItem, typename TRowBounds, typename TSettings>
	class DataConstItemBounds
	{
	public:
		typedef TItem Item;
		typedef TRowBounds RowBounds;

	protected:
		typedef TSettings Settings;

	public:
		typedef DataConstItemIterator<Item, typename RowBounds::Iterator, Settings> Iterator;

		typedef DataConstItemBounds ConstBounds;

		typedef typename Iterator::Reference Reference;

	public:
		explicit DataConstItemBounds() noexcept
			: mOffset(0)
		{
		}

		explicit DataConstItemBounds(size_t offset, RowBounds rowBounds) noexcept
			: mOffset(offset),
			mRowBounds(rowBounds)
		{
		}

		operator ConstBounds() const noexcept
		{
			return ConstBounds(mOffset, mRowBounds);
		}

		Iterator GetBegin() const noexcept
		{
			return Iterator(mOffset, mRowBounds.GetBegin());
		}

		Iterator GetEnd() const noexcept
		{
			return Iterator(mOffset, mRowBounds.GetEnd());
		}

		MOMO_FRIENDS_BEGIN_END(const DataConstItemBounds&, Iterator)

		size_t GetCount() const noexcept
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

	template<typename TRowReference, typename TDataTraits>
	class DataSelection : private VersionKeeper<typename TRowReference::ColumnList::Settings>
	{
	public:
		typedef TRowReference RowReference;
		typedef TDataTraits DataTraits;
		typedef typename RowReference::ColumnList ColumnList;
		typedef typename ColumnList::MemManager MemManager;
		typedef typename ColumnList::Settings Settings;
		typedef typename ColumnList::Raw Raw;

		template<typename Item>
		using Column = typename ColumnList::template Column<Item>;

		template<typename Item>
		using Equaler = DataOperator<DataOperatorType::equal, Column<Item>, const Item&>;	//?

	protected:
		typedef internal::VersionKeeper<Settings> VersionKeeper;

		typedef Array<Raw*, MemManager, ArrayItemTraits<Raw*, MemManager>,
			NestedArraySettings<typename Settings::SelectionRawsSettings>> Raws;

	private:
		typedef typename RowReference::ConstReference ConstRowReference;

		typedef DataRawIterator<Raws, Settings> RawIterator;

		typedef ArrayBounds<RawIterator> RawBounds;
		typedef DataRowBounds<ConstRowReference, RawBounds> ConstRowBounds;

	public:
		typedef DataSelection<ConstRowReference, DataTraits> ConstSelection;

		typedef DataRowIterator<RowReference, RawIterator> ConstIterator;
		typedef ConstIterator Iterator;

		template<typename Item>
		using ConstItemBounds = DataConstItemBounds<Item, ConstRowBounds, Settings>;

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

		struct ConstRowBoundsProxy : public ConstRowBounds
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstRowBounds)
		};

	public:
		DataSelection() = delete;

		DataSelection(DataSelection&& selection) noexcept
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
				if (rowFilter(pvMakeConstRowReference(raw)))
					mRaws.AddBack(raw);
			}
		}

		~DataSelection() noexcept
		{
		}

		DataSelection& operator=(DataSelection&& selection) noexcept
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

		operator ConstSelection() && noexcept
		{
			return ConstSelectionProxy(mColumnList, std::move(mRaws), *this);
		}

		operator ConstSelection() const&
		{
			return ConstSelectionProxy(mColumnList, Raws(mRaws), *this);
		}

		void Swap(DataSelection& selection) noexcept
		{
			std::swap(static_cast<VersionKeeper&>(*this), static_cast<VersionKeeper&>(selection));
			std::swap(mColumnList, selection.mColumnList);
			mRaws.Swap(selection.mRaws);
		}

		ConstIterator GetBegin() const noexcept
		{
			return ConstIteratorProxy(mColumnList, RawIterator(mRaws, 0), *this);
		}

		ConstIterator GetEnd() const noexcept
		{
			return ConstIteratorProxy(mColumnList, RawIterator(mRaws, GetCount()), *this);
		}

		MOMO_FRIEND_SWAP(DataSelection)
		MOMO_FRIENDS_BEGIN_END(const DataSelection&, ConstIterator)

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
		ConstItemBounds<Item> GetColumnItems(const Column<Item>& column) const
		{
			size_t offset = mColumnList->GetOffset(column);
			RawBounds rawBounds(RawIterator(mRaws, 0), GetCount());
			return ConstItemBounds<Item>(offset,
				ConstRowBoundsProxy(mColumnList, rawBounds, *this));
		}

		template<typename RowIterator>
		void Assign(RowIterator begin, RowIterator end)
		{
			size_t initCount = GetCount();
			Add(begin, end);	//?
			mRaws.Remove(0, initCount);
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
			size_t initCount = GetCount();
			size_t count = pvGetCount(begin, end);
			mRaws.Reserve(initCount + count);
			try
			{
				for (RowIterator iter = begin; iter != end; ++iter)
				{
					RowReference rowRef = *iter;
					mRaws.AddBackNogrow(RowReferenceProxy::GetRaw(rowRef));
				}
			}
			catch (...)
			{
				mRaws.SetCount(initCount);
				throw;
			}
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
			size_t initCount = GetCount();
			MOMO_CHECK(index <= initCount);
			Add(begin, end);
			std::rotate(mRaws.GetBegin() + index, mRaws.GetBegin() + initCount, mRaws.GetEnd());
		}

		void Remove(size_t index, size_t count = 1)
		{
			MOMO_CHECK(index + count <= GetCount());
			mRaws.Remove(index, count);
		}

		template<typename RowFilter,
			typename = decltype(std::declval<const RowFilter&>()(std::declval<ConstRowReference>()))>
		void Remove(const RowFilter& rowFilter)
		{
			auto newRowFilter = [&rowFilter] (ConstRowReference rowRef)
				{ return !rowFilter(rowRef); };
			Filter(newRowFilter);
		}

		template<typename RowFilter>
		void Filter(const RowFilter& rowFilter)
		{
			size_t index = 0;
			for (Raw*& raw : mRaws)
			{
				if (!rowFilter(pvMakeConstRowReference(raw)))
					continue;
				std::swap(mRaws[index], raw);
				++index;
			}
			mRaws.RemoveBack(mRaws.GetCount() - index);
		}

		DataSelection&& Reverse() && noexcept
		{
			pvReverse();
			return std::move(*this);
		}

		DataSelection& Reverse() & noexcept
		{
			pvReverse();
			return *this;
		}

		template<typename Item, typename... Items>
		DataSelection&& Sort(const Column<Item>& column, const Column<Items>&... columns) &&
		{
			pvSort(column, columns...);
			return std::move(*this);
		}

		template<typename Item, typename... Items>
		DataSelection& Sort(const Column<Item>& column, const Column<Items>&... columns) &
		{
			pvSort(column, columns...);
			return *this;
		}

		template<typename RowComparer,
			typename = decltype(std::declval<const RowComparer&>()(
				std::declval<ConstRowReference>(), std::declval<ConstRowReference>()))>
		DataSelection&& Sort(const RowComparer& rowComp) &&
		{
			pvSort(rowComp);
			return std::move(*this);
		}

		template<typename RowComparer,
			typename = decltype(std::declval<const RowComparer&>()(
				std::declval<ConstRowReference>(), std::declval<ConstRowReference>()))>
		DataSelection& Sort(const RowComparer& rowComp) &
		{
			pvSort(rowComp);
			return *this;
		}

		template<typename Item, typename... Items>
		size_t GetLowerBound(Equaler<Item> equaler, Equaler<Items>... equalers) const
		{
			return pvBinarySearch<-1>(equaler, equalers...);
		}

		template<typename Item, typename... Items>
		size_t GetUpperBound(Equaler<Item> equaler, Equaler<Items>... equalers) const
		{
			return pvBinarySearch<0>(equaler, equalers...);
		}

		template<typename RowPredicate>
		size_t BinarySearch(const RowPredicate& rowPred) const
		{
			auto rawPred = [this, &rowPred] (Raw*, Raw* raw)
				{ return rowPred(pvMakeConstRowReference(raw)); };
			return std::upper_bound(mRaws.GetBegin(), mRaws.GetEnd(), nullptr, rawPred) - mRaws.GetBegin();
		}

	protected:
		explicit DataSelection(const ColumnList* columnList, Raws&& raws,
			VersionKeeper version) noexcept
			: VersionKeeper(version),
			mColumnList(columnList),
			mRaws(std::move(raws))
		{
		}

	private:
		ConstRowReference pvMakeConstRowReference(Raw* raw) const noexcept
		{
			return pvMakeRowReference(raw);
		}

		RowReference pvMakeRowReference(Raw* raw) const noexcept
		{
			return RowReferenceProxy(mColumnList, raw, *this);
		}

		template<typename RowIterator>
		size_t pvGetCount(RowIterator begin, RowIterator end)
		{
			MOMO_STATIC_ASSERT(IsForwardIterator<RowIterator>::value);
			size_t count = 0;
			for (RowIterator iter = begin; iter != end; (void)++iter, ++count)
				MOMO_CHECK(&iter->GetColumnList() == mColumnList);
			return count;
		}

		void pvReverse() noexcept
		{
			std::reverse(mRaws.GetBegin(), mRaws.GetEnd());
		}

		template<typename... Items>
		void pvSort(const Column<Items>&... columns)
		{
			static const size_t columnCount = sizeof...(columns);
			std::array<size_t, columnCount> offsets = {{ mColumnList->GetOffset(columns)... }};
			auto rawComp = [&offsets] (Raw* raw1, Raw* raw2)
				{ return pvCompare<void, Items...>(raw1, raw2, offsets.data()) < 0; };
			DataTraits::Sort(mRaws.GetBegin(), mRaws.GetCount(), rawComp, GetMemManager());
		}

		template<typename Void, typename Item, typename... Items>
		static int pvCompare(Raw* raw1, Raw* raw2, const size_t* offsets)
		{
			const Item& item1 = ColumnList::template GetByOffset<const Item>(raw1, *offsets);
			const Item& item2 = ColumnList::template GetByOffset<const Item>(raw2, *offsets);
			int cmp = DataTraits::Compare(item1, item2);
			if (cmp != 0)
				return cmp;
			return pvCompare<void, Items...>(raw1, raw2, offsets + 1);
		}

		template<typename Void>
		static int pvCompare(Raw* /*raw1*/, Raw* /*raw2*/, const size_t* /*offsets*/) noexcept
		{
			return 0;
		}

		template<typename RowComparer,
			typename = decltype(std::declval<const RowComparer&>()(
				std::declval<ConstRowReference>(), std::declval<ConstRowReference>()))>
		void pvSort(const RowComparer& rowComp)
		{
			auto rawComp = [this, &rowComp] (Raw* raw1, Raw* raw2)
				{ return rowComp(pvMakeConstRowReference(raw1), pvMakeConstRowReference(raw2)); };
			DataTraits::Sort(mRaws.GetBegin(), mRaws.GetCount(), rawComp, GetMemManager());
		}

		template<int bound, typename... Items>
		size_t pvBinarySearch(const Equaler<Items>&... equalers) const
		{
			static const size_t columnCount = sizeof...(equalers);
			std::array<size_t, columnCount> offsets = {{ mColumnList->GetOffset(equalers.GetColumn())... }};
			auto rawPred = [&offsets, &equalers...] (Raw*, Raw* raw)
				{ return pvCompare<void>(raw, offsets.data(), equalers...) > bound; };
			return std::upper_bound(mRaws.GetBegin(), mRaws.GetEnd(), nullptr, rawPred) - mRaws.GetBegin();
		}

		template<typename Void, typename Item, typename... Items>
		static int pvCompare(Raw* raw, const size_t* offsets, const Equaler<Item>& equaler,
			const Equaler<Items>&... equalers)
		{
			const Item& item1 = ColumnList::template GetByOffset<const Item>(raw, *offsets);
			const Item& item2 = equaler.GetItemArg();
			int cmp = DataTraits::Compare(item1, item2);
			if (cmp != 0)
				return cmp;
			return pvCompare<void>(raw, offsets + 1, equalers...);
		}

		template<typename Void>
		static int pvCompare(Raw* /*raw*/, const size_t* /*offsets*/) noexcept
		{
			return 0;
		}

	private:
		const ColumnList* mColumnList;
		Raws mRaws;
	};
}

} // namespace momo

namespace std
{
	template<typename R, typename S>
	struct iterator_traits<momo::internal::DataRawIterator<R, S>>
		: public iterator_traits<const typename R::Item*>
	{
	};

	template<typename RR, typename RI>
	struct iterator_traits<momo::internal::DataRowIterator<RR, RI>>
	{
		typedef random_access_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename momo::internal::DataRowIterator<RR, RI>::Pointer pointer;
		typedef typename momo::internal::DataRowIterator<RR, RI>::Reference reference;
		typedef typename momo::internal::DataRowIterator<RR, RI>::Reference value_type;	//?
	};

	template<typename I, typename RI, typename S>
	struct iterator_traits<momo::internal::DataConstItemIterator<I, RI, S>>
	{
		typedef random_access_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename momo::internal::DataConstItemIterator<I, RI, S>::Pointer pointer;
		typedef typename momo::internal::DataConstItemIterator<I, RI, S>::Reference reference;
		typedef typename momo::internal::DataConstItemIterator<I, RI, S>::Item value_type;
	};
} // namespace std
