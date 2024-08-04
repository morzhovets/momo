/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/DataTable.h

  namespace momo:
    class DataTraits
    class DataTable
    class DataTableStatic
    class DataTableNative

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_DATA_TABLE
#define MOMO_INCLUDE_GUARD_DATA_TABLE

#include "DataColumn.h"
#include "DataRow.h"
#include "DataSelection.h"
#include "DataIndexes.h"
#include "Array.h"
#include "MemPool.h"

namespace momo
{

class DataTraits
{
public:
	typedef MemPoolParams<MemPoolConst::defaultBlockCount, 0> RawMemPoolParams;

	typedef HashBucketOpenDefault HashBucket;

	static const size_t selectEqualityMaxCount = 6;

public:
	template<typename Item>
	static void AccumulateHashCode(size_t& hashCode, const Item& item, size_t /*offset*/)
	{
		hashCode += HashCoder<Item>()(item);
	}

	template<typename Item>
	static bool IsEqual(const Item& item1, const Item& item2)
	{
		return item1 == item2;
	}

	template<typename Item>
	static int Compare(const Item& item1, const Item& item2)
	{
#ifdef MOMO_HAS_THREE_WAY_COMPARISON
		std::weak_ordering cmp = std::compare_weak_order_fallback(item1, item2);
		return (cmp < 0) ? -1 : (cmp > 0) ? 1 : 0;
#else
		if (std::less<Item>()(item1, item2))
			return -1;
		return (item1 == item2) ? 0 : 1;
#endif
	}

	template<typename Iterator, typename LessFunc, typename MemManager>
	static void Sort(Iterator begin, size_t count, const LessFunc& lessFunc,
		MemManager& /*memManager*/)
	{
		std::sort(begin, internal::UIntMath<>::Next(begin, count), lessFunc);	//?
	}
};

/*!
	\brief
	`momo::DataTable` is similar to `Boost.MultiIndex`, but its API looks like `ADO.NET DataTable`.
*/

template<typename TColumnList = DataColumnList<>,
	typename TDataTraits = DataTraits>
class DataTable
{
public:
	typedef TColumnList ColumnList;
	typedef TDataTraits DataTraits;
	typedef typename ColumnList::MemManager MemManager;
	typedef typename ColumnList::Settings Settings;
	typedef typename ColumnList::Raw Raw;

	template<typename Item>
	using Column = typename ColumnList::template Column<Item>;

	template<typename Item>
	using Equality = DataEquality<Column<Item>>;

	template<typename... Items>
	using Equalities = DataEquality<Column<Items>...>;

	template<typename Item, typename ItemArg>
	using Assignment = DataAssignment<Column<Item>, ItemArg>;

	typedef internal::DataRow<ColumnList> Row;
	typedef internal::DataRowReference<ColumnList> RowReference;
	typedef typename RowReference::ConstReference ConstRowReference;

	typedef internal::DataSelection<RowReference, DataTraits> Selection;
	typedef typename Selection::ConstSelection ConstSelection;

private:
	typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

	typedef Array<Raw*, MemManagerPtr, ArrayItemTraits<Raw*, MemManagerPtr>,
		internal::NestedArraySettings<typename Settings::TableRawsSettings>> Raws;

	typedef internal::DataIndexes<ColumnList, DataTraits> Indexes;

	typedef internal::DataRawIterator<Raws, Settings> RawIterator;

	typedef internal::ArrayBounds<RawIterator> RawBounds;
	typedef internal::DataRowBounds<RawBounds, ConstRowReference> ConstRowBounds;

public:
	typedef internal::DataRowIterator<RawIterator, RowReference> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::DataRowPointer<internal::DataRowBounds<
		typename Indexes::UniqueHashRawBounds, RowReference>> RowHashPointer;
	typedef typename RowHashPointer::ConstPointer ConstRowHashPointer;

	typedef internal::DataRowBounds<
		typename Indexes::MultiHashRawBounds, RowReference> RowHashBounds;
	typedef typename RowHashBounds::ConstBounds ConstRowHashBounds;

	template<typename Item>
	using ConstItemBounds = internal::DataConstItemBounds<ConstRowBounds, Item>;

	typedef typename Indexes::UniqueHashIndex UniqueHashIndex;
	typedef typename Indexes::MultiHashIndex MultiHashIndex;

	class TryResult
	{
	public:
		explicit operator bool() const noexcept
		{
			return uniqueHashIndex == UniqueHashIndex::empty;
		}

	public:
		RowReference rowReference;
		UniqueHashIndex uniqueHashIndex;
	};

	class UniqueIndexViolation : public std::runtime_error, public TryResult
	{
	public:
		explicit UniqueIndexViolation(const TryResult& tryResult)
			: std::runtime_error("Unique index violation"),
			TryResult(tryResult)
		{
		}
	};

private:
	typedef internal::VersionKeeper<Settings> VersionKeeper;

	static const size_t invalidNumber = internal::UIntConst::maxSize;

	typedef MemPool<typename DataTraits::RawMemPoolParams, MemManagerPtr,
		internal::NestedMemPoolSettings> RawMemPool;

	template<typename... Items>
	using OffsetItemTuple = typename Indexes::template OffsetItemTuple<Items...>;

	struct EmptyRowFilter
	{
		bool operator()(ConstRowReference /*rowRef*/) const noexcept
		{
			return true;
		}
	};

	typedef std::atomic<void*> FreeRaws;

	class Crew
	{
		MOMO_STATIC_ASSERT(std::is_nothrow_move_constructible<ColumnList>::value);

	private:
		typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

		class Data
		{
		public:
			explicit Data(ColumnList&& columnList) noexcept
				: columnList(std::move(columnList)),
				changeVersion(0),
				removeVersion(0),
				freeRaws(nullptr)
			{
			}

		public:
			ColumnList columnList;
			size_t changeVersion;
			size_t removeVersion;
			FreeRaws freeRaws;
		};

	public:
		explicit Crew(ColumnList&& columnList)
		{
			mData = MemManagerProxy::template Allocate<Data>(columnList.GetMemManager(),
				sizeof(Data));
			::new(static_cast<void*>(mData)) Data(std::move(columnList));
		}

		Crew(Crew&& crew) noexcept
			: mData(nullptr)
		{
			Swap(crew);
		}

		Crew(const Crew&) = delete;

		~Crew() noexcept
		{
			if (!IsNull())
			{
				ColumnList columnList = std::move(mData->columnList);
				mData->~Data();
				MemManagerProxy::Deallocate(columnList.GetMemManager(), mData, sizeof(Data));
			}
		}

		Crew& operator=(const Crew&) = delete;

		void Swap(Crew& crew) noexcept
		{
			std::swap(mData, crew.mData);
		}

		bool IsNull() const noexcept
		{
			return mData == nullptr;
		}

		const ColumnList& GetColumnList() const noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->columnList;
		}

		const MemManager& GetMemManager() const noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->columnList.GetMemManager();
		}

