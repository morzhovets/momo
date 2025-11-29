/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataSelection.h

\**********************************************************/

#pragma once

#include "DataColumn.h"	//?
#include "Array.h"
#include "HashSorter.h"

namespace momo
{

namespace internal
{
	template<typename TRaws, typename TSettings>
	class DataRawIterator
		: public ArrayIndexIterator<const TRaws, const typename TRaws::Item, TSettings>
	{
	protected:
		typedef TRaws Raws;
		typedef TSettings Settings;

	private:
		typedef typename Raws::Item RawPtr;
		typedef internal::ArrayIndexIterator<const Raws, const RawPtr, Settings> ArrayIndexIterator;

	public:
		typedef DataRawIterator ConstIterator;

	public:
		explicit DataRawIterator() noexcept = default;

		explicit DataRawIterator(const Raws& raws, size_t index) noexcept
			: ArrayIndexIterator(&raws, index)
		{
		}

		//operator ConstIterator() const noexcept

		DataRawIterator& operator+=(ptrdiff_t diff)
		{
			ArrayIndexIterator::operator+=(diff);
			return *this;
		}
	};

	template<typename DataRowIterator, typename RowReference>
	concept conceptDataRowIterator = std::input_iterator<DataRowIterator> &&
		requires (DataRowIterator iter) { { *iter } -> std::convertible_to<RowReference>; };

	template<typename TRawIterator, typename TRowReference>
	class MOMO_EMPTY_BASES DataRowIterator
		: private VersionKeeper<typename TRowReference::Settings>,
		public ArrayIteratorBase
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

		friend auto operator<=>(DataRowIterator iter1, DataRowIterator iter2)
		{
			MOMO_CHECK(iter1.mColumnList == iter2.mColumnList);
			return iter1.mRawIterator <=> iter2.mRawIterator;
		}

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
	class MOMO_EMPTY_BASES DataRowBounds
		: private VersionKeeper<typename TRowReference::Settings>,
		public Rangeable
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
	class DataConstItemIterator : public ArrayIteratorBase
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

		friend auto operator<=>(DataConstItemIterator iter1, DataConstItemIterator iter2)
		{
			MOMO_CHECK(iter1.mOffset == iter2.mOffset);
			return iter1.mRowIterator <=> iter2.mRowIterator;
		}

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
	class DataConstItemBounds : public Rangeable
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

		size_t GetCount() const noexcept
		{
			return mRowBounds.GetCount();
		}

		decltype(auto) operator[](size_t index) const
		{
			MOMO_CHECK(index < GetCount());
			return *UIntMath<>::Next(GetBegin(), index);
		}

	private:
		RowBounds mRowBounds;
		size_t mOffset;
	};

	template<typename TRowReference, typename TDataTraits>
	class MOMO_EMPTY_BASES DataSelection
		: private VersionKeeper<typename TRowReference::ColumnList::Settings>,
		public Rangeable,
		public Swappable<DataSelection>
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

		typedef ArrayCore<ArrayItemTraits<Raw*, MemManager>,
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

		template<internal::conceptPredicate<ConstRowReference> RowFilter>
		explicit DataSelection(const DataSelection& selection, RowFilter rowFilter)
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

		~DataSelection() noexcept = default;

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

		template<conceptDataRowIterator<RowReference> RowIterator,
			conceptSentinel<RowIterator> RowSentinel>
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

		template<conceptDataRowIterator<RowReference> RowIterator,
			conceptSentinel<RowIterator> RowSentinel>
		void Add(RowIterator begin, RowSentinel end)
		{
			Finalizer fin(&DataSelection::pvDecCount, *this, GetCount());
			for (RowIterator iter = std::move(begin); iter != end; ++iter)
			{
				RowReference rowRef = *iter;
				MOMO_CHECK(&rowRef.GetColumnList() == mColumnList);
				mRaws.AddBack(RowReferenceProxy::GetRaw(rowRef));
			}
			fin.Detach();
		}

		void Insert(size_t index, RowReference rowRef)
		{
			rowRef.GetRaw();	// check
			MOMO_CHECK(index <= GetCount());
			MOMO_CHECK(mColumnList == &rowRef.GetColumnList());
			mRaws.Insert(index, RowReferenceProxy::GetRaw(rowRef));
		}

		template<conceptDataRowIterator<RowReference> RowIterator,
			conceptSentinel<RowIterator> RowSentinel>
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

		template<internal::conceptPredicate<ConstRowReference> RowFilter>
		size_t Remove(RowFilter rowFilter)
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

		template<internal::conceptPredicate<ConstRowReference, ConstRowReference> RowLessComparer>
		DataSelection&& Sort(RowLessComparer rowLessComp) &&
		{
			pvSort(FastCopyableFunctor(rowLessComp));
			return std::move(*this);
		}

