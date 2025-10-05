/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/DataSelection.h

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_DATA_SELECTION
#define MOMO_INCLUDE_GUARD_DATA_SELECTION

#include "DataColumn.h"
#include "Array.h"
#include "HashSorter.h"

namespace momo
{

namespace internal
{
	template<typename TRaws, typename TSettings>
	class DataRawIterator
	{
	protected:
		typedef TRaws Raws;
		typedef TSettings Settings;

	private:
		typedef typename Raws::Item RawPtr;
		typedef ArrayIndexIterator<const Raws, const RawPtr, Settings> IndexIterator;

		struct IndexIteratorProxy : public IndexIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(IndexIterator)
		};

	public:
		typedef typename IndexIterator::Reference Reference;
		typedef typename IndexIterator::Pointer Pointer;

		typedef DataRawIterator ConstIterator;

	public:
		explicit DataRawIterator() noexcept
		{
		}

		explicit DataRawIterator(const Raws& raws, size_t index) noexcept
			: mIndexIterator(IndexIteratorProxy(&raws, index))
		{
		}

		//operator ConstIterator() const noexcept

		DataRawIterator& operator+=(ptrdiff_t diff)
		{
			mIndexIterator += diff;
			return *this;
		}

		friend ptrdiff_t operator-(DataRawIterator iter1, DataRawIterator iter2)
		{
			return iter1.mIndexIterator - iter2.mIndexIterator;
		}

		Pointer operator->() const
		{
			return mIndexIterator.operator->();
		}

		friend bool operator==(DataRawIterator iter1, DataRawIterator iter2) noexcept
		{
			return iter1.mIndexIterator == iter2.mIndexIterator;
		}

#ifdef MOMO_HAS_THREE_WAY_COMPARISON
		friend auto operator<=>(DataRawIterator iter1, DataRawIterator iter2)
		{
			return iter1.mIndexIterator <=> iter2.mIndexIterator;
		}
#else
		friend bool operator<(DataRawIterator iter1, DataRawIterator iter2)
		{
			return iter1.mIndexIterator < iter2.mIndexIterator;
		}
#endif

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(DataRawIterator)

	private:
		IndexIterator mIndexIterator;
	};

	template<typename TRawIterator, typename TRowReference>
	class DataRowIterator : private VersionKeeper<typename TRowReference::Settings>
	{
	protected:
		typedef TRawIterator RawIterator;
		typedef TRowReference RowReference;
		typedef typename RowReference::ColumnList ColumnList;

	public:
		typedef const RowReference Reference;
		typedef IteratorPointer<Reference> Pointer;

		typedef DataRowIterator<RawIterator, typename RowReference::ConstReference> ConstIterator;

		typedef typename ColumnList::Settings Settings;

	protected:
		typedef internal::VersionKeeper<Settings> VersionKeeper;

	private:
		struct RowReferenceProxy : public RowReference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(RowReference)
		};

		struct ConstIteratorProxy : public ConstIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		};

	public:
		explicit DataRowIterator() noexcept
			: mColumnList(nullptr),
			mRawIterator()
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

		friend ptrdiff_t operator-(DataRowIterator iter1, DataRowIterator iter2)
		{
			MOMO_CHECK(iter1.mColumnList == iter2.mColumnList);
			return iter1.mRawIterator - iter2.mRawIterator;
		}

		Pointer operator->() const
		{
			MOMO_CHECK(mColumnList != nullptr);
			return Pointer(RowReferenceProxy(mColumnList, *mRawIterator, *this));
		}

		friend bool operator==(DataRowIterator iter1, DataRowIterator iter2) noexcept
		{
			return iter1.mColumnList == iter2.mColumnList
				&& iter1.mRawIterator == iter2.mRawIterator;
		}

#ifdef MOMO_HAS_THREE_WAY_COMPARISON
		friend auto operator<=>(DataRowIterator iter1, DataRowIterator iter2)
		{
			MOMO_CHECK(iter1.mColumnList == iter2.mColumnList);
			return iter1.mRawIterator <=> iter2.mRawIterator;
		}
#else
		friend bool operator<(DataRowIterator iter1, DataRowIterator iter2)
		{
			MOMO_CHECK(iter1.mColumnList == iter2.mColumnList);
			return iter1.mRawIterator < iter2.mRawIterator;
		}
