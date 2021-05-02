/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataSelection.h

\**********************************************************/

#pragma once

#include "IteratorUtility.h"
#include "Array.h"
#include "HashSorter.h"

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
			size_t newIndex = static_cast<size_t>(static_cast<ptrdiff_t>(pvGetIndex()) + diff);
			(void)raws; (void)newIndex;
			MOMO_CHECK((raws != nullptr) ? newIndex <= raws->GetCount() : diff == 0);
			ArrayIndexIterator::operator+=(diff);
			return *this;
		}

		friend ptrdiff_t operator-(DataRawIterator iter1, DataRawIterator iter2)
		{
			MOMO_CHECK(iter1.pvGetRaws() == iter2.pvGetRaws());
			return static_cast<ptrdiff_t>(iter1.ptGetIndex() - iter2.ptGetIndex());
		}

		Pointer operator->() const
		{
			const Raws* raws = pvGetRaws();
			size_t index = pvGetIndex();
			MOMO_CHECK(raws != nullptr && index < raws->GetCount());
			return raws->GetItems() + index;
		}

		friend bool operator==(DataRawIterator iter1, DataRawIterator iter2) noexcept
		{
			return iter1.pvGetRaws() == iter2.pvGetRaws()
				&& iter1.pvGetIndex() == iter2.pvGetIndex();
		}

		friend auto operator<=>(DataRawIterator iter1, DataRawIterator iter2)
		{
			MOMO_CHECK(iter1.pvGetRaws() == iter2.pvGetRaws());
			return iter1.pvGetIndex() <=> iter2.pvGetIndex();
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

	template<typename RowIterator, typename RowReference>
	concept conceptDataRowIterator = conceptInputIterator<RowIterator> &&
		requires (RowIterator iter) { { *iter } -> std::convertible_to<RowReference>; };

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

		friend auto operator<=>(DataRowIterator iter1, DataRowIterator iter2)
		{
			MOMO_CHECK(iter1.mColumnList == iter2.mColumnList);
			return iter1.mRawIterator <=> iter2.mRawIterator;
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

		MOMO_FRIENDS_SIZE_BEGIN_END(DataRowBounds)

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

		friend auto operator<=>(DataConstItemIterator iter1, DataConstItemIterator iter2)
		{
			MOMO_CHECK(iter1.mOffset == iter2.mOffset);
			return iter1.mRowIterator <=> iter2.mRowIterator;
		}

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

		MOMO_FRIENDS_SIZE_BEGIN_END(DataConstItemBounds)

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
		using Equaler = internal::DataEqualer<Column<Item>, const Item&>;

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
		requires std::predicate<const RowFilter&, ConstRowReference>
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
		MOMO_FRIENDS_SIZE_BEGIN_END(DataSelection)

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

		template<conceptDataRowIterator<RowReference> RowIterator>
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

		template<conceptDataRowIterator<RowReference> RowIterator>
		void Add(RowIterator begin, RowIterator end)
		{
			size_t initCount = GetCount();
			try
			{
				for (RowIterator iter = begin; iter != end; ++iter)
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

		template<conceptDataRowIterator<RowReference> RowIterator>
		void Insert(size_t index, RowIterator begin, RowIterator end)
		{
			size_t initCount = GetCount();
			MOMO_CHECK(index <= initCount);
			Add(begin, end);
			std::rotate(UIntMath<>::Next(mRaws.GetBegin(), index),
				UIntMath<>::Next(mRaws.GetBegin(), initCount), mRaws.GetEnd());
		}

		void Remove(size_t index, size_t count = 1)
		{
			MOMO_CHECK(index + count <= GetCount());
			mRaws.Remove(index, count);
		}

		template<typename RowFilter>
		requires std::predicate<const RowFilter&, ConstRowReference>
		size_t Remove(const RowFilter& rowFilter)
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

		template<typename RowComparer>
		requires std::strict_weak_order<const RowComparer&, ConstRowReference, ConstRowReference>
		DataSelection&& Sort(const RowComparer& rowComp) &&
		{
			pvSort(rowComp);
			return std::move(*this);
		}

		template<typename RowComparer>
		requires std::strict_weak_order<const RowComparer&, ConstRowReference, ConstRowReference>
		DataSelection& Sort(const RowComparer& rowComp) &
		{
			pvSort(rowComp);
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
		requires std::predicate<const RowPredicate&, ConstRowReference>
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

		template<typename RowComparer>
		requires std::strict_weak_order<const RowComparer&, ConstRowReference, ConstRowReference>
		void pvSort(const RowComparer& rowComp)
		{
			auto rawComp = [this, &rowComp] (Raw* raw1, Raw* raw2)
				{ return rowComp(pvMakeConstRowReference(raw1), pvMakeConstRowReference(raw2)); };
			DataTraits::Sort(mRaws.GetBegin(), mRaws.GetCount(), rawComp, GetMemManager());
		}

		template<typename... Items>
		void pvGroup(const Column<Items>&... columns)
		{
			static const size_t columnCount = sizeof...(columns);
			std::array<size_t, columnCount> offsets = {{ mColumnList->GetOffset(columns)... }};
			auto hashFunc = [&offsets] (Raw* raw)
				{ return pvGetHashCode<void, Items...>(raw, offsets.data()); };
			auto equalFunc = [&offsets] (Raw* raw1, Raw* raw2)
				{ return pvIsEqual<void, Items...>(raw1, raw2, offsets.data()); };
			Array<size_t, MemManagerPtr<MemManager>> hashes(
				(MemManagerPtr<MemManager>(GetMemManager())));
			try
			{
				hashes.Reserve(mRaws.GetCount());
			}
			catch (const std::bad_alloc&)
			{
				HashSorter::Sort(mRaws.GetBegin(), mRaws.GetCount(), hashFunc, equalFunc);
				return;
			}
			for (Raw* raw : mRaws)
				hashes.AddBackNogrow(hashFunc(raw));
			HashSorter::SortPrehashed(mRaws.GetBegin(), mRaws.GetCount(),
				hashes.GetBegin(), equalFunc);
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

		template<int bound, typename... Items>
		size_t pvBinarySearch(const Equaler<Items>&... equalers) const
		{
			static const size_t columnCount = sizeof...(equalers);
			std::array<size_t, columnCount> offsets = {{ mColumnList->GetOffset(equalers.GetColumn())... }};
			auto rawPred = [&offsets, &equalers...] (Raw*, Raw* raw)
				{ return pvCompare<void>(raw, offsets.data(), equalers...) > bound; };
			return UIntMath<>::Dist(mRaws.GetBegin(),
				std::upper_bound(mRaws.GetBegin(), mRaws.GetEnd(), nullptr, rawPred));
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