		MemManager& GetMemManager() noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->columnList.GetMemManager();
		}

		const size_t& GetChangeVersion() const noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->changeVersion;
		}

		size_t& GetChangeVersion() noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->changeVersion;
		}

		const size_t& GetRemoveVersion() const noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->removeVersion;
		}

		size_t& GetRemoveVersion() noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->removeVersion;
		}

		FreeRaws& GetFreeRaws() noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->freeRaws;
		}

	private:
		Data* mData;
	};

	struct RowProxy : public Row
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Row)
		MOMO_DECLARE_PROXY_FUNCTION(Row, GetRaw, Raw*)
		MOMO_DECLARE_PROXY_FUNCTION(Row, ExtractRaw, Raw*)
	};

	struct ConstRowReferenceProxy : public ConstRowReference
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstRowReference)
		MOMO_DECLARE_PROXY_FUNCTION(ConstRowReference, GetRaw, Raw*)
	};

	struct RowReferenceProxy : public RowReference
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(RowReference)
		//MOMO_DECLARE_PROXY_FUNCTION(RowReference, GetRaw, Raw*)
	};

	struct IteratorProxy : public Iterator
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
	};

	struct SelectionProxy : public Selection
	{
		typedef typename Selection::Raws Raws;
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Selection)
	};

	struct RowHashPointerProxy : public RowHashPointer
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(RowHashPointer)
	};

	struct RowHashBoundsProxy : public RowHashBounds
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(RowHashBounds)
	};

	struct ConstRowBoundsProxy : public ConstRowBounds
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstRowBounds)
	};

