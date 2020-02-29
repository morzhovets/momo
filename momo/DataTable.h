/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataTable.h

  namespace momo:
    class DataTraits
    class DataTable

\**********************************************************/

#pragma once

#include "DataColumn.h"
#include "DataRow.h"
#include "DataSelection.h"
#include "DataIndexes.h"

namespace momo
{

class DataTraits
{
public:
	typedef MemPoolParams<MemPoolConst::defaultBlockCount, 0> RawMemPoolParams;

	typedef HashBucketOpenDefault HashBucket;

	static const size_t selectEqualerMaxCount = 6;

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
		return (item1 < item2) ? -1 : (item1 == item2) ? 0 : 1;
	}

	template<typename Iterator, typename Comparer, typename MemManager>
	static void Sort(Iterator begin, size_t count, const Comparer& comparer,
		MemManager& /*memManager*/)
	{
		std::sort(begin, internal::UIntMath<>::Next(begin, count), comparer);	//?
	}
};

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
	using Equaler = internal::DataEqualer<Column<Item>, const Item&>;

	template<typename Item, typename ItemArg>
	using Assigner = internal::DataAssigner<Column<Item>, ItemArg>;

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
	typedef internal::DataRowBounds<ConstRowReference, RawBounds> ConstRowBounds;

public:
	typedef internal::DataRowIterator<RowReference, RawIterator> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::DataRowPointer<internal::DataRowBounds<RowReference,
		typename Indexes::UniqueHashRawBounds>> RowHashPointer;
	typedef typename RowHashPointer::ConstPointer ConstRowHashPointer;

	typedef internal::DataRowBounds<RowReference,
		typename Indexes::MultiHashRawBounds> RowHashBounds;
	typedef typename RowHashBounds::ConstBounds ConstRowHashBounds;

	template<typename Item>
	using ConstItemBounds = internal::DataConstItemBounds<Item, ConstRowBounds, Settings>;

	typedef typename Indexes::UniqueHashIndex UniqueHashIndex;
	typedef typename Indexes::MultiHashIndex MultiHashIndex;

	struct TryResult
	{
		RowReference rowReference;
		UniqueHashIndex uniqueHashIndex;
	};

	class UniqueIndexViolation : public std::runtime_error, public TryResult
	{
	public:
		explicit UniqueIndexViolation(TryResult tryResult)
			: std::runtime_error("Unique index violation"),
			TryResult(tryResult)
		{
		}
	};