#endif

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

	template<typename TRawBounds, typename TRowReference>
	class DataRowBounds : private VersionKeeper<typename TRowReference::Settings>
	{
	protected:
		typedef TRawBounds RawBounds;
		typedef TRowReference RowReference;
		typedef typename RowReference::ColumnList ColumnList;

	public:
		typedef DataRowIterator<typename RawBounds::Iterator, RowReference> Iterator;

		typedef DataRowBounds<RawBounds, typename RowReference::ConstReference> ConstBounds;

		typedef typename ColumnList::Settings Settings;

	protected:
		typedef internal::VersionKeeper<Settings> VersionKeeper;

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

		MOMO_FRIENDS_SIZE_BEGIN_END_CONST(DataRowBounds, Iterator)

		size_t GetCount() const noexcept
		{
			return mRawBounds.GetCount();
		}

		RowReference operator[](size_t index) const
		{
			MOMO_CHECK(index < GetCount());
			return *UIntMath<>::Next(GetBegin(), index);
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

	template<typename TRowIterator, typename TItem>
	class DataConstItemIterator
	{
	public:
		typedef TRowIterator RowIterator;
		typedef TItem Item;
		typedef typename RowIterator::Settings Settings;

		typedef const Item& Reference;
		typedef const Item* Pointer;

		typedef DataConstItemIterator ConstIterator;

	public:
		explicit DataConstItemIterator() noexcept
			: mRowIterator(),
			mOffset(0)
		{
		}

		explicit DataConstItemIterator(RowIterator rowIter, size_t offset) noexcept
			: mRowIterator(rowIter),
			mOffset(offset)
		{
		}

		//operator ConstIterator() const noexcept

		DataConstItemIterator& operator+=(ptrdiff_t diff)
		{
			mRowIterator += diff;
			return *this;
		}

		friend ptrdiff_t operator-(DataConstItemIterator iter1, DataConstItemIterator iter2)
		{
			MOMO_CHECK(iter1.mOffset == iter2.mOffset);
			return iter1.mRowIterator - iter2.mRowIterator;
		}

		Pointer operator->() const
		{
			return std::addressof(mRowIterator->template GetByOffset<Item>(mOffset));
		}

		friend bool operator==(DataConstItemIterator iter1, DataConstItemIterator iter2) noexcept
		{
			return iter1.mOffset == iter2.mOffset && iter1.mRowIterator == iter2.mRowIterator;
		}

#ifdef MOMO_HAS_THREE_WAY_COMPARISON
		friend auto operator<=>(DataConstItemIterator iter1, DataConstItemIterator iter2)
		{
			MOMO_CHECK(iter1.mOffset == iter2.mOffset);
			return iter1.mRowIterator <=> iter2.mRowIterator;
		}
#else
		friend bool operator<(DataConstItemIterator iter1, DataConstItemIterator iter2)
		{
			MOMO_CHECK(iter1.mOffset == iter2.mOffset);
			return iter1.mRowIterator < iter2.mRowIterator;
		}
#endif

		MOMO_MORE_ARRAY_ITERATOR_OPERATORS(DataConstItemIterator)

		RowIterator GetRowIterator() const noexcept
		{
			return mRowIterator;
		}

		size_t GetOffset() const noexcept
		{
			return mOffset;
		}

	private:
		RowIterator mRowIterator;
		size_t mOffset;
	};

	template<typename TRowBounds, typename TItem>
	class DataConstItemBounds
	{
	public:
		typedef TRowBounds RowBounds;
		typedef TItem Item;
		typedef typename RowBounds::Settings Settings;

		typedef DataConstItemIterator<typename RowBounds::Iterator, Item> Iterator;

		typedef DataConstItemBounds ConstBounds;

	public:
		explicit DataConstItemBounds() noexcept
			: mOffset(0)
		{
		}

		explicit DataConstItemBounds(RowBounds rowBounds, size_t offset) noexcept
			: mRowBounds(rowBounds),
			mOffset(offset)
		{
		}

		//operator ConstBounds() const noexcept

		Iterator GetBegin() const noexcept
		{
			return Iterator(mRowBounds.GetBegin(), mOffset);
		}

		Iterator GetEnd() const noexcept
		{
			return Iterator(mRowBounds.GetEnd(), mOffset);
		}

		MOMO_FRIENDS_SIZE_BEGIN_END_CONST(DataConstItemBounds, Iterator)

		size_t GetCount() const noexcept
		{
			return mRowBounds.GetCount();
		}

		typename Iterator::Reference operator[](size_t index) const
		{
			MOMO_CHECK(index < GetCount());
			return *UIntMath<>::Next(GetBegin(), index);
		}

	private:
		RowBounds mRowBounds;
		size_t mOffset;
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
		using Equality = DataEquality<Column<Item>>;

	protected:
		typedef internal::VersionKeeper<Settings> VersionKeeper;

		typedef Array<Raw*, MemManager, ArrayItemTraits<Raw*, MemManager>,
			NestedArraySettings<typename Settings::SelectionRawsSettings>> Raws;

	private:
		typedef typename RowReference::ConstReference ConstRowReference;

		typedef DataRawIterator<Raws, Settings> RawIterator;

		typedef ArrayBounds<RawIterator> RawBounds;
		typedef DataRowBounds<RawBounds, ConstRowReference> ConstRowBounds;

	public:
		typedef DataSelection<ConstRowReference, DataTraits> ConstSelection;

		typedef DataRowIterator<RawIterator, RowReference> ConstIterator;
		typedef ConstIterator Iterator;

		template<typename Item>
		using ConstItemBounds = DataConstItemBounds<ConstRowBounds, Item>;

	private:
		struct RowReferenceProxy : public RowReference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(RowReference)
			MOMO_DECLARE_PROXY_FUNCTION(RowReference, GetRaw)
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
		explicit DataSelection(const DataSelection& selection, const RowFilter& rowFilter)
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

		~DataSelection() = default;

		DataSelection& operator=(DataSelection&& selection) noexcept
		{
			return ContainerAssigner::Move(std::move(selection), *this);
		}

		DataSelection& operator=(const DataSelection& selection)
		{
			return ContainerAssigner::Copy(selection, *this);
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
		MOMO_FRIENDS_SIZE_BEGIN_END_CONST(DataSelection, ConstIterator)

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
			RawBounds rawBounds(RawIterator(mRaws, 0), GetCount());
			return ConstItemBounds<Item>(ConstRowBoundsProxy(mColumnList, rawBounds, *this),
				mColumnList->GetOffset(column));
		}

		template<typename RowIterator, typename RowSentinel>
		void Assign(RowIterator begin, RowSentinel end)
		{
			size_t initCount = GetCount();
			Add(std::move(begin), std::move(end));	//?
			mRaws.Remove(0, initCount);
		}

		void Add(RowReference rowRef)
		{
			rowRef.GetRaw();	// check
			MOMO_CHECK(mColumnList == &rowRef.GetColumnList());
			mRaws.AddBack(RowReferenceProxy::GetRaw(rowRef));
		}

		template<typename RowIterator, typename RowSentinel>
		void Add(RowIterator begin, RowSentinel end)
		{
			size_t initCount = GetCount();
			try
			{
				for (RowIterator iter = std::move(begin); iter != end; ++iter)
				{
					RowReference rowRef = *iter;
					MOMO_CHECK(&rowRef.GetColumnList() == mColumnList);
					mRaws.AddBack(RowReferenceProxy::GetRaw(rowRef));
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

		template<typename RowIterator, typename RowSentinel>
		void Insert(size_t index, RowIterator begin, RowSentinel end)
		{
			size_t initCount = GetCount();
			MOMO_CHECK(index <= initCount);
			Add(std::move(begin), std::move(end));
			std::rotate(UIntMath<>::Next(mRaws.GetBegin(), index),
				UIntMath<>::Next(mRaws.GetBegin(), initCount), mRaws.GetEnd());
		}

		void Remove(size_t index, size_t count = 1)
		{
			MOMO_CHECK(index + count <= GetCount());
			mRaws.Remove(index, count);
		}

		template<typename RowFilter>
		EnableIf<IsInvocable<const RowFilter&, bool, ConstRowReference>::value,
		size_t> Remove(const RowFilter& rowFilter)
		{
			size_t newCount = 0;
			for (Raw*& raw : mRaws)
			{
				if (rowFilter(pvMakeConstRowReference(raw)))
					continue;
				std::swap(mRaws[newCount], raw);
				++newCount;
			}
			size_t remCount = mRaws.GetCount() - newCount;
			mRaws.RemoveBack(remCount);
			return remCount;
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

		template<typename RowLessComparer>
		EnableIf<IsInvocable<const RowLessComparer&, bool, ConstRowReference, ConstRowReference>::value,
		DataSelection&&> Sort(const RowLessComparer& rowLessComp) &&
		{
			pvSort(rowLessComp);
			return std::move(*this);
		}

		template<typename RowLessComparer>
		EnableIf<IsInvocable<const RowLessComparer&, bool, ConstRowReference, ConstRowReference>::value,
		DataSelection&> Sort(const RowLessComparer& rowLessComp) &
		{
			pvSort(rowLessComp);
			return *this;
		}

		template<typename Item, typename... Items>
		DataSelection&& Group(const Column<Item>& column, const Column<Items>&... columns) &&
		{
			pvGroup(column, columns...);
			return std::move(*this);
		}

		template<typename Item, typename... Items>
		DataSelection& Group(const Column<Item>& column, const Column<Items>&... columns) &
		{
			pvGroup(column, columns...);
			return *this;
		}

		template<typename Item, typename... Items>
		size_t GetLowerBound(Equality<Item> equal, Equality<Items>... equals) const
		{
			return pvBinarySearch<true>(equal, equals...);
		}

		template<typename Item, typename... Items>
		size_t GetUpperBound(Equality<Item> equal, Equality<Items>... equals) const
		{
			return pvBinarySearch<false>(equal, equals...);
		}

		template<typename RowPredicate>
		size_t BinarySearch(const RowPredicate& rowPred) const
		{
			auto rawPred = [this, &rowPred] (Raw*, Raw* raw)
				{ return rowPred(pvMakeConstRowReference(raw)); };
			return UIntMath<>::Dist(mRaws.GetBegin(),
				std::upper_bound(mRaws.GetBegin(), mRaws.GetEnd(), nullptr, rawPred));
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

		void pvReverse() noexcept
		{
			std::reverse(mRaws.GetBegin(), mRaws.GetEnd());
		}

		template<typename... Items>
		void pvSort(const Column<Items>&... columns)
		{
			static const size_t columnCount = sizeof...(columns);
			std::array<size_t, columnCount> offsets = {{ mColumnList->GetOffset(columns)... }};
			auto rawLessComp = [&offsets] (Raw* raw1, Raw* raw2)
				{ return pvIsLess<void, Items...>(raw1, raw2, offsets.data()); };
			DataTraits::Sort(mRaws.GetBegin(), mRaws.GetCount(), rawLessComp, GetMemManager());
		}

		template<typename Void, typename Item, typename... Items>
		static bool pvIsLess(Raw* raw1, Raw* raw2, const size_t* offsets)
		{
			const Item& item1 = ColumnList::template GetByOffset<const Item>(raw1, *offsets);
			const Item& item2 = ColumnList::template GetByOffset<const Item>(raw2, *offsets);
			int cmp = DataTraits::Compare(item1, item2);
			if (cmp != 0)
				return cmp < 0;
			return pvIsLess<void, Items...>(raw1, raw2, offsets + 1);
		}

		template<typename Void>
		static bool pvIsLess(Raw* /*raw1*/, Raw* /*raw2*/, const size_t* /*offsets*/) noexcept
		{
			return false;
		}

		template<typename RowLessComparer>
		EnableIf<IsInvocable<const RowLessComparer&, bool, ConstRowReference, ConstRowReference>::value>
		pvSort(const RowLessComparer& rowLessComp)
		{
			auto rawLessComp = [this, &rowLessComp] (Raw* raw1, Raw* raw2)
				{ return rowLessComp(pvMakeConstRowReference(raw1), pvMakeConstRowReference(raw2)); };
			DataTraits::Sort(mRaws.GetBegin(), mRaws.GetCount(), rawLessComp, GetMemManager());
		}

		template<typename... Items>
		void pvGroup(const Column<Items>&... columns)
		{
			static const size_t columnCount = sizeof...(columns);
			std::array<size_t, columnCount> offsets = {{ mColumnList->GetOffset(columns)... }};
			auto rawHasher = [&offsets] (Raw* raw)
				{ return pvGetHashCode<void, Items...>(raw, offsets.data()); };
			auto rawEqualComp = [&offsets] (Raw* raw1, Raw* raw2)
				{ return pvIsEqual<void, Items...>(raw1, raw2, offsets.data()); };
			Array<size_t, MemManagerPtr<MemManager>> hashCodes(
				(MemManagerPtr<MemManager>(GetMemManager())));
			try
			{
				hashCodes.Reserve(mRaws.GetCount());
			}
			catch (const std::bad_alloc&)
			{
				HashSorter::Sort(mRaws.GetBegin(), mRaws.GetCount(), rawHasher, rawEqualComp);
				return;
			}
			for (Raw* raw : mRaws)
				hashCodes.AddBackNogrow(rawHasher(raw));
			HashSorter::SortPrehashed(mRaws.GetBegin(), mRaws.GetCount(),
				hashCodes.GetBegin(), rawEqualComp);
		}

		template<typename Void, typename Item, typename... Items>
		static size_t pvGetHashCode(Raw* raw, const size_t* offsets)
		{
			size_t offset = *offsets;
			const Item& item = ColumnList::template GetByOffset<const Item>(raw, offset);
			size_t hashCode = pvGetHashCode<void, Items...>(raw, offsets + 1);
			DataTraits::AccumulateHashCode(hashCode, item, offset);
			return hashCode;
		}

		template<typename Void>
		static size_t pvGetHashCode(Raw* /*raw*/, const size_t* /*offsets*/) noexcept
		{
			return 0;
		}

		template<typename Void, typename Item, typename... Items>
		static bool pvIsEqual(Raw* raw1, Raw* raw2, const size_t* offsets)
		{
			size_t offset = *offsets;
			const Item& item1 = ColumnList::template GetByOffset<const Item>(raw1, offset);
			const Item& item2 = ColumnList::template GetByOffset<const Item>(raw2, offset);
			return DataTraits::IsEqual(item1, item2)
				&& pvIsEqual<void, Items...>(raw1, raw2, offsets + 1);
		}

		template<typename Void>
		static bool pvIsEqual(Raw* /*raw1*/, Raw* /*raw2*/, const size_t* /*offsets*/) noexcept
		{
			return true;
		}

		template<bool includeEqual, typename... Items>
		size_t pvBinarySearch(const Equality<Items>&... equals) const
		{
			static const int bound = includeEqual ? -1 : 0;
			static const size_t columnCount = sizeof...(equals);
			std::array<size_t, columnCount> offsets =
				{{ mColumnList->GetOffset(equals.GetColumn())... }};
			auto rawPred = [&offsets, &equals...] (Raw*, Raw* raw)
				{ return pvCompare<void>(raw, offsets.data(), equals...) > bound; };
			return UIntMath<>::Dist(mRaws.GetBegin(),
				std::upper_bound(mRaws.GetBegin(), mRaws.GetEnd(), nullptr, rawPred));
		}

		template<typename Void, typename Item, typename... Items>
		static int pvCompare(Raw* raw, const size_t* offsets, const Equality<Item>& equal,
			const Equality<Items>&... equals)
		{
			const Item& item1 = ColumnList::template GetByOffset<const Item>(raw, *offsets);
			const Item& item2 = equal.GetItem();
			int cmp = DataTraits::Compare(item1, item2);
			if (cmp != 0)
				return cmp;
			return pvCompare<void>(raw, offsets + 1, equals...);
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
		: public momo::internal::IteratorTraitsStd<momo::internal::DataRawIterator<R, S>,
			random_access_iterator_tag>
	{
	};

	template<typename RI, typename RR>
	struct iterator_traits<momo::internal::DataRowIterator<RI, RR>>
		: public momo::internal::IteratorTraitsStd<momo::internal::DataRowIterator<RI, RR>,
			random_access_iterator_tag>
	{
	};

	template<typename RI, typename I>
	struct iterator_traits<momo::internal::DataConstItemIterator<RI, I>>
		: public momo::internal::IteratorTraitsStd<momo::internal::DataConstItemIterator<RI, I>,
			random_access_iterator_tag>
	{
	};
} // namespace std

#endif // MOMO_INCLUDE_GUARD_DATA_SELECTION