public:
	explicit DataTable()
		: DataTable(ColumnList())
	{
		MOMO_STATIC_ASSERT(!std::is_void<Raw>::value);
	}

	explicit DataTable(ColumnList&& columnList)
		: mCrew(std::move(columnList)),
		mRaws(MemManagerPtr(GetMemManager())),
		mRawMemPool(pvCreateRawMemPool()),
		mIndexes(GetMemManager())
	{
	}

	DataTable(DataTable&& table) noexcept
		: mCrew(std::move(table.mCrew)),
		mRaws(std::move(table.mRaws)),
		mRawMemPool(std::move(table.mRawMemPool)),
		mIndexes(std::move(table.mIndexes))
	{
	}

	DataTable(const DataTable& table)
		: DataTable(table, EmptyRowFilter())
	{
	}

	template<typename RowFilter>
	explicit DataTable(const DataTable& table, const RowFilter& rowFilter)
		: DataTable(ColumnList(table.GetColumnList()))
	{
		mIndexes.Assign(table.mIndexes);
		pvFill(table, rowFilter);
	}

	explicit DataTable(const ConstSelection& selection)
		: DataTable(ColumnList(selection.GetColumnList()))
	{
		pvFill(selection, EmptyRowFilter());
	}

	explicit DataTable(const Selection& selection)
		: DataTable(ColumnList(selection.GetColumnList()))
	{
		pvFill(selection, EmptyRowFilter());
	}

	~DataTable() noexcept
	{
		pvDestroyRaws();
	}

	DataTable& operator=(DataTable&& table) noexcept
	{
		DataTable(std::move(table)).Swap(*this);
		return *this;
	}

	DataTable& operator=(const DataTable& table)
	{
		if (this != &table)
			DataTable(table).Swap(*this);
		return *this;
	}

	void Swap(DataTable& table) noexcept
	{
		mCrew.Swap(table.mCrew);
		mRaws.Swap(table.mRaws);
		mRawMemPool.Swap(table.mRawMemPool);
		mIndexes.Swap(table.mIndexes);
	}

	ConstIterator GetBegin() const noexcept
	{
		return pvMakeIterator(0);
	}

	Iterator GetBegin() noexcept
	{
		return pvMakeIterator(0);
	}

	ConstIterator GetEnd() const noexcept
	{
		return pvMakeIterator(GetCount());
	}

	Iterator GetEnd() noexcept
	{
		return pvMakeIterator(GetCount());
	}

	MOMO_FRIEND_SWAP(DataTable)
	MOMO_FRIENDS_SIZE_BEGIN_END_CONST(DataTable, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(DataTable, Iterator)

	const ColumnList& GetColumnList() const noexcept
	{
		return mCrew.GetColumnList();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mCrew.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mCrew.GetMemManager();
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
		mIndexes.ClearRaws();
		pvDestroyRaws();
		mRaws.Clear();
		++mCrew.GetChangeVersion();
		++mCrew.GetRemoveVersion();
	}

	void Reserve(size_t capacity)
	{
		mRaws.Reserve(capacity);
		mIndexes.Reserve(capacity);
	}

	const ConstRowReference operator[](size_t rowNumber) const
	{
		MOMO_CHECK(rowNumber < GetCount());
		return pvMakeConstRowReference(mRaws[rowNumber]);
	}

	const RowReference operator[](size_t rowNumber)
	{
		MOMO_CHECK(rowNumber < GetCount());
		return pvMakeRowReference(mRaws[rowNumber]);
	}

	template<typename Item>
	bool ContainsColumn(const Column<Item>& column) const
	{
		return GetColumnList().Contains(column);
	}

	template<typename Item>
	ConstItemBounds<Item> GetColumnItems(const Column<Item>& column) const
	{
		const ColumnList& columnList = GetColumnList();
		RawBounds rawBounds(RawIterator(mRaws, 0), GetCount());
		return ConstItemBounds<Item>(
			ConstRowBoundsProxy(&columnList, rawBounds, VersionKeeper(&mCrew.GetRemoveVersion())),
			columnList.GetOffset(column));
	}

	Row NewRow()
	{
		return pvMakeRow(pvCreateRaw());
	}

	template<typename Item, typename ItemArg, typename... Items, typename... ItemArgs>
	Row NewRow(Assignment<Item, ItemArg> assign, Assignment<Items, ItemArgs>... assigns)
	{
		return pvNewRow(assign, assigns...);
	}

	Row NewRow(typename std::conditional<std::is_void<Raw>::value,
		std::piecewise_construct_t, Raw>::type&& srcRaw)
	{
		MOMO_STATIC_ASSERT(!std::is_void<Raw>::value);
		auto rawCreator = [this, &srcRaw] (Raw* raw)
			{ GetColumnList().CreateRaw(GetMemManager(), std::move(srcRaw), raw); };
		return pvMakeRow(pvCreateRaw(rawCreator));
	}

	Row NewRow(const Row& row)
	{
		return pvMakeRow(pvImportRaw(row.GetColumnList(), row.GetRaw()));
	}

	Row NewRow(ConstRowReference rowRef)
	{
		return pvMakeRow(pvImportRaw(rowRef.GetColumnList(), rowRef.GetRaw()));
	}

	RowReference Add(Row&& row)
	{
		TryResult res = TryAdd(std::move(row));
		if (!res)
			throw UniqueIndexViolation(res);
		return res.rowReference;
	}

	template<typename Item, typename ItemArg, typename... Items, typename... ItemArgs>
	RowReference AddRow(Assignment<Item, ItemArg> assign, Assignment<Items, ItemArgs>... assigns)
	{
		return Add(pvNewRow(assign, assigns...));
	}

	TryResult TryAdd(Row&& row)
	{
		MOMO_CHECK(&row.GetColumnList() == &GetColumnList());
		mRaws.Reserve(mRaws.GetCount() + 1);
		auto res = mIndexes.AddRaw(row.GetRaw());
		if (res.raw != nullptr)
			return { pvMakeRowReference(res.raw), res.uniqueHashIndex };
		Raw* raw = RowProxy::ExtractRaw(row);
		pvSetNumber(raw, mRaws.GetCount());
		mRaws.AddBackNogrow(raw);
		++mCrew.GetChangeVersion();
		return { pvMakeRowReference(raw), UniqueHashIndex::empty };
	}

	template<typename Item, typename ItemArg, typename... Items, typename... ItemArgs>
	TryResult TryAddRow(Assignment<Item, ItemArg> assign, Assignment<Items, ItemArgs>... assigns)
	{
		return TryAdd(pvNewRow(assign, assigns...));
	}

	RowReference Insert(size_t rowNumber, Row&& row)
	{
		TryResult res = TryInsert(rowNumber, std::move(row));
		if (!res)
			throw UniqueIndexViolation(res);
		return res.rowReference;
	}

	template<typename Item, typename ItemArg, typename... Items, typename... ItemArgs>
	RowReference InsertRow(size_t rowNumber, Assignment<Item, ItemArg> assign,
		Assignment<Items, ItemArgs>... assigns)
	{
		return Insert(rowNumber, pvNewRow(assign, assigns...));
	}

	TryResult TryInsert(size_t rowNumber, Row&& row)
	{
		MOMO_CHECK(rowNumber <= GetCount());
		TryResult res = TryAdd(std::move(row));
		if (res)
		{
			std::rotate(internal::UIntMath<>::Next(mRaws.GetBegin(), rowNumber),
				std::prev(mRaws.GetEnd()), mRaws.GetEnd());
			pvSetNumbers(rowNumber);
		}
		return res;
	}

	template<typename Item, typename ItemArg, typename... Items, typename... ItemArgs>
	TryResult TryInsertRow(size_t rowNumber, Assignment<Item, ItemArg> assign,
		Assignment<Items, ItemArgs>... assigns)
	{
		return TryInsert(rowNumber, pvNewRow(assign, assigns...));
	}

	RowReference Update(size_t rowNumber, Row&& row)
	{
		TryResult res = TryUpdate(rowNumber, std::move(row));
		if (!res)
			throw UniqueIndexViolation(res);
		return res.rowReference;
	}

	template<typename Item>
	RowReference Update(ConstRowReference rowRef, const Column<Item>& column,
		internal::Identity<Item>&& newItem)
	{
		TryResult res = pvTryUpdate(rowRef, column, std::move(newItem));
		if (!res)
			throw UniqueIndexViolation(res);
		return res.rowReference;
	}

	template<typename Item>
	RowReference Update(ConstRowReference rowRef, const Column<Item>& column,
		const internal::Identity<Item>& newItem)
	{
		TryResult res = pvTryUpdate(rowRef, column, newItem);
		if (!res)
			throw UniqueIndexViolation(res);
		return res.rowReference;
	}

	TryResult TryUpdate(size_t rowNumber, Row&& row)
	{
		MOMO_CHECK(rowNumber < GetCount());
		Raw*& raw = mRaws[rowNumber];
		auto res = mIndexes.UpdateRaw(raw, row.GetRaw());
		if (res.raw != nullptr)
			return { pvMakeRowReference(res.raw), res.uniqueHashIndex };
		pvDestroyRaw(raw);
		raw = RowProxy::ExtractRaw(row);
		pvSetNumber(raw, rowNumber);
		++mCrew.GetChangeVersion();
		++mCrew.GetRemoveVersion();
		return { pvMakeRowReference(raw), UniqueHashIndex::empty };
	}

	template<typename Item>
	TryResult TryUpdate(ConstRowReference rowRef, const Column<Item>& column,
		internal::Identity<Item>&& newItem)
	{
		return pvTryUpdate(rowRef, column, std::move(newItem));
	}

	template<typename Item>
	TryResult TryUpdate(ConstRowReference rowRef, const Column<Item>& column,
		const internal::Identity<Item>& newItem)
	{
		return pvTryUpdate(rowRef, column, newItem);
	}

	template<typename RowIterator>
	void Assign(RowIterator begin, RowIterator end)
	{
		pvAssign(begin, end);
	}

	template<typename RowIterator>
	void Remove(RowIterator begin, RowIterator end)
	{
		pvRemove(begin, end);
	}

	template<typename RowFilter>
	internal::EnableIf<internal::IsInvocable<const RowFilter&, bool, ConstRowReference>::value,
	size_t> Remove(const RowFilter& rowFilter)
	{
		size_t initCount = GetCount();
		pvRemove(rowFilter);
		return initCount - GetCount();
	}

	void Remove(ConstRowReference rowRef)
	{
		MOMO_CHECK(&rowRef.GetColumnList() == &GetColumnList());
		pvDestroyRaw(pvExtractRaw(rowRef));
	}

	void Remove(size_t rowNumber, bool keepRowOrder = true)
	{
		MOMO_CHECK(rowNumber < GetCount());
		pvDestroyRaw(pvExtractRaw(rowNumber, keepRowOrder));
	}

	Row Extract(ConstRowReference rowRef)
	{
		MOMO_CHECK(&rowRef.GetColumnList() == &GetColumnList());
		return pvMakeRow(pvExtractRaw(rowRef));
	}

	Row Extract(size_t rowNumber, bool keepRowOrder = true)
	{
		MOMO_CHECK(rowNumber < GetCount());
		return pvMakeRow(pvExtractRaw(rowNumber, keepRowOrder));
	}

	template<typename Item, typename... Items>
	UniqueHashIndex GetUniqueHashIndex(const Column<Item>& column, const Column<Items>&... columns) const
	{
		return mIndexes.GetUniqueHashIndex(pvGetOffsets(column, columns...));
	}

	template<typename Item, typename... Items>
	MultiHashIndex GetMultiHashIndex(const Column<Item>& column, const Column<Items>&... columns) const
	{
		return mIndexes.GetMultiHashIndex(pvGetOffsets(column, columns...));
	}

	template<typename Item, typename... Items>
	UniqueHashIndex AddUniqueHashIndex(const Column<Item>& column, const Column<Items>&... columns)
	{
		auto offsets = pvGetOffsets(column, columns...);
		pvCheckImmutable(offsets);
		auto res = mIndexes.template AddUniqueHashIndex<Item, Items...>(mRaws, offsets);
		if (res.raw != nullptr)
			throw UniqueIndexViolation({ pvMakeRowReference(res.raw), UniqueHashIndex::empty });
		return res.uniqueHashIndex;
	}

	template<typename Item, typename... Items>
	MultiHashIndex AddMultiHashIndex(const Column<Item>& column, const Column<Items>&... columns)
	{
		auto offsets = pvGetOffsets(column, columns...);
		pvCheckImmutable(offsets);
		return mIndexes.template AddMultiHashIndex<Item, Items...>(mRaws, offsets);
	}

	void RemoveUniqueHashIndexes() noexcept
	{
		mIndexes.RemoveUniqueHashIndexes();
	}

	void RemoveMultiHashIndexes() noexcept
	{
		mIndexes.RemoveMultiHashIndexes();
	}

	ConstSelection SelectEmpty() const
	{
		return pvSelectEmpty();
	}

	Selection SelectEmpty()
	{
		return pvSelectEmpty();
	}

	template<typename RowFilter = EmptyRowFilter>
	internal::EnableIf<internal::IsInvocable<const RowFilter&, bool, ConstRowReference>::value,
	ConstSelection> Select(const RowFilter& rowFilter = RowFilter()) const
	{
		return pvSelect<Selection>(rowFilter);
	}

	template<typename RowFilter, typename Item, typename... Items>
	internal::EnableIf<internal::IsInvocable<const RowFilter&, bool, ConstRowReference>::value,
	ConstSelection> Select(const RowFilter& rowFilter, Equality<Item> equal,
		Equality<Items>... equals) const
	{
		return pvSelect<Selection>(rowFilter, equal, equals...);
	}

	template<typename RowFilter = EmptyRowFilter,
		typename... Items>
	internal::EnableIf<internal::IsInvocable<const RowFilter&, bool, ConstRowReference>::value,
	ConstSelection> Select(Equalities<Items...> equals, const RowFilter& rowFilter = RowFilter()) const
	{
		return pvSelect<Selection>(equals, rowFilter);
	}

	template<typename RowFilter = EmptyRowFilter>
	internal::EnableIf<internal::IsInvocable<const RowFilter&, bool, ConstRowReference>::value,
	Selection> Select(const RowFilter& rowFilter = RowFilter())
	{
		return pvSelect<Selection>(rowFilter);
	}

	template<typename RowFilter, typename Item, typename... Items>
	internal::EnableIf<internal::IsInvocable<const RowFilter&, bool, ConstRowReference>::value,
	Selection> Select(const RowFilter& rowFilter, Equality<Item> equal,
		Equality<Items>... equals)
	{
		return pvSelect<Selection>(rowFilter, equal, equals...);
	}

	template<typename RowFilter = EmptyRowFilter,
		typename... Items>
	internal::EnableIf<internal::IsInvocable<const RowFilter&, bool, ConstRowReference>::value,
	Selection> Select(Equalities<Items...> equals, const RowFilter& rowFilter = RowFilter())
	{
		return pvSelect<Selection>(equals, rowFilter);
	}

	template<typename RowFilter = EmptyRowFilter>
	internal::EnableIf<internal::IsInvocable<const RowFilter&, bool, ConstRowReference>::value,
	size_t> SelectCount(const RowFilter& rowFilter = RowFilter()) const
	{
		return pvSelect<size_t>(rowFilter);
	}

	template<typename RowFilter, typename Item, typename... Items>
	internal::EnableIf<internal::IsInvocable<const RowFilter&, bool, ConstRowReference>::value,
	size_t> SelectCount(const RowFilter& rowFilter, Equality<Item> equal,
		Equality<Items>... equals) const
	{
		return pvSelect<size_t>(rowFilter, equal, equals...);
	}

	template<typename RowFilter = EmptyRowFilter,
		typename... Items>
	internal::EnableIf<internal::IsInvocable<const RowFilter&, bool, ConstRowReference>::value,
	size_t> SelectCount(Equalities<Items...> equals, const RowFilter& rowFilter = RowFilter()) const
	{
		return pvSelect<size_t>(equals, rowFilter);
	}

	ConstRowHashPointer FindByUniqueHash(UniqueHashIndex uniqueHashIndex, const Row& row) const
	{
		return pvFindByUniqueHash(uniqueHashIndex, row);
	}

	RowHashPointer FindByUniqueHash(UniqueHashIndex uniqueHashIndex, const Row& row)
	{
		return pvFindByUniqueHash(uniqueHashIndex, row);
	}

	template<typename... Items>
	ConstRowHashPointer FindByUniqueHash(Equalities<Items...> equals,
		UniqueHashIndex uniqueHashIndex = UniqueHashIndex::empty) const
	{
		return pvFindByHash<RowHashPointerProxy>(equals, uniqueHashIndex);
	}

	template<typename... Items>
	RowHashPointer FindByUniqueHash(Equalities<Items...> equals,
		UniqueHashIndex uniqueHashIndex = UniqueHashIndex::empty)
	{
		return pvFindByHash<RowHashPointerProxy>(equals, uniqueHashIndex);
	}

	template<typename Item, typename... Items>
	ConstRowHashPointer FindByUniqueHash(UniqueHashIndex uniqueHashIndex, Equality<Item> equal,
		Equality<Items>... equals) const
	{
		return pvFindByHash<RowHashPointerProxy>(uniqueHashIndex, equal, equals...);
	}

	template<typename Item, typename... Items>
	RowHashPointer FindByUniqueHash(UniqueHashIndex uniqueHashIndex, Equality<Item> equal,
		Equality<Items>... equals)
	{
		return pvFindByHash<RowHashPointerProxy>(uniqueHashIndex, equal, equals...);
	}

	template<typename... Items>
	ConstRowHashBounds FindByMultiHash(Equalities<Items...> equals,
		MultiHashIndex multiHashIndex = MultiHashIndex::empty) const
	{
		return pvFindByHash<RowHashBoundsProxy>(equals, multiHashIndex);
	}

	template<typename... Items>
	RowHashBounds FindByMultiHash(Equalities<Items...> equals,
		MultiHashIndex multiHashIndex = MultiHashIndex::empty)
	{
		return pvFindByHash<RowHashBoundsProxy>(equals, multiHashIndex);
	}

	template<typename Item, typename... Items>
	ConstRowHashBounds FindByMultiHash(MultiHashIndex multiHashIndex, Equality<Item> equal,
		Equality<Items>... equals) const
	{
		return pvFindByHash<RowHashBoundsProxy>(multiHashIndex, equal, equals...);
	}

	template<typename Item, typename... Items>
	RowHashBounds FindByMultiHash(MultiHashIndex multiHashIndex, Equality<Item> equal,
		Equality<Items>... equals)
	{
		return pvFindByHash<RowHashBoundsProxy>(multiHashIndex, equal, equals...);
	}

	template<typename Item, typename... Items>
	DataTable Project(ColumnList&& resColumnList, const Column<Item>& column,
		const Column<Items>&... columns) const
	{
		return pvProject<false>(std::move(resColumnList), EmptyRowFilter(), column, columns...);
	}

	template<typename RowFilter, typename Item, typename... Items>
	internal::EnableIf<internal::IsInvocable<const RowFilter&, bool, ConstRowReference>::value,
	DataTable> Project(ColumnList&& resColumnList, const RowFilter& rowFilter,
		const Column<Item>& column, const Column<Items>&... columns) const
	{
		return pvProject<false>(std::move(resColumnList), rowFilter, column, columns...);
	}

	template<typename Item, typename... Items>
	DataTable ProjectDistinct(ColumnList&& resColumnList, const Column<Item>& column,
		const Column<Items>&... columns) const
	{
		return pvProject<true>(std::move(resColumnList), EmptyRowFilter(), column, columns...);
	}

	template<typename RowFilter, typename Item, typename... Items>
	internal::EnableIf<internal::IsInvocable<const RowFilter&, bool, ConstRowReference>::value,
	DataTable> ProjectDistinct(ColumnList&& resColumnList, const RowFilter& rowFilter,
		const Column<Item>& column, const Column<Items>&... columns) const
	{
		return pvProject<true>(std::move(resColumnList), rowFilter, column, columns...);
	}

	RowReference MakeMutableReference(ConstRowReference rowRef)
	{
		MOMO_CHECK(&rowRef.GetColumnList() == &GetColumnList());
		rowRef.GetRaw();	// check
		Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
		return pvMakeRowReference(raw);
	}

	MOMO_DEPRECATED RowReference AddRow(Row&& row)
	{
		return Add(std::move(row));
	}

	MOMO_DEPRECATED TryResult TryAddRow(Row&& row)
	{
		return TryAdd(std::move(row));
	}

	MOMO_DEPRECATED RowReference InsertRow(size_t rowNumber, Row&& row)
	{
		return Insert(rowNumber, std::move(row));
	}

	MOMO_DEPRECATED TryResult TryInsertRow(size_t rowNumber, Row&& row)
	{
		return TryInsert(rowNumber, std::move(row));
	}

	MOMO_DEPRECATED RowReference UpdateRow(size_t rowNumber, Row&& row)
	{
		return Update(rowNumber, std::move(row));
	}

	template<typename Item>
	MOMO_DEPRECATED RowReference UpdateRow(ConstRowReference rowRef, const Column<Item>& column,
		internal::Identity<Item>&& newItem)
	{
		return Update(rowRef, column, std::move(newItem));
	}

	template<typename Item>
	MOMO_DEPRECATED RowReference UpdateRow(ConstRowReference rowRef, const Column<Item>& column,
		const internal::Identity<Item>& newItem)
	{
		return Update(rowRef, column, newItem);
	}

	MOMO_DEPRECATED TryResult TryUpdateRow(size_t rowNumber, Row&& row)
	{
		return TryUpdate(rowNumber, std::move(row));
	}

	template<typename Item>
	MOMO_DEPRECATED TryResult TryUpdateRow(ConstRowReference rowRef, const Column<Item>& column,
		internal::Identity<Item>&& newItem)
	{
		return TryUpdate(rowRef, column, std::move(newItem));
	}

	template<typename Item>
	MOMO_DEPRECATED TryResult TryUpdateRow(ConstRowReference rowRef, const Column<Item>& column,
		const internal::Identity<Item>& newItem)
	{
		return TryUpdate(rowRef, column, newItem);
	}

	template<typename RowIterator>
	MOMO_DEPRECATED void AssignRows(RowIterator begin, RowIterator end)
	{
		Assign(begin, end);
	}

	template<typename RowIterator>
	MOMO_DEPRECATED void RemoveRows(RowIterator begin, RowIterator end)
	{
		Remove(begin, end);
	}

	template<typename RowFilter>
	MOMO_DEPRECATED size_t RemoveRows(const RowFilter& rowFilter)
	{
		return Remove(rowFilter);
	}

	MOMO_DEPRECATED void RemoveRow(ConstRowReference rowRef)
	{
		Remove(rowRef);
	}

	MOMO_DEPRECATED void RemoveRow(size_t rowNumber, bool keepRowOrder = true)
	{
		Remove(rowNumber, keepRowOrder);
	}

	MOMO_DEPRECATED Row ExtractRow(ConstRowReference rowRef)
	{
		return Extract(rowRef);
	}

	MOMO_DEPRECATED Row ExtractRow(size_t rowNumber, bool keepRowOrder = true)
	{
		return Extract(rowNumber, keepRowOrder);
	}

	template<typename... Items>
	MOMO_DEPRECATED internal::EnableIf<(sizeof...(Items) > 1),
	ConstSelection> Select(Equality<Items>... equals) const
	{
		return Select(EmptyRowFilter(), equals...);
	}

	template<typename... Items>
	MOMO_DEPRECATED internal::EnableIf<(sizeof...(Items) > 1),
	Selection> Select(Equality<Items>... equals)
	{
		return Select(EmptyRowFilter(), equals...);
	}

	template<typename... Items>
	MOMO_DEPRECATED internal::EnableIf<(sizeof...(Items) > 1),
	size_t> SelectCount(Equality<Items>... equals) const
	{
		return Select(EmptyRowFilter(), equals...);
	}

private:
	RawMemPool pvCreateRawMemPool()
	{
		const ColumnList& columnList = GetColumnList();
		size_t size = std::minmax(columnList.GetTotalSize(), sizeof(void*)).second;
		return RawMemPool(typename RawMemPool::Params(size, columnList.GetAlignment()),
			MemManagerPtr(GetMemManager()));
	}

	template<typename Rows, typename RowFilter>
	void pvFill(const Rows& rows, const RowFilter& rowFilter)
	{
		const ColumnList& columnList = GetColumnList();
		if (std::is_same<RowFilter, EmptyRowFilter>::value)
			Reserve(rows.GetCount());
		try
		{
			for (ConstRowReference rowRef : rows)
			{
				if (!rowFilter(rowRef))
					continue;
				mRaws.Reserve(mRaws.GetCount() + 1);
				Raw* raw = pvImportRaw(columnList, rowRef.GetRaw());
				try
				{
					mIndexes.AddRaw(raw);
				}
				catch (...)
				{
					pvDestroyRaw(raw);
					throw;
				}
				mRaws.AddBackNogrow(raw);
			}
		}
		catch (...)
		{
			pvDestroyRaws();
			throw;
		}
		pvSetNumbers();
	}

	ConstRowReference pvMakeConstRowReference(Raw* raw) const noexcept
	{
		return ConstRowReferenceProxy(&GetColumnList(), raw,
			VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	RowReference pvMakeRowReference(Raw* raw) const noexcept
	{
		return RowReferenceProxy(&GetColumnList(), raw, VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	Iterator pvMakeIterator(size_t index) const noexcept
	{
		return IteratorProxy(&GetColumnList(), RawIterator(mRaws, index),
			VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	template<typename RawCreator>
	Raw* pvCreateRaw(RawCreator&& rawCreator)
	{
		Raw* raw = pvAllocateRaw();
		try
		{
			std::forward<RawCreator>(rawCreator)(raw);
		}
		catch (...)
		{
			mRawMemPool.Deallocate(raw);
			throw;
		}
		return raw;
	}

	Raw* pvCreateRaw()
	{
		auto rawCreator = [this] (Raw* raw) { GetColumnList().CreateRaw(GetMemManager(), raw); };
		return pvCreateRaw(rawCreator);
	}

	Raw* pvImportRaw(const ColumnList& srcColumnList, const Raw* srcRaw)
	{
		auto rawCreator = [this, &srcColumnList, srcRaw] (Raw* raw)
			{ GetColumnList().ImportRaw(GetMemManager(), srcColumnList, srcRaw, raw); };
		return pvCreateRaw(rawCreator);
	}

	Raw* pvAllocateRaw()
	{
		if (mCrew.GetFreeRaws() != nullptr)
			pvDeallocateFreeRaws();
		return mRawMemPool.template Allocate<Raw>();
	}

	void pvDestroyRaws() noexcept
	{
		if (mCrew.IsNull())
			return;
		pvDeallocateFreeRaws();
		for (Raw* raw : mRaws)
			pvDestroyRaw(raw);
	}

	void pvDestroyRaw(Raw* raw) noexcept
	{
		GetColumnList().DestroyRaw(&GetMemManager(), raw);
		mRawMemPool.Deallocate(raw);
	}

	void pvDeallocateFreeRaws() noexcept
	{
		void* headRaw = mCrew.GetFreeRaws().exchange(nullptr);
		while (headRaw != nullptr)
		{
			void* nextRaw = internal::MemCopyer::FromBuffer<void*>(headRaw);
			mRawMemPool.Deallocate(headRaw);
			headRaw = nextRaw;
		}
	}

	Row pvMakeRow(Raw* raw) noexcept
	{
		return RowProxy(&GetColumnList(), raw, &mCrew.GetFreeRaws());
	}

	template<typename... Items, typename... ItemArgs>
	Row pvNewRow(const Assignment<Items, ItemArgs>&... assigns)
	{
		Raw* raw = pvCreateRaw();
		try
		{
			pvFillRaw(raw, assigns...);
		}
		catch (...)
		{
			pvDestroyRaw(raw);
			throw;
		}
		return pvMakeRow(raw);
	}

	template<typename Item, typename ItemArg, typename... Items, typename... ItemArgs>
	void pvFillRaw(Raw* raw, const Assignment<Item, ItemArg>& assign,
		const Assignment<Items, ItemArgs>&... assigns)
	{
		const ColumnList& columnList = GetColumnList();
		size_t offset = columnList.GetOffset(assign.GetColumn());
		columnList.template Assign<Item>(raw, offset,
			std::forward<ItemArg>(assign.GetItemArg()));
		pvFillRaw(raw, assigns...);
	}

	void pvFillRaw(Raw* /*raw*/) noexcept
	{
	}

	void pvSetNumbers(size_t beginNumber = 0) noexcept
	{
		for (size_t i = beginNumber, count = mRaws.GetCount(); i < count; ++i)
			pvSetNumber(mRaws[i], i);
	}

	template<bool keepRowNumber = Settings::keepRowNumber>
	internal::EnableIf<keepRowNumber>
	pvSetNumber(Raw* raw, size_t number) noexcept
	{
		GetColumnList().SetNumber(raw, number);
	}

	template<bool keepRowNumber = Settings::keepRowNumber>
	internal::EnableIf<!keepRowNumber>
	pvSetNumber(Raw* /*raw*/, size_t /*number*/) noexcept
	{
	}

	template<bool keepRowNumber = Settings::keepRowNumber>
	internal::EnableIf<keepRowNumber,
	Raw*> pvExtractRaw(ConstRowReference rowRef)
	{
		return pvExtractRaw(rowRef.GetNumber());
	}

	template<bool keepRowNumber = Settings::keepRowNumber>
	internal::EnableIf<!keepRowNumber,
	Raw*> pvExtractRaw(ConstRowReference rowRef)
	{
		const Raw* raw = rowRef.GetRaw();
		size_t number = mRaws.GetCount() - 1;
		while (mRaws[number] != raw)
			--number;
		return pvExtractRaw(number);
	}

	Raw* pvExtractRaw(size_t number, bool keepOrder = true)
	{
		Raw* raw = mRaws[number];
		mIndexes.RemoveRaw(raw);
		if (keepOrder)
		{
			mRaws.Remove(number);
			pvSetNumbers(number);
		}
		else
		{
			mRaws[number] = mRaws.GetBackItem();
			mRaws.RemoveBack();
			if (number < mRaws.GetCount())
				pvSetNumber(mRaws[number], number);
		}
		++mCrew.GetChangeVersion();
		++mCrew.GetRemoveVersion();
		return raw;
	}

	template<typename Item, typename RItem>
	TryResult pvTryUpdate(ConstRowReference rowRef, const Column<Item>& column, RItem&& newItem)
	{
		const ColumnList& columnList = GetColumnList();
		MOMO_CHECK(&rowRef.GetColumnList() == &columnList);
		rowRef.GetRaw();	// check
		Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
		size_t offset = GetColumnList().GetOffset(column);
		auto itemAssigner = [&columnList, &newItem] (Raw* raw, size_t offset)
			{ columnList.template Assign<Item>(raw, offset, std::forward<RItem>(newItem)); };
		auto res = mIndexes.UpdateRaw(raw, offset, static_cast<const Item&>(newItem), itemAssigner);
		if (res.raw != nullptr)
			return { pvMakeRowReference(res.raw), res.uniqueHashIndex };
		++mCrew.GetChangeVersion();
		return { pvMakeRowReference(raw), UniqueHashIndex::empty };
	}

	template<typename RowIterator,
		bool keepRowNumber = Settings::keepRowNumber>
	internal::EnableIf<keepRowNumber>
	pvAssign(RowIterator begin, RowIterator end)
	{
		const ColumnList& columnList = GetColumnList();
		for (Raw* raw : mRaws)
			columnList.SetNumber(raw, invalidNumber);
		size_t count = 0;
		try
		{
			for (RowIterator iter = begin; iter != end; ++iter)
			{
				ConstRowReference rowRef = *iter;
				MOMO_CHECK(&rowRef.GetColumnList() == &columnList);
				Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
				if (columnList.GetNumber(raw) != invalidNumber)
					continue;
				columnList.SetNumber(raw, count);
				++count;
			}
		}
		catch (...)
		{
			pvSetNumbers();
			throw;
		}
		pvRemoveInvalidRaws();
		for (size_t i = 0; i < count; ++i)
		{
			Raw*& raw = mRaws[i];
			while (true)
			{
				size_t number = columnList.GetNumber(raw);
				if (number == i)
					break;
				std::swap(raw, mRaws[number]);
			}
		}
	}

	template<typename RowIterator,
		bool keepRowNumber = Settings::keepRowNumber>
	internal::EnableIf<!keepRowNumber>
	pvAssign(RowIterator begin, RowIterator end)
	{
		typedef HashMap<void*, size_t, HashTraits<void*>, MemManagerPtr,
			HashMapKeyValueTraits<void*, size_t, MemManagerPtr>,
			internal::NestedHashMapSettings> RawMap;
		RawMap rawMap((HashTraits<void*>()), MemManagerPtr(GetMemManager()));
		size_t count = 0;
		for (RowIterator iter = begin; iter != end; ++iter)
		{
			ConstRowReference rowRef = *iter;
			MOMO_CHECK(&rowRef.GetColumnList() == &GetColumnList());
			Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
			if (rawMap.Insert(raw, count).inserted)
				++count;
		}
		auto rawFilter = [&rawMap] (Raw* raw) { return rawMap.ContainsKey(raw); };
		pvFilterRaws(rawFilter);
		for (size_t i = 0; i < count; ++i)
		{
			Raw*& raw = mRaws[i];
			while (true)
			{
				size_t number = rawMap[raw];
				if (number == i)
					break;
				std::swap(raw, mRaws[number]);
			}
		}
	}

	template<typename RowIterator,
		bool keepRowNumber = Settings::keepRowNumber>
	internal::EnableIf<keepRowNumber>
	pvRemove(RowIterator begin, RowIterator end)
	{
		const ColumnList& columnList = GetColumnList();
		try
		{
			for (RowIterator iter = begin; iter != end; ++iter)
			{
				ConstRowReference rowRef = *iter;
				MOMO_CHECK(&rowRef.GetColumnList() == &columnList);
				columnList.SetNumber(ConstRowReferenceProxy::GetRaw(rowRef), invalidNumber);
			}
		}
		catch (...)
		{
			pvSetNumbers();
			throw;
		}
		pvRemoveInvalidRaws();
		pvSetNumbers();
	}

	template<typename RowIterator,
		bool keepRowNumber = Settings::keepRowNumber>
	internal::EnableIf<!keepRowNumber>
	pvRemove(RowIterator begin, RowIterator end)
	{
		typedef HashSet<void*, HashTraits<void*>, MemManagerPtr,
			HashSetItemTraits<void*, MemManagerPtr>, internal::NestedHashSetSettings> RawSet;
		RawSet rawSet((HashTraits<void*>()), MemManagerPtr(GetMemManager()));
		for (RowIterator iter = begin; iter != end; ++iter)
		{
			ConstRowReference rowRef = *iter;
			MOMO_CHECK(&rowRef.GetColumnList() == &GetColumnList());
			rawSet.Insert(ConstRowReferenceProxy::GetRaw(rowRef));
		}
		auto rawFilter = [&rawSet] (Raw* raw) { return !rawSet.ContainsKey(raw); };
		pvFilterRaws(rawFilter);
	}

	template<typename RowFilter,
		bool keepRowNumber = Settings::keepRowNumber>
	internal::EnableIf<keepRowNumber>
	pvRemove(const RowFilter& rowFilter)
	{
		const ColumnList& columnList = GetColumnList();
		try
		{
			for (Raw* raw : mRaws)
			{
				if (rowFilter(pvMakeConstRowReference(raw)))
					columnList.SetNumber(raw, invalidNumber);
			}
		}
		catch (...)
		{
			pvSetNumbers();
			throw;
		}
		pvRemoveInvalidRaws();
		pvSetNumbers();
	}

	template<typename RowFilter,
		bool keepRowNumber = Settings::keepRowNumber>
	internal::EnableIf<!keepRowNumber>
	pvRemove(const RowFilter& rowFilter)
	{
		typedef HashSet<void*, HashTraits<void*>, MemManagerPtr,
			HashSetItemTraits<void*, MemManagerPtr>, internal::NestedHashSetSettings> RawSet;
		RawSet rawSet((HashTraits<void*>()), MemManagerPtr(GetMemManager()));
		for (Raw* raw : mRaws)
		{
			if (rowFilter(pvMakeConstRowReference(raw)))
				rawSet.Insert(raw);
		}
		auto rawFilter = [&rawSet] (Raw* raw) { return !rawSet.ContainsKey(raw); };
		pvFilterRaws(rawFilter);
	}

	void pvRemoveInvalidRaws() noexcept
	{
		const ColumnList& columnList = GetColumnList();
		auto rawFilter = [&columnList] (Raw* raw)
			{ return columnList.GetNumber(raw) != invalidNumber; };
		pvFilterRaws(rawFilter);
	}

	template<typename RawFilter>
	void pvFilterRaws(RawFilter rawFilter) noexcept
	{
		mIndexes.FilterRaws(rawFilter);
		size_t count = 0;
		for (Raw* raw : mRaws)
		{
			if (!rawFilter(raw))
			{
				pvDestroyRaw(raw);
				continue;
			}
			mRaws[count] = raw;
			++count;
		}
		mRaws.RemoveBack(mRaws.GetCount() - count);
		++mCrew.GetChangeVersion();
		++mCrew.GetRemoveVersion();
	}

	template<typename... Items,
		size_t columnCount = sizeof...(Items)>
	std::array<size_t, columnCount> pvGetOffsets(const Column<Items>&... columns) const
	{
		const ColumnList& columnList = GetColumnList();
		return {{ columnList.GetOffset(columns)... }};
	}

	template<typename... Items,
		size_t columnCount = sizeof...(Items)>
	std::array<size_t, columnCount> pvGetOffsets(const Equality<Items>&... equals) const
	{
		return pvGetOffsets(equals.GetColumn()...);
	}

	template<size_t columnCount>
	void pvCheckImmutable(const std::array<size_t, columnCount>& offsets) const
	{
		for (size_t offset : offsets)
		{
			(void)offset;
			MOMO_CHECK(!GetColumnList().IsMutable(offset));
		}
	}

	Selection pvSelectEmpty() const
	{
		MemManager memManager = GetMemManager();
		return SelectionProxy(&GetColumnList(), typename SelectionProxy::Raws(std::move(memManager)),
			VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	template<typename Result, typename... Items, typename RowFilter>
	Result pvSelect(const Equalities<Items...>& equals, const RowFilter& rowFilter) const
	{
		return pvSelect<Result>(equals, rowFilter,
			typename internal::SequenceMaker<sizeof...(Items)>::Sequence());
	}

	template<typename Result, typename... Items, typename RowFilter, size_t... sequence>
	Result pvSelect(const Equalities<Items...>& equals, const RowFilter& rowFilter,
		internal::Sequence<sequence...>) const
	{
		return pvSelect<Result>(rowFilter, equals.template Get<sequence>()...);
	}

	template<typename Result, typename RowFilter, typename Item, typename... Items,
		size_t columnCount = 1 + sizeof...(Items)>
	internal::EnableIf<(columnCount > DataTraits::selectEqualityMaxCount),
	Result> pvSelect(const RowFilter& rowFilter, const Equality<Item>& equal,
		const Equality<Items>&... equals) const
	{
		auto newRowFilter = [&rowFilter, &equal] (ConstRowReference rowRef)
		{
			Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
			size_t offset = rowRef.GetColumnList().GetOffset(equal.GetColumn());
			const Item& item = ColumnList::template GetByOffset<const Item>(raw, offset);
			return DataTraits::IsEqual(item, equal.GetItem()) && rowFilter(rowRef);
		};
		return pvSelect<Result>(newRowFilter, equals...);
	}

	template<typename Result, typename RowFilter, typename... Items,
		size_t columnCount = sizeof...(Items)>
	internal::EnableIf<(0 < columnCount && columnCount <= DataTraits::selectEqualityMaxCount),
	Result> pvSelect(const RowFilter& rowFilter, const Equality<Items>&... equals) const
	{
		auto offsets = pvGetOffsets(equals...);
		auto sortedOffsets = Indexes::GetSortedOffsets(offsets);
		UniqueHashIndex uniqueHashIndex = mIndexes.GetFitUniqueHashIndex(sortedOffsets);
		if (uniqueHashIndex != UniqueHashIndex::empty)
		{
			return pvSelectRec<Result>(uniqueHashIndex, offsets.data(), rowFilter,
				OffsetItemTuple<>(), equals...);
		}
		MultiHashIndex multiHashIndex = mIndexes.GetFitMultiHashIndex(sortedOffsets);
		if (multiHashIndex != MultiHashIndex::empty)
		{
			return pvSelectRec<Result>(multiHashIndex, offsets.data(), rowFilter,
				OffsetItemTuple<>(), equals...);
		}
		auto newRowFilter = [&rowFilter, &offsets, &equals...] (ConstRowReference rowRef)
			{ return pvIsSatisfied(rowRef, offsets.data(), equals...) && rowFilter(rowRef); };
		return pvMakeSelection(mRaws, newRowFilter, static_cast<Result*>(nullptr));
	}

	template<typename Result, typename RowFilter>
	Result pvSelect(const RowFilter& rowFilter) const
	{
		return pvMakeSelection(mRaws, rowFilter, static_cast<Result*>(nullptr));
	}

	template<typename Item, typename... Items>
	static bool pvIsSatisfied(ConstRowReference rowRef, const size_t* offsets,
		const Equality<Item>& equal, const Equality<Items>&... equals)
	{
		Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
		const Item& item = ColumnList::template GetByOffset<const Item>(raw, *offsets);
		return DataTraits::IsEqual(item, equal.GetItem())
			&& pvIsSatisfied(rowRef, offsets + 1, equals...);
	}

	static bool pvIsSatisfied(ConstRowReference /*rowRef*/, const size_t* /*offsets*/) noexcept
	{
		return true;
	}

	template<typename Result, typename Index, typename RowFilter, typename Tuple, typename Item,
		typename... Items>
	Result pvSelectRec(Index index, const size_t* offsets, const RowFilter& rowFilter,
		Tuple&& tuple, const Equality<Item>& equal, const Equality<Items>&... equals) const
	{
		size_t offset = *offsets;
		if (mIndexes.ContainsOffset(index, offset))
		{
			auto newTuple = std::tuple_cat(std::move(tuple),
				std::make_tuple(std::pair<size_t, const Item&>(offset, equal.GetItem())));
			return pvSelectRec<Result>(index, offsets + 1, rowFilter, std::move(newTuple),
				equals...);
		}
		else
		{
			auto newRowFilter = [&rowFilter, offset, &equal] (ConstRowReference rowRef)
			{
				Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
				const Item& item = ColumnList::template GetByOffset<const Item>(raw, offset);
				return DataTraits::IsEqual(item, equal.GetItem()) && rowFilter(rowRef);
			};
			return pvSelectRec<Result>(index, offsets + 1, newRowFilter, std::move(tuple),
				equals...);
		}
	}

	template<typename Result, typename Index, typename RowFilter, typename Tuple>
	Result pvSelectRec(Index index, const size_t* /*offsets*/, const RowFilter& rowFilter,
		Tuple&& tuple) const
	{
		return pvMakeSelection(mIndexes.FindRaws(index, tuple, VersionKeeper(&mCrew.GetChangeVersion())),
			rowFilter, static_cast<Result*>(nullptr));
	}

#ifdef _MSC_VER	//?
	template<typename Result, typename Index, typename RowFilter>
	Result pvSelectRec(Index, const size_t*, const RowFilter&, OffsetItemTuple<>&&) const
	{
		throw std::exception();
	}
#endif

	template<typename Raws, typename RowFilter>
	Selection pvMakeSelection(const Raws& raws, const RowFilter& rowFilter, Selection*) const
	{
		MemManager memManager = GetMemManager();
		typename SelectionProxy::Raws selRaws(std::move(memManager));
		for (Raw* raw : raws)
		{
			if (rowFilter(pvMakeConstRowReference(raw)))
				selRaws.AddBack(raw);
		}
		return SelectionProxy(&GetColumnList(), std::move(selRaws),
			VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	template<typename Raws>
	Selection pvMakeSelection(const Raws& raws, const EmptyRowFilter& /*rowFilter*/, Selection*) const
	{
		MemManager memManager = GetMemManager();
		typename SelectionProxy::Raws selRaws(raws.GetBegin(), raws.GetEnd(), std::move(memManager));
		return SelectionProxy(&GetColumnList(), std::move(selRaws),
			VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	template<typename Raws, typename RowFilter>
	size_t pvMakeSelection(const Raws& raws, const RowFilter& rowFilter, size_t*) const
	{
		return static_cast<size_t>(std::count_if(raws.GetBegin(), raws.GetEnd(),
			[this, &rowFilter] (Raw* raw) { return rowFilter(pvMakeConstRowReference(raw)); }));
	}

	template<typename Raws>
	size_t pvMakeSelection(const Raws& raws, const EmptyRowFilter& /*rowFilter*/,
		size_t*) const noexcept
	{
		return internal::UIntMath<>::Dist(raws.GetBegin(), raws.GetEnd());
	}

	RowHashPointer pvFindByUniqueHash(UniqueHashIndex uniqueHashIndex, const Row& row) const
	{
		const ColumnList* columnList = &GetColumnList();
		MOMO_CHECK(uniqueHashIndex != UniqueHashIndex::empty);
		MOMO_CHECK(&row.GetColumnList() == columnList);
		auto raws = mIndexes.FindRaws(uniqueHashIndex, RowProxy::GetRaw(row),
			VersionKeeper(&mCrew.GetChangeVersion()));
		return RowHashPointerProxy(columnList, raws, VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	template<typename RowBoundsProxy, typename... Items, typename Index>
	RowBoundsProxy pvFindByHash(const Equalities<Items...>& equals, Index index) const
	{
		MOMO_STATIC_ASSERT(sizeof...(Items) > 0);
		return pvFindByHash<RowBoundsProxy>(equals, index,
			typename internal::SequenceMaker<sizeof...(Items)>::Sequence());
	}

	template<typename RowBoundsProxy, typename... Items, typename Index, size_t... sequence>
	RowBoundsProxy pvFindByHash(const Equalities<Items...>& equals, Index index,
		internal::Sequence<sequence...>) const
	{
		return pvFindByHash<RowBoundsProxy>(index, internal::Sequence<sequence...>(),
			equals.template Get<sequence>()...);
	}

	template<typename RowBoundsProxy, typename Index, typename... Items,
		size_t columnCount = sizeof...(Items)>
	RowBoundsProxy pvFindByHash(Index index, const Equality<Items>&... equals) const
	{
		return pvFindByHash<RowBoundsProxy>(index,
			typename internal::SequenceMaker<columnCount>::Sequence(), equals...);
	}

	template<typename RowBoundsProxy, typename Index, typename... Items, size_t... sequence>
	RowBoundsProxy pvFindByHash(Index index, internal::Sequence<sequence...>,
		const Equality<Items>&... equals) const
	{
		auto offsets = pvGetOffsets(equals...);
		Index trueIndex = mIndexes.GetTrueIndex(index, offsets);
		OffsetItemTuple<Items...> tuple{
			std::pair<size_t, const Items&>(offsets[sequence], equals.GetItem())... };
		auto raws = mIndexes.FindRaws(trueIndex, tuple, VersionKeeper(&mCrew.GetChangeVersion()));
		return RowBoundsProxy(&GetColumnList(), raws, VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	template<bool distinct, typename RowFilter, typename... Items>
	DataTable pvProject(ColumnList&& resColumnList, const RowFilter& rowFilter,
		const Column<Items>&... columns) const
	{
		DataTable resTable(std::move(resColumnList));
		auto offsets = pvGetOffsets(columns...);
		auto resOffsets = resTable.pvGetOffsets(columns...);
		if (distinct)
			resTable.mIndexes.template AddUniqueHashIndex<Items...>(resTable.mRaws, resOffsets);
		for (Raw* raw : mRaws)
		{
			if (!rowFilter(pvMakeConstRowReference(raw)))
				continue;
			resTable.mRaws.Reserve(resTable.mRaws.GetCount() + 1);
			Raw* resRaw = resTable.pvCreateRaw();
			resTable.mRaws.AddBackNogrow(resRaw);
			resTable.template pvAssign<void, Items...>(raw, offsets.data(),
				resRaw, resOffsets.data());
			if (distinct && resTable.mIndexes.AddRaw(resRaw).raw != nullptr)
			{
				resTable.mRaws.RemoveBack();
				resTable.pvDestroyRaw(resRaw);
			}
		}
		resTable.RemoveUniqueHashIndexes();
		resTable.pvSetNumbers();
		return resTable;
	}

	template<typename Void, typename Item, typename... Items>
	void pvAssign(Raw* srcRaw, const size_t* srcOffsets, Raw* dstRaw, const size_t* dstOffsets)
	{
		GetColumnList().template Assign<Item>(dstRaw, *dstOffsets,
			ColumnList::template GetByOffset<const Item>(srcRaw, *srcOffsets));
		pvAssign<void, Items...>(srcRaw, srcOffsets + 1, dstRaw, dstOffsets + 1);
	}

	template<typename Void>
	void pvAssign(Raw* /*srcRaw*/, const size_t* /*srcOffsets*/, Raw* /*dstRaw*/,
		const size_t* /*dstOffsets*/) noexcept
	{
	}

private:
	Crew mCrew;
	Raws mRaws;
	RawMemPool mRawMemPool;
	Indexes mIndexes;
};

template<typename TStruct,
	typename TDataTraits = DataTraits>
using DataTableStatic = DataTable<DataColumnListStatic<TStruct>, TDataTraits>;

template<typename TStruct,
	typename TDataTraits = DataTraits>
using DataTableNative = DataTable<DataColumnListNative<TStruct>, TDataTraits>;

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_DATA_TABLE