		template<internal::conceptPredicate<ConstRowReference, ConstRowReference> RowLessComparer>
		DataSelection& Sort(RowLessComparer rowLessComp) &
		{
			pvSort(FastCopyableFunctor(rowLessComp));
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

		template<internal::conceptPredicate<ConstRowReference> RowPredicate>
		size_t BinarySearch(RowPredicate rowPred) const
		{
			auto rawPred = [this, fastRowPred = FastCopyableFunctor(rowPred)] (Raw*, Raw* raw)
				{ return fastRowPred(pvMakeConstRowReference(raw)); };
			return UIntMath<>::Dist(mRaws.GetBegin(),
				std::upper_bound(mRaws.GetBegin(), mRaws.GetEnd(),
					nullptr, FastCopyableFunctor(rawPred)));
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

		void pvDecCount(size_t count) noexcept
		{
			MOMO_ASSERT(count <= mRaws.GetCount());
			mRaws.SetCount(count);
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
				{ return pvIsLess<Items...>(offsets.data(), raw1, raw2); };
			DataTraits::Sort(mRaws.GetBegin(), mRaws.GetCount(),
				FastCopyableFunctor(rawLessComp), GetMemManager());
		}

		template<typename Item, typename... Items>
		static bool pvIsLess(const size_t* offsetPtr, Raw* raw1, Raw* raw2)
		{
			size_t offset = *offsetPtr;
			const Item& item1 = ColumnList::template GetByOffset<const Item>(raw1, offset);
			const Item& item2 = ColumnList::template GetByOffset<const Item>(raw2, offset);
			if (std::weak_ordering cmp = DataTraits::Compare(item1, item2); cmp != 0)
				return cmp < 0;
			if constexpr (sizeof...(Items) > 0)
				return pvIsLess<Items...>(offsetPtr + 1, raw1, raw2);
			else
				return false;
		}

		template<internal::conceptPredicate<ConstRowReference, ConstRowReference> RowLessComparer>
		void pvSort(FastCopyableFunctor<RowLessComparer> rowLessComp)
		{
			auto rawLessComp = [this, rowLessComp] (Raw* raw1, Raw* raw2)
			{
				return rowLessComp(pvMakeConstRowReference(raw1), pvMakeConstRowReference(raw2));
			};
			DataTraits::Sort(mRaws.GetBegin(), mRaws.GetCount(),
				FastCopyableFunctor(rawLessComp), GetMemManager());
		}

		template<typename... Items>
		void pvGroup(const Column<Items>&... columns)
		{
			static const size_t columnCount = sizeof...(columns);
			std::array<size_t, columnCount> offsets = {{ mColumnList->GetOffset(columns)... }};
			auto rawHasher = [&offsets] (Raw* raw)
			{
				size_t hashCode = 0;
				const size_t* offsetPtr = offsets.data();
				(pvAccumulateHashCode<Items>(hashCode, raw, *offsetPtr++), ...);
				return hashCode;
			};
			auto rawEqualComp = [&offsets] (Raw* raw1, Raw* raw2)
			{
				const size_t* offsetPtr = offsets.data();
				return (pvIsEqual<Items>(raw1, raw2, *offsetPtr++) && ...);
			};
			typedef Array<size_t, MemManagerPtr<MemManager>> HashCodes;
			HashCodes hashCodes((MemManagerPtr<MemManager>(GetMemManager())));
			if constexpr (internal::Catcher::allowExceptionSuppression<Settings>)
				Catcher::CatchAll(&HashCodes::Reserve, hashCodes, mRaws.GetCount());
			if (hashCodes.GetCapacity() >= mRaws.GetCount())
			{
				for (Raw* raw : mRaws)
					hashCodes.AddBackNogrow(rawHasher(raw));
				HashSorter::SortPrehashed(mRaws.GetBegin(), mRaws.GetCount(),
					hashCodes.GetBegin(), rawEqualComp);
			}
			else
			{
				HashSorter::Sort(mRaws.GetBegin(), mRaws.GetCount(), rawHasher, rawEqualComp);
			}
		}

		template<typename Item>
		static void pvAccumulateHashCode(size_t& hashCode, Raw* raw, size_t offset)
		{
			const Item& item = ColumnList::template GetByOffset<const Item>(raw, offset);
			DataTraits::AccumulateHashCode(hashCode, item, offset);
		}

		template<typename Item>
		static bool pvIsEqual(Raw* raw1, Raw* raw2, size_t offset)
		{
			const Item& item1 = ColumnList::template GetByOffset<const Item>(raw1, offset);
			const Item& item2 = ColumnList::template GetByOffset<const Item>(raw2, offset);
			return DataTraits::IsEqual(item1, item2);
		}

		template<bool includeEqual, typename... Items>
		size_t pvBinarySearch(const Equality<Items>&... equals) const
		{
			static const size_t columnCount = sizeof...(equals);
			std::array<size_t, columnCount> offsets =
				{{ mColumnList->GetOffset(equals.GetColumn())... }};
			auto rawPred = [&offsets, &equals...] (Raw*, Raw* raw)
			{
				std::weak_ordering cmp = pvCompare(offsets.data(), raw, equals...);
				return includeEqual ? cmp >= 0 : cmp > 0;
			};
			return UIntMath<>::Dist(mRaws.GetBegin(),
				std::upper_bound(mRaws.GetBegin(), mRaws.GetEnd(),
					nullptr, FastCopyableFunctor(rawPred)));
		}

		template<typename Item, typename... Items>
		static std::weak_ordering pvCompare(const size_t* offsetPtr, Raw* raw1,
			const Equality<Item>& equal2, const Equality<Items>&... equals2)
		{
			size_t offset = *offsetPtr;
			const Item& item1 = ColumnList::template GetByOffset<const Item>(raw1, offset);
			const Item& item2 = equal2.GetItem();
			if (std::weak_ordering cmp = DataTraits::Compare(item1, item2); cmp != 0)
				return cmp;
			if constexpr (sizeof...(Items) > 0)
				return pvCompare(offsetPtr + 1, raw1, equals2...);
			else
				return std::weak_ordering::equivalent;
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