private:
	typedef internal::VersionKeeper<Settings> VersionKeeper;

	static const size_t invalidNumber = SIZE_MAX;

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

	class Crew
	{
	private:
		typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

		typedef std::atomic<void*> FreeRaws;

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

		ColumnList& GetColumnList() noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->columnList;
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
	explicit DataTable(ColumnList&& columnList = ColumnList())
		: mCrew(std::move(columnList)),
		mRaws(MemManagerPtr(GetMemManager())),
		mRawMemPool(pvCreateRawMemPool()),
		mIndexes(GetMemManager())
	{
	}

	template<typename Item, typename... Items>
	explicit DataTable(const Column<Item>& column, const Column<Items>&... columns)
		: DataTable(ColumnList(column, columns...))
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
		pvFreeRaws();
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
	MOMO_FRIENDS_BEGIN_END(const DataTable&, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(DataTable&, Iterator)

	const ColumnList& GetColumnList() const noexcept
	{
		return mCrew.GetColumnList();
	}

	const MemManager& GetMemManager() const noexcept
	{
		return mCrew.GetColumnList().GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mCrew.GetColumnList().GetMemManager();
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
		pvFreeRaws();
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
		size_t offset = columnList.GetOffset(column);
		RawBounds rawBounds(RawIterator(mRaws, 0), GetCount());
		return ConstItemBounds<Item>(offset, ConstRowBoundsProxy(&columnList, rawBounds,
			VersionKeeper(&mCrew.GetRemoveVersion())));
	}

	Row NewRow()
	{
		return pvMakeRow(pvCreateRaw());
	}

	template<typename Item, typename ItemArg, typename... Assigners>
	Row NewRow(Assigner<Item, ItemArg> assigner, Assigners... assigners)
	{
		return pvNewRow(assigner, assigners...);
	}

	Row NewRow(const Row& row)
	{
		MOMO_CHECK(&row.GetColumnList() == &GetColumnList());
		return pvMakeRow(pvCopyRaw(row.GetRaw()));
	}

	Row NewRow(ConstRowReference rowRef)
	{
		MOMO_CHECK(&rowRef.GetColumnList() == &GetColumnList());
		return pvMakeRow(pvCopyRaw(rowRef.GetRaw()));
	}

	RowReference AddRow(Row&& row)
	{
		TryResult res = TryAddRow(std::move(row));
		if (res.uniqueHashIndex != UniqueHashIndex::empty)
			throw UniqueIndexViolation(res);
		return res.rowReference;
	}

	template<typename Item, typename ItemArg, typename... Assigners>
	RowReference AddRow(Assigner<Item, ItemArg> assigner, Assigners... assigners)
	{
		return AddRow(pvNewRow(assigner, assigners...));
	}

	TryResult TryAddRow(Row&& row)
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

	template<typename Item, typename ItemArg, typename... Assigners>
	TryResult TryAddRow(Assigner<Item, ItemArg> assigner, Assigners... assigners)
	{
		return TryAddRow(pvNewRow(assigner, assigners...));
	}

	RowReference InsertRow(size_t rowNumber, Row&& row)
	{
		TryResult res = TryInsertRow(rowNumber, std::move(row));
		if (res.uniqueHashIndex != UniqueHashIndex::empty)
			throw UniqueIndexViolation(res);
		return res.rowReference;
	}

	template<typename Item, typename ItemArg, typename... Assigners>
	RowReference InsertRow(size_t rowNumber, Assigner<Item, ItemArg> assigner,
		Assigners... assigners)
	{
		return InsertRow(rowNumber, pvNewRow(assigner, assigners...));
	}

	TryResult TryInsertRow(size_t rowNumber, Row&& row)
	{
		MOMO_CHECK(rowNumber <= GetCount());
		TryResult res = TryAddRow(std::move(row));
		if (res.uniqueHashIndex == UniqueHashIndex::empty)
		{
			std::rotate(mRaws.GetBegin() + rowNumber, std::prev(mRaws.GetEnd()), mRaws.GetEnd());
			pvSetNumbers(rowNumber);
		}
		return res;
	}

	template<typename Item, typename ItemArg, typename... Assigners>
	TryResult TryInsertRow(size_t rowNumber, Assigner<Item, ItemArg> assigner,
		Assigners... assigners)
	{
		return TryInsertRow(rowNumber, pvNewRow(assigner, assigners...));
	}

	void RemoveRow(ConstRowReference rowRef)
	{
		MOMO_CHECK(&rowRef.GetColumnList() == &GetColumnList());
		pvFreeRaw(pvExtractRaw(rowRef));
	}

	void RemoveRow(size_t rowNumber, bool keepRowOrder = true)
	{
		MOMO_CHECK(rowNumber < GetCount());
		pvFreeRaw(pvExtractRaw(rowNumber, keepRowOrder));
	}

	Row ExtractRow(ConstRowReference rowRef)
	{
		MOMO_CHECK(&rowRef.GetColumnList() == &GetColumnList());
		return pvMakeRow(pvExtractRaw(rowRef));
	}

	Row ExtractRow(size_t rowNumber, bool keepRowOrder = true)
	{
		MOMO_CHECK(rowNumber < GetCount());
		return pvMakeRow(pvExtractRaw(rowNumber, keepRowOrder));
	}

	RowReference UpdateRow(size_t rowNumber, Row&& row)
	{
		TryResult res = TryUpdateRow(rowNumber, std::move(row));
		if (res.uniqueHashIndex != UniqueHashIndex::empty)
			throw UniqueIndexViolation(res);
		return res.rowReference;
	}

	template<typename Item>
	RowReference UpdateRow(ConstRowReference rowRef, const Column<Item>& column, Item&& newItem)
	{
		TryResult res = TryUpdateRow(rowRef, column, std::move(newItem));
		if (res.uniqueHashIndex != UniqueHashIndex::empty)
			throw UniqueIndexViolation(res);
		return res.rowReference;
	}

	TryResult TryUpdateRow(size_t rowNumber, Row&& row)
	{
		MOMO_CHECK(rowNumber < GetCount());
		Raw*& raw = mRaws[rowNumber];
		auto res = mIndexes.UpdateRaw(raw, row.GetRaw());
		if (res.raw != nullptr)
			return { pvMakeRowReference(res.raw), res.uniqueHashIndex };
		pvFreeRaw(raw);
		raw = RowProxy::ExtractRaw(row);
		pvSetNumber(raw, rowNumber);
		++mCrew.GetChangeVersion();
		++mCrew.GetRemoveVersion();
		return { pvMakeRowReference(raw), UniqueHashIndex::empty };
	}

	template<typename Item>
	TryResult TryUpdateRow(ConstRowReference rowRef, const Column<Item>& column, Item&& newItem)
	{
		const ColumnList& columnList = GetColumnList();
		MOMO_CHECK(&rowRef.GetColumnList() == &columnList);
		rowRef.GetRaw();	// check
		Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
		size_t offset = GetColumnList().GetOffset(column);
		auto assigner = [&columnList, raw, offset, &newItem] ()
			{ columnList.template Assign<Item>(raw, offset, std::move(newItem)); };
		auto res = mIndexes.UpdateRaw(raw, offset, static_cast<const Item&>(newItem), assigner);
		if (res.raw != nullptr)
			return { pvMakeRowReference(res.raw), res.uniqueHashIndex };
		++mCrew.GetChangeVersion();
		return { pvMakeRowReference(raw), UniqueHashIndex::empty };
	}

	template<typename RowIterator>
	void AssignRows(RowIterator begin, RowIterator end)
	{
		const ColumnList& columnList = GetColumnList();
		for (Raw* raw : mRaws)
			pvSetNumber(raw, invalidNumber);
		try
		{
			size_t number = 0;
			for (RowIterator iter = begin; iter != end; ++iter)
			{
				MOMO_CHECK(&iter->GetColumnList() == &GetColumnList());
				ConstRowReference rowRef = *iter;
				Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
				if (columnList.GetNumber(raw) != invalidNumber)
					continue;
				pvSetNumber(raw, number);
				++number;
			}
		}
		catch (...)
		{
			pvSetNumbers();
			throw;
		}
		pvRemoveInvalidRaws();
		for (size_t i = 0, count = mRaws.GetCount(); i < count; ++i)
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

	template<typename RowIterator>
	void RemoveRows(RowIterator begin, RowIterator end)
	{
		try
		{
			for (RowIterator iter = begin; iter != end; ++iter)
			{
				MOMO_CHECK(&iter->GetColumnList() == &GetColumnList());
				ConstRowReference rowRef = *iter;
				pvSetNumber(ConstRowReferenceProxy::GetRaw(rowRef), invalidNumber);
			}
		}
		catch (...)
		{
			pvSetNumbers();
			throw;
		}
		pvRemoveInvalidRaws();
	}

	template<typename RowFilter>
	void RemoveRows(const RowFilter& rowFilter)
	{
		auto newRowFilter = [&rowFilter] (ConstRowReference rowRef)
			{ return !rowFilter(rowRef); };
		FilterRows(newRowFilter);
	}

	template<typename RowFilter>
	void FilterRows(const RowFilter& rowFilter)
	{
		try
		{
			for (Raw* raw : mRaws)
			{
				if (!rowFilter(pvMakeConstRowReference(raw)))
					pvSetNumber(raw, invalidNumber);
			}
		}
		catch (...)
		{
			pvSetNumbers();
			throw;
		}
		pvRemoveInvalidRaws();
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
		return mIndexes.template AddUniqueHashIndex<Item, Items...>(mRaws, offsets);
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

	template<typename... Items>
	ConstSelection Select(Equaler<Items>... equalers) const
	{
		return pvSelect<Selection>(EmptyRowFilter(), equalers...);
	}

	template<typename RowFilter, typename... Items,
		typename = decltype(std::declval<const RowFilter&>()(std::declval<ConstRowReference>()))>
	ConstSelection Select(const RowFilter& rowFilter, Equaler<Items>... equalers) const
	{
		return pvSelect<Selection>(rowFilter, equalers...);
	}

	template<typename... Items>
	Selection Select(Equaler<Items>... equalers)
	{
		return pvSelect<Selection>(EmptyRowFilter(), equalers...);
	}

	template<typename RowFilter, typename... Items,
		typename = decltype(std::declval<const RowFilter&>()(std::declval<ConstRowReference>()))>
	Selection Select(const RowFilter& rowFilter, Equaler<Items>... equalers)
	{
		return pvSelect<Selection>(rowFilter, equalers...);
	}

	template<typename... Items>
	size_t SelectCount(Equaler<Items>... equalers) const
	{
		return pvSelect<size_t>(EmptyRowFilter(), equalers...);
	}

	template<typename RowFilter, typename... Items,
		typename = decltype(std::declval<const RowFilter&>()(std::declval<ConstRowReference>()))>
	size_t SelectCount(const RowFilter& rowFilter, Equaler<Items>... equalers) const
	{
		return pvSelect<size_t>(rowFilter, equalers...);
	}

	ConstRowHashPointer FindByUniqueHash(UniqueHashIndex uniqueHashIndex, const Row& row) const
	{
		return pvFindByUniqueHash(uniqueHashIndex, row);
	}

	RowHashPointer FindByUniqueHash(UniqueHashIndex uniqueHashIndex, const Row& row)
	{
		return pvFindByUniqueHash(uniqueHashIndex, row);
	}

	template<typename Item, typename... Items>
	ConstRowHashPointer FindByUniqueHash(UniqueHashIndex uniqueHashIndex, Equaler<Item> equaler,
		Equaler<Items>... equalers) const
	{
		return pvFindByHash<RowHashPointerProxy>(uniqueHashIndex, equaler, equalers...);
	}

	template<typename Item, typename... Items>
	RowHashPointer FindByUniqueHash(UniqueHashIndex uniqueHashIndex, Equaler<Item> equaler,
		Equaler<Items>... equalers)
	{
		return pvFindByHash<RowHashPointerProxy>(uniqueHashIndex, equaler, equalers...);
	}

	template<typename Item, typename... Items>
	ConstRowHashBounds FindByMultiHash(MultiHashIndex multiHashIndex, Equaler<Item> equaler,
		Equaler<Items>... equalers) const
	{
		return pvFindByHash<RowHashBoundsProxy>(multiHashIndex, equaler, equalers...);
	}

	template<typename Item, typename... Items>
	RowHashBounds FindByMultiHash(MultiHashIndex multiHashIndex, Equaler<Item> equaler,
		Equaler<Items>... equalers)
	{
		return pvFindByHash<RowHashBoundsProxy>(multiHashIndex, equaler, equalers...);
	}

	template<typename Item, typename... Items>
	DataTable Project(const Column<Item>& column, const Column<Items>&... columns) const
	{
		return pvProject<false>(EmptyRowFilter(), column, columns...);
	}

	template<typename RowFilter, typename Item, typename... Items,
		typename = decltype(std::declval<const RowFilter&>()(std::declval<ConstRowReference>()))>
	DataTable Project(const RowFilter& rowFilter, const Column<Item>& column,
		const Column<Items>&... columns) const
	{
		return pvProject<false>(rowFilter, column, columns...);
	}

	template<typename Item, typename... Items>
	DataTable ProjectDistinct(const Column<Item>& column, const Column<Items>&... columns) const
	{
		return pvProject<true>(EmptyRowFilter(), column, columns...);
	}

	template<typename RowFilter, typename Item, typename... Items,
		typename = decltype(std::declval<const RowFilter&>()(std::declval<ConstRowReference>()))>
	DataTable ProjectDistinct(const RowFilter& rowFilter, const Column<Item>& column,
		const Column<Items>&... columns) const
	{
		return pvProject<true>(rowFilter, column, columns...);
	}

	RowReference MakeMutableReference(ConstRowReference rowRef)
	{
		MOMO_CHECK(&rowRef.GetColumnList() == &GetColumnList());
		rowRef.GetRaw();	// check
		Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
		return pvMakeRowReference(raw);
	}

private:
	RawMemPool pvCreateRawMemPool()
	{
		const ColumnList& columnList = GetColumnList();
		size_t size = std::minmax(columnList.GetTotalSize(), sizeof(void*)).second;
		size_t alignment = std::minmax(columnList.GetAlignment(),
			size_t{internal::AlignmentOf<void*>::value}).second;
		return RawMemPool(typename RawMemPool::Params(size, alignment),
			MemManagerPtr(GetMemManager()));
	}

	template<typename Rows, typename RowFilter>
	void pvFill(const Rows& rows, const RowFilter& rowFilter)
	{
		if (std::is_same<RowFilter, EmptyRowFilter>::value)
			Reserve(rows.GetCount());
		try
		{
			for (ConstRowReference rowRef : rows)
			{
				if (!rowFilter(rowRef))
					continue;
				mRaws.Reserve(mRaws.GetCount() + 1);
				Raw* raw = pvCopyRaw(rowRef.GetRaw());
				try
				{
					mIndexes.AddRaw(raw);
				}
				catch (...)
				{
					pvFreeRaw(raw);
					throw;
				}
				mRaws.AddBackNogrow(raw);
			}
		}
		catch (...)
		{
			pvFreeRaws();
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

	Raw* pvCreateRaw()
	{
		Raw* raw = mRawMemPool.template Allocate<Raw>();
		try
		{
			mCrew.GetColumnList().CreateRaw(raw);
		}
		catch (...)
		{
			mRawMemPool.Deallocate(raw);
			throw;
		}
		return raw;
	}

	Raw* pvCopyRaw(const Raw* raw)
	{
		Raw* resRaw = mRawMemPool.template Allocate<Raw>();
		try
		{
			mCrew.GetColumnList().CopyRaw(raw, resRaw);
		}
		catch (...)
		{
			mRawMemPool.Deallocate(resRaw);
			throw;
		}
		return resRaw;
	}

	void pvFreeRaws() noexcept
	{
		if (mCrew.IsNull())
			return;
		pvFreeNewRaws();
		for (Raw* raw : mRaws)
			pvFreeRaw(raw);
	}

	void pvFreeRaw(Raw* raw) noexcept
	{
		mCrew.GetColumnList().DestroyRaw(raw);
		mRawMemPool.Deallocate(raw);
	}

	void pvFreeNewRaws() noexcept
	{
		void* headRaw = mCrew.GetFreeRaws().exchange(nullptr);
		while (headRaw != nullptr)
		{
			void* nextRaw = *static_cast<void**>(headRaw);	//?
			mRawMemPool.Deallocate(headRaw);
			headRaw = nextRaw;
		}
	}

	Row pvMakeRow(Raw* raw) noexcept
	{
		pvFreeNewRaws();
		return RowProxy(&GetColumnList(), raw, &mCrew.GetFreeRaws());
	}

	template<typename Item, typename ItemArg, typename... Assigners>
	Row pvNewRow(const Assigner<Item, ItemArg>& assigner, const Assigners&... assigners)
	{
		Raw* raw = pvCreateRaw();
		try
		{
			pvFillRaw(raw, assigner, assigners...);
		}
		catch (...)
		{
			pvFreeRaw(raw);
			throw;
		}
		return pvMakeRow(raw);
	}

	template<typename Item, typename ItemArg, typename... Assigners>
	void pvFillRaw(Raw* raw, const Assigner<Item, ItemArg>& assigner, const Assigners&... assigners)
	{
		const ColumnList& columnList = GetColumnList();
		size_t offset = columnList.GetOffset(assigner.GetColumn());
		columnList.template Assign<Item>(raw, offset,
			std::forward<ItemArg>(assigner.GetItemArg()));
		pvFillRaw(raw, assigners...);
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
	internal::EnableIf<keepRowNumber> pvSetNumber(Raw* raw, size_t number) noexcept
	{
		GetColumnList().SetNumber(raw, number);
	}

	template<bool keepRowNumber = Settings::keepRowNumber>
	internal::EnableIf<!keepRowNumber> pvSetNumber(Raw* /*raw*/, size_t /*number*/) noexcept
	{
	}

	template<bool keepRowNumber = Settings::keepRowNumber>
	internal::EnableIf<keepRowNumber, Raw*> pvExtractRaw(ConstRowReference rowRef)
	{
		return pvExtractRaw(rowRef.GetNumber());
	}

	template<bool keepRowNumber = Settings::keepRowNumber>
	internal::EnableIf<!keepRowNumber, Raw*> pvExtractRaw(ConstRowReference rowRef)
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

	void pvRemoveInvalidRaws() noexcept
	{
		auto rawFilter = [this] (Raw* raw)
			{ return GetColumnList().GetNumber(raw) != invalidNumber; };
		mIndexes.FilterRaws(rawFilter);
		size_t number = 0;
		for (Raw* raw : mRaws)
		{
			if (!rawFilter(raw))
			{
				pvFreeRaw(raw);
				continue;
			}
			mRaws[number] = raw;
			++number;
		}
		mRaws.RemoveBack(mRaws.GetCount() - number);
		pvSetNumbers();
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
	std::array<size_t, columnCount> pvGetOffsets(const Equaler<Items>&... equalers) const
	{
		return pvGetOffsets(equalers.GetColumn()...);
	}

	template<size_t columnCount>
	void pvCheckImmutable(const std::array<size_t, columnCount>& offsets) const
	{
		const ColumnList& columnList = GetColumnList();
		for (size_t offset : offsets)
		{
			(void)offset;
			MOMO_CHECK(!columnList.IsMutable(offset));
		}
	}

	Selection pvSelectEmpty() const
	{
		MemManager memManager = GetMemManager();
		return SelectionProxy(&GetColumnList(), typename SelectionProxy::Raws(std::move(memManager)),
			VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	template<typename Result, typename RowFilter, typename Item, typename... Items,
		size_t columnCount = sizeof...(Items) + 1,
		typename = internal::EnableIf<(columnCount > DataTraits::selectEqualerMaxCount)>>
	Result pvSelect(const RowFilter& rowFilter, const Equaler<Item>& equaler,
		const Equaler<Items>&... equalers) const
	{
		auto newRowFilter = [&rowFilter, &equaler] (ConstRowReference rowRef)
		{
			Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
			size_t offset = rowRef.GetColumnList().GetOffset(equaler.GetColumn());
			const Item& item = ColumnList::template GetByOffset<const Item>(raw, offset);
			return DataTraits::IsEqual(item, equaler.GetItemArg()) && rowFilter(rowRef);
		};
		return pvSelect<Result>(newRowFilter, equalers...);
	}

	template<typename Result, typename RowFilter, typename... Items,
		size_t columnCount = sizeof...(Items),
		typename = internal::EnableIf<(0 < columnCount
			&& columnCount <= DataTraits::selectEqualerMaxCount)>>
	Result pvSelect(const RowFilter& rowFilter, const Equaler<Items>&... equalers) const
	{
		auto offsets = pvGetOffsets(equalers...);
		auto sortedOffsets = Indexes::GetSortedOffsets(offsets);
		UniqueHashIndex uniqueHashIndex = mIndexes.GetFitUniqueHashIndex(sortedOffsets);
		if (uniqueHashIndex != UniqueHashIndex::empty)
		{
			return pvSelectRec<Result>(uniqueHashIndex, offsets.data(), rowFilter,
				OffsetItemTuple<>(), equalers...);
		}
		MultiHashIndex multiHashIndex = mIndexes.GetFitMultiHashIndex(sortedOffsets);
		if (multiHashIndex != MultiHashIndex::empty)
		{
			return pvSelectRec<Result>(multiHashIndex, offsets.data(), rowFilter,
				OffsetItemTuple<>(), equalers...);
		}
		auto newRowFilter = [&offsets, &rowFilter, &equalers...] (ConstRowReference rowRef)
			{ return pvIsSatisfied(rowRef, offsets.data(), equalers...) && rowFilter(rowRef); };
		return pvMakeSelection(mRaws, newRowFilter, static_cast<Result*>(nullptr));
	}

	template<typename Result, typename RowFilter>
	Result pvSelect(const RowFilter& rowFilter) const
	{
		return pvMakeSelection(mRaws, rowFilter, static_cast<Result*>(nullptr));
	}

	template<typename Item, typename... Items>
	static bool pvIsSatisfied(ConstRowReference rowRef, const size_t* offsets,
		const Equaler<Item>& equaler, const Equaler<Items>&... equalers)
	{
		Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
		const Item& item = ColumnList::template GetByOffset<const Item>(raw, *offsets);
		return DataTraits::IsEqual(item, equaler.GetItemArg())
			&& pvIsSatisfied(rowRef, offsets + 1, equalers...);
	}

	static bool pvIsSatisfied(ConstRowReference /*rowRef*/, const size_t* /*offsets*/) noexcept
	{
		return true;
	}

	template<typename Result, typename Index, typename RowFilter, typename Tuple, typename Item,
		typename... Items>
	Result pvSelectRec(Index index, const size_t* offsets, const RowFilter& rowFilter,
		Tuple&& tuple, const Equaler<Item>& equaler, const Equaler<Items>&... equalers) const
	{
		size_t offset = *offsets;
		if (mIndexes.ContainsOffset(index, offset))
		{
			auto newTuple = std::tuple_cat(std::move(tuple),
				std::make_tuple(std::pair<size_t, const Item&>(offset, equaler.GetItemArg())));
			return pvSelectRec<Result>(index, offsets + 1, rowFilter, std::move(newTuple),
				equalers...);
		}
		else
		{
			auto newRowFilter = [&rowFilter, offset, &equaler] (ConstRowReference rowRef)
			{
				Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
				const Item& item = ColumnList::template GetByOffset<const Item>(raw, offset);
				return DataTraits::IsEqual(item, equaler.GetItemArg()) && rowFilter(rowRef);
			};
			return pvSelectRec<Result>(index, offsets + 1, newRowFilter, std::move(tuple),
				equalers...);
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

	template<typename RowBoundsProxy, typename Index, typename... Items>
	RowBoundsProxy pvFindByHash(Index index, const Equaler<Items>&... equalers) const
	{
		auto offsets = pvGetOffsets(equalers...);
		Index trueIndex = mIndexes.GetTrueIndex(index, offsets);
		return pvFindByHashRec<RowBoundsProxy>(trueIndex, offsets.data(),
			OffsetItemTuple<>(), equalers...);
	}

	template<typename RowBoundsProxy, typename Index, typename Tuple, typename Item, typename... Items>
	RowBoundsProxy pvFindByHashRec(Index index, const size_t* offsets, Tuple&& tuple,
		const Equaler<Item>& equaler, const Equaler<Items>&... equalers) const
	{
		auto newTuple = std::tuple_cat(std::move(tuple),
			std::make_tuple(std::pair<size_t, const Item&>(*offsets, equaler.GetItemArg())));
		return pvFindByHashRec<RowBoundsProxy>(index, offsets + 1, std::move(newTuple), equalers...);
	}

	template<typename RowBoundsProxy, typename Index, typename Tuple>
	RowBoundsProxy pvFindByHashRec(Index index, const size_t* /*offsets*/, Tuple&& tuple) const
	{
		return RowBoundsProxy(&GetColumnList(),
			mIndexes.FindRaws(index, tuple, VersionKeeper(&mCrew.GetChangeVersion())),
			VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	template<bool distinct, typename RowFilter, typename... Items>
	DataTable pvProject(const RowFilter& rowFilter, const Column<Items>&... columns) const
	{
		MemManager memManager = GetMemManager();
		ColumnList resColumnList(std::move(memManager));
		resColumnList.Add(columns...);
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
				resTable.pvFreeRaw(resRaw);
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

} // namespace momo
