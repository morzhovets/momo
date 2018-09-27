/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/DataTable.h

  namespace momo::experimental:
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

namespace experimental
{

class DataTraits
{
public:
	typedef MemPoolParams<MemPoolConst::defaultBlockCount, 0> RawMemPoolParams;

	typedef HashBucketOpenDefault HashBucket;

public:
	template<typename Item>
	static size_t GetHashCode(const Item& item)
	{
		return HashCoder<Item>()(item);
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

	template<typename Iterator, typename Comparer>
	static void Sort(Iterator begin, Iterator end, const Comparer& comparer)
	{
		std::stable_sort(begin, end, comparer);
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
	using Equaler = DataOperator<DataOperatorType::equal, Column<Item>, const Item&>;

	template<typename Item, typename ItemArg>
	using Assigner = DataOperator<DataOperatorType::assign, Column<Item>, ItemArg>;

	typedef internal::DataRow<ColumnList> Row;
	typedef internal::DataRowReference<ColumnList> RowReference;
	typedef typename RowReference::ConstReference ConstRowReference;

	typedef internal::DataSelection<RowReference, DataTraits> Selection;
	typedef typename Selection::ConstSelection ConstSelection;

private:
	typedef momo::internal::MemManagerPtr<MemManager> MemManagerPtr;

	typedef Array<Raw*, MemManagerPtr, ArrayItemTraits<Raw*, MemManagerPtr>,
		momo::internal::NestedArraySettings<typename Settings::TableRawsSettings>> Raws;

	typedef internal::DataIndexes<ColumnList, DataTraits> Indexes;

	typedef typename Indexes::UniqueIndexViolation UniqueIndexViolation;
	typedef typename Indexes::UniqueHash UniqueHashIndex;
	typedef typename Indexes::MultiHash MultiHashIndex;

	typedef internal::DataRawIterator<Raws, Settings> RawIterator;

	typedef momo::internal::ArrayBounds<RawIterator> RawBounds;
	typedef internal::DataRowBounds<ConstRowReference, RawBounds> ConstRowBounds;

public:
	typedef internal::DataRowIterator<RowReference, RawIterator> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::DataRowPointer<internal::DataRowBounds<RowReference,
		typename UniqueHashIndex::RawBounds>> RowHashPointer;
	typedef typename RowHashPointer::ConstPointer ConstRowHashPointer;

	typedef internal::DataRowBounds<RowReference,
		typename MultiHashIndex::RawBounds> RowHashBounds;
	typedef typename RowHashBounds::ConstBounds ConstRowHashBounds;

	template<typename Item>
	using ConstItemBounds = internal::DataConstItemBounds<Item, ConstRowBounds, Settings>;

	typedef const void* IndexHandle;

	struct TryResult
	{
		RowReference rowReference;
		IndexHandle uniqueHashIndex;
	};

private:
	typedef momo::internal::VersionKeeper<Settings> VersionKeeper;

	typedef momo::internal::BoolConstant<Settings::keepRowNumber> KeepRowNumber;

	static const size_t invalidRowNumber = SIZE_MAX;

	typedef MemPool<typename DataTraits::RawMemPoolParams, MemManagerPtr,
		momo::internal::NestedMemPoolSettings> RawMemPool;

	template<typename... Items>
	using OffsetItemTuple = typename Indexes::template OffsetItemTuple<Items...>;

	struct EmptyRowFilter
	{
		bool operator()(ConstRowReference /*rowRef*/) const MOMO_NOEXCEPT
		{
			return true;
		}
	};

	class Crew
	{
	private:
		typedef std::atomic<Raw*> FreeRaws;

		struct Data
		{
			ColumnList columnList;
			size_t changeVersion;
			size_t removeVersion;
			FreeRaws freeRaws;
		};

	public:
		explicit Crew(ColumnList&& columnList)
		{
			mData = columnList.GetMemManager().template Allocate<Data>(sizeof(Data));
			new(&mData->columnList) ColumnList(std::move(columnList));
			mData->changeVersion = 0;
			mData->removeVersion = 0;
			new(&mData->freeRaws) FreeRaws(nullptr);
		}

		Crew(Crew&& crew) MOMO_NOEXCEPT
			: mData(nullptr)
		{
			Swap(crew);
		}

		Crew(const Crew&) = delete;

		~Crew() MOMO_NOEXCEPT
		{
			if (!IsNull())
			{
				ColumnList columnList = std::move(mData->columnList);
				mData->columnList.~ColumnList();
				mData->freeRaws.~FreeRaws();
				columnList.GetMemManager().Deallocate(mData, sizeof(Data));
			}
		}

		Crew& operator=(const Crew&) = delete;

		void Swap(Crew& crew) MOMO_NOEXCEPT
		{
			std::swap(mData, crew.mData);
		}

		bool IsNull() const MOMO_NOEXCEPT
		{
			return mData == nullptr;
		}

		const ColumnList& GetColumnList() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!IsNull());
			return mData->columnList;
		}

		ColumnList& GetColumnList() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!IsNull());
			return mData->columnList;
		}

		const size_t& GetChangeVersion() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!IsNull());
			return mData->changeVersion;
		}

		size_t& GetChangeVersion() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!IsNull());
			return mData->changeVersion;
		}

		const size_t& GetRemoveVersion() const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!IsNull());
			return mData->removeVersion;
		}

		size_t& GetRemoveVersion() MOMO_NOEXCEPT
		{
			MOMO_ASSERT(!IsNull());
			return mData->removeVersion;
		}

		FreeRaws& GetFreeRaws() MOMO_NOEXCEPT
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
	};

	struct RowReferenceProxy : public RowReference
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(RowReference)
		MOMO_DECLARE_PROXY_FUNCTION(RowReference, GetRaw, Raw*)
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

	template<typename Item, typename... Items>
	explicit DataTable(MemManager&& memManager, const Column<Item>& column,
		const Column<Items>&... columns)
		: DataTable(ColumnList(std::move(memManager), column, columns...))
	{
	}

	DataTable(DataTable&& table) MOMO_NOEXCEPT
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
	DataTable(const DataTable& table, const RowFilter& rowFilter)
		: DataTable(ColumnList(table.GetColumnList()))
	{
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

	~DataTable() MOMO_NOEXCEPT
	{
		pvFreeRaws();
	}

	DataTable& operator=(DataTable&& table) MOMO_NOEXCEPT
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

	void Swap(DataTable& table) MOMO_NOEXCEPT
	{
		mCrew.Swap(table.mCrew);
		mRaws.Swap(table.mRaws);
		mRawMemPool.Swap(table.mRawMemPool);
		mIndexes.Swap(table.mIndexes);
	}

	ConstIterator GetBegin() const MOMO_NOEXCEPT
	{
		return pvMakeIterator(0);
	}

	Iterator GetBegin() MOMO_NOEXCEPT
	{
		return pvMakeIterator(0);
	}

	ConstIterator GetEnd() const MOMO_NOEXCEPT
	{
		return pvMakeIterator(GetCount());
	}

	Iterator GetEnd() MOMO_NOEXCEPT
	{
		return pvMakeIterator(GetCount());
	}

	MOMO_FRIEND_SWAP(DataTable)
	MOMO_FRIENDS_BEGIN_END(const DataTable&, ConstIterator)
	MOMO_FRIENDS_BEGIN_END(DataTable&, Iterator)

	const ColumnList& GetColumnList() const MOMO_NOEXCEPT
	{
		return mCrew.GetColumnList();
	}

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return mCrew.GetColumnList().GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return mCrew.GetColumnList().GetMemManager();
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
		return pvMakeRow(raw);
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
		MOMO_CHECK(&row.GetColumnList() == &GetColumnList());
		mRaws.Reserve(mRaws.GetCount() + 1);
		mIndexes.AddRaw(row.GetRaw());
		Raw* raw = RowProxy::ExtractRaw(row);
		pvSetNumber(raw, mRaws.GetCount());
		mRaws.AddBackNogrow(raw);
		++mCrew.GetChangeVersion();
		return pvMakeRowReference(raw);
	}

	template<typename Item, typename ItemArg, typename... Assigners>
	RowReference AddRow(Assigner<Item, ItemArg> assigner, Assigners... assigners)
	{
		return AddRow(pvNewRow(assigner, assigners...));
	}

	TryResult TryAddRow(Row&& row)
	{
		try
		{
			return { AddRow(std::move(row)), nullptr };
		}
		catch (const UniqueIndexViolation& exception)
		{
			return { pvMakeRowReference(exception.raw), &exception.uniqueHash };
		}
	}

	template<typename Item, typename ItemArg, typename... Assigners>
	TryResult TryAddRow(Assigner<Item, ItemArg> assigner, Assigners... assigners)
	{
		return TryAddRow(pvNewRow(assigner, assigners...));
	}

	RowReference InsertRow(size_t rowNumber, Row&& row)
	{
		MOMO_CHECK(rowNumber <= GetCount());
		RowReference rowRef = AddRow(std::move(row));
		for (size_t i = mRaws.GetCount() - 1; i > rowNumber; --i)
			mRaws[i] = mRaws[i - 1];
		mRaws[rowNumber] = RowReferenceProxy::GetRaw(rowRef);
		pvSetNumbers(rowNumber);
		return rowRef;
	}

	template<typename Item, typename ItemArg, typename... Assigners>
	RowReference InsertRow(size_t rowNumber, Assigner<Item, ItemArg> assigner,
		Assigners... assigners)
	{
		return InsertRow(rowNumber, pvNewRow(assigner, assigners...));
	}

	TryResult TryInsertRow(size_t rowNumber, Row&& row)
	{
		try
		{
			return { InsertRow(rowNumber, std::move(row)), nullptr };
		}
		catch (const UniqueIndexViolation& exception)
		{
			return { pvMakeRowReference(exception.raw), &exception.uniqueHash };
		}
	}

	template<typename Item, typename ItemArg, typename... Assigners>
	TryResult TryInsertRow(size_t rowNumber, Assigner<Item, ItemArg> assigner,
		Assigners... assigners)
	{
		return TryInsertRow(rowNumber, pvNewRow(assigner, assigners...));
	}

	Row ExtractRow(ConstRowReference rowRef)
	{
		MOMO_CHECK(&rowRef.GetColumnList() == &GetColumnList());
		return ExtractRow(rowRef.GetNumber());
	}

	Row ExtractRow(size_t rowNumber)
	{
		MOMO_CHECK(rowNumber < GetCount());
		Raw* raw = mRaws[rowNumber];
		mIndexes.RemoveRaw(raw);
		mRaws.Remove(rowNumber, 1);
		pvSetNumbers(rowNumber);
		++mCrew.GetChangeVersion();
		++mCrew.GetRemoveVersion();
		return pvMakeRow(raw);
	}

	RowReference UpdateRow(ConstRowReference rowRef, Row&& row)
	{
		MOMO_CHECK(&rowRef.GetColumnList() == &GetColumnList());
		return UpdateRow(rowRef.GetNumber(), std::move(row));
	}

	RowReference UpdateRow(size_t rowNumber, Row&& row)
	{
		MOMO_CHECK(rowNumber < GetCount());
		Raw*& raw = mRaws[rowNumber];
		mIndexes.UpdateRaw(raw, row.GetRaw());
		pvFreeRaw(raw);
		raw = RowProxy::ExtractRaw(row);
		pvSetNumber(raw, rowNumber);
		++mCrew.GetChangeVersion();
		++mCrew.GetRemoveVersion();
		return pvMakeRowReference(raw);
	}

	TryResult TryUpdateRow(size_t rowNumber, Row&& row)
	{
		try
		{
			return { UpdateRow(rowNumber, std::move(row)), nullptr };
		}
		catch (const UniqueIndexViolation& exception)
		{
			return { pvMakeRowReference(exception.raw), &exception.uniqueHash };
		}
	}

	template<typename RowIterator>
	void AssignRows(RowIterator begin, RowIterator end)
	{
		for (Raw* raw : mRaws)
			pvSetNumber(raw, invalidRowNumber);
		try
		{
			size_t number = 0;
			for (RowIterator iter = begin; iter != end; ++iter)
			{
				MOMO_CHECK(&iter->GetColumnList() == &GetColumnList());
				Raw* raw = RowReferenceProxy::GetRaw(*iter);
				if (GetColumnList().GetNumber(raw) != invalidRowNumber)
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
				size_t number = GetColumnList().GetNumber(raw);
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
				pvSetNumber(RowReferenceProxy::GetRaw(*iter), invalidRowNumber);
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
					pvSetNumber(raw, invalidRowNumber);
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
	IndexHandle GetUniqueHashIndex(const Column<Item>& column, const Column<Items>&... columns) const
	{
		return mIndexes.GetUniqueHash(&GetColumnList(), column, columns...);
	}

	template<typename Item, typename... Items>
	IndexHandle GetMultiHashIndex(const Column<Item>& column, const Column<Items>&... columns) const
	{
		return mIndexes.GetMultiHash(&GetColumnList(), column, columns...);
	}

	template<typename Item, typename... Items>
	void AddUniqueHashIndex(const Column<Item>& column, const Column<Items>&... columns)
	{
		mIndexes.AddUniqueHash(&GetColumnList(), mRaws, column, columns...);
	}

	template<typename Item, typename... Items>
	void AddMultiHashIndex(const Column<Item>& column, const Column<Items>&... columns)
	{
		mIndexes.AddMultiHash(&GetColumnList(), mRaws, column, columns...);
	}

	template<typename Item, typename... Items>
	bool RemoveUniqueHashIndex(const Column<Item>& column, const Column<Items>&... columns)
	{
		return mIndexes.RemoveUniqueHash(&GetColumnList(), column, columns...);
	}

	template<typename Item, typename... Items>
	bool RemoveMultiHashIndex(const Column<Item>& column, const Column<Items>&... columns)
	{
		return mIndexes.RemoveMultiHash(&GetColumnList(), column, columns...);
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

	ConstRowHashPointer FindByUniqueHash(IndexHandle uniqueHashIndex, const Row& row) const
	{
		return pvFindByUniqueHash(static_cast<const UniqueHashIndex*>(uniqueHashIndex), row);
	}

	RowHashPointer FindByUniqueHash(IndexHandle uniqueHashIndex, const Row& row)
	{
		return pvFindByUniqueHash(static_cast<const UniqueHashIndex*>(uniqueHashIndex), row);
	}

	template<typename Item, typename... Items>
	ConstRowHashPointer FindByUniqueHash(IndexHandle uniqueHashIndex, Equaler<Item> equaler,
		Equaler<Items>... equalers) const
	{
		return pvFindByHash<RowHashPointerProxy>(
			static_cast<const UniqueHashIndex*>(uniqueHashIndex), equaler, equalers...);
	}

	template<typename Item, typename... Items>
	RowHashPointer FindByUniqueHash(IndexHandle uniqueHashIndex, Equaler<Item> equaler,
		Equaler<Items>... equalers)
	{
		return pvFindByHash<RowHashPointerProxy>(
			static_cast<const UniqueHashIndex*>(uniqueHashIndex), equaler, equalers...);
	}

	template<typename Item, typename... Items>
	ConstRowHashBounds FindByMultiHash(IndexHandle multiHashIndex, Equaler<Item> equaler,
		Equaler<Items>... equalers) const
	{
		return pvFindByHash<RowHashBoundsProxy>(
			static_cast<const MultiHashIndex*>(multiHashIndex), equaler, equalers...);
	}

	template<typename Item, typename... Items>
	RowHashBounds FindByMultiHash(IndexHandle multiHashIndex, Equaler<Item> equaler,
		Equaler<Items>... equalers)
	{
		return pvFindByHash<RowHashBoundsProxy>(
			static_cast<const MultiHashIndex*>(multiHashIndex), equaler, equalers...);
	}

private:
	RawMemPool pvCreateRawMemPool()
	{
		size_t size = std::minmax(GetColumnList().GetTotalSize(), sizeof(void*)).second;
		size_t alignment = std::minmax(GetColumnList().GetAlignment(), MOMO_ALIGNMENT_OF(void*)).second;
		return RawMemPool(typename RawMemPool::Params(size, alignment), MemManagerPtr(GetMemManager()));
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

	ConstRowReference pvMakeConstRowReference(Raw* raw) const MOMO_NOEXCEPT
	{
		return ConstRowReferenceProxy(&GetColumnList(), raw,
			VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	RowReference pvMakeRowReference(Raw* raw) const MOMO_NOEXCEPT
	{
		return RowReferenceProxy(&GetColumnList(), raw, VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	Iterator pvMakeIterator(size_t index) const MOMO_NOEXCEPT
	{
		return IteratorProxy(&GetColumnList(), RawIterator(mRaws, index),
			VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	Raw* pvCopyRaw(const Raw* srcRaw)
	{
		Raw* dstRaw = mRawMemPool.template Allocate<Raw>();
		try
		{
			mCrew.GetColumnList().CopyRaw(srcRaw, dstRaw);
		}
		catch (...)
		{
			mRawMemPool.Deallocate(dstRaw);
			throw;
		}
		return dstRaw;
	}

	void pvFreeRaws() MOMO_NOEXCEPT
	{
		if (mCrew.IsNull())
			return;
		pvFreeNewRaws();
		for (Raw* raw : mRaws)
			pvFreeRaw(raw);
	}

	void pvFreeRaw(Raw* raw) MOMO_NOEXCEPT
	{
		mCrew.GetColumnList().DestroyRaw(raw);
		mRawMemPool.Deallocate(raw);
	}

	void pvFreeNewRaws() MOMO_NOEXCEPT
	{
		Raw* headRaw = mCrew.GetFreeRaws().exchange(nullptr);
		while (headRaw != nullptr)
		{
			Raw* nextRaw = *reinterpret_cast<Raw**>(headRaw);
			mRawMemPool.Deallocate(headRaw);
			headRaw = nextRaw;
		}
	}

	Row pvMakeRow(Raw* raw) MOMO_NOEXCEPT
	{
		pvFreeNewRaws();
		return RowProxy(&GetColumnList(), raw, &mCrew.GetFreeRaws());
	}

	template<typename Item, typename ItemArg, typename... Assigners>
	Row pvNewRow(const Assigner<Item, ItemArg>& assigner, const Assigners&... assigners)
	{
		Row row = NewRow();
		pvFillRaw(row.GetRaw(), assigner, assigners...);
		return row;
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

	void pvFillRaw(Raw* /*raw*/) MOMO_NOEXCEPT
	{
	}

	void pvSetNumbers(size_t beginIndex = 0) MOMO_NOEXCEPT
	{
		for (size_t i = beginIndex, count = mRaws.GetCount(); i < count; ++i)
			pvSetNumber(mRaws[i], i);
	}

	void pvSetNumber(Raw* raw, size_t number) MOMO_NOEXCEPT
	{
		pvSetNumber(raw, number, KeepRowNumber());
	}

	void pvSetNumber(Raw* raw, size_t number, std::true_type /*keepRowNumber*/) MOMO_NOEXCEPT
	{
		GetColumnList().SetNumber(raw, number);
	}

	void pvSetNumber(Raw* /*raw*/, size_t /*number*/, std::false_type /*keepRowNumber*/) MOMO_NOEXCEPT
	{
	}

	void pvRemoveInvalidRaws() MOMO_NOEXCEPT
	{
		auto rawFilter = [this] (Raw* raw)
			{ return GetColumnList().GetNumber(raw) != invalidRowNumber; };
		mIndexes.FilterRaws(rawFilter);
		size_t number = 0;
		for (Raw* raw : mRaws)
		{
			if (rawFilter(raw))
				mRaws[number++] = raw;
			else
				pvFreeRaw(raw);
		}
		mRaws.RemoveBack(mRaws.GetCount() - number);
		pvSetNumbers();
		++mCrew.GetChangeVersion();
		++mCrew.GetRemoveVersion();
	}

	Selection pvSelectEmpty() const
	{
		MemManager memManager = GetMemManager();
		return SelectionProxy(&GetColumnList(), typename SelectionProxy::Raws(std::move(memManager)),
			VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	template<typename Result, typename RowFilter, typename... Items,
		typename std::enable_if<(sizeof...(Items) > 0), int>::type = 0>
	Result pvSelect(const RowFilter& rowFilter, const Equaler<Items>&... equalers) const
	{
		static const size_t columnCount = sizeof...(equalers);
		const ColumnList& columnList = GetColumnList();
		std::array<size_t, columnCount> offsets = {{ columnList.GetOffset(equalers.GetColumn())... }};
		std::array<size_t, columnCount> sortedOffsets = Indexes::GetSortedOffsets(offsets);
		const UniqueHashIndex* uniqueHash = mIndexes.GetFitUniqueHash(sortedOffsets);
		if (uniqueHash != nullptr)
		{
			return pvSelectRec<Result>(*uniqueHash, offsets.data(), rowFilter,
				OffsetItemTuple<>(), equalers...);
		}
		const MultiHashIndex* multiHash = mIndexes.GetFitMultiHash(sortedOffsets);
		if (multiHash != nullptr)
		{
			return pvSelectRec<Result>(*multiHash, offsets.data(), rowFilter,
				OffsetItemTuple<>(), equalers...);
		}
		auto newFilter = [&offsets, &rowFilter, &equalers...] (ConstRowReference rowRef)
			{ return pvIsSatisfied(rowRef, offsets.data(), equalers...) && rowFilter(rowRef); };
		return pvMakeSelection(mRaws, newFilter, static_cast<Result*>(nullptr));
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
		return DataTraits::IsEqual(rowRef.template GetByOffset<Item>(*offsets), equaler.GetItemArg())
			&& pvIsSatisfied(rowRef, offsets + 1, equalers...);
	}

	static bool pvIsSatisfied(ConstRowReference /*rowRef*/, const size_t* /*offsets*/) MOMO_NOEXCEPT
	{
		return true;
	}

	template<typename Result, typename Index, typename RowFilter, typename Tuple, typename Item,
		typename... Items>
	Result pvSelectRec(const Index& index, const size_t* offsets, const RowFilter& rowFilter,
		const Tuple& tuple, const Equaler<Item>& equaler, const Equaler<Items>&... equalers) const
	{
		size_t offset = *offsets;
		const Item& item = equaler.GetItemArg();
		if (Indexes::HasOffset(index, offset))
		{
			auto newTuple = std::tuple_cat(tuple,
				std::make_tuple(std::pair<size_t, const Item&>(offset, item)));
			return pvSelectRec<Result>(index, offsets + 1, rowFilter, newTuple, equalers...);
		}
		else
		{
			auto newFilter = [&rowFilter, offset, &item] (ConstRowReference rowRef)
			{
				return DataTraits::IsEqual(rowRef.template GetByOffset<Item>(offset), item)
					&& rowFilter(rowRef);
			};
			return pvSelectRec<Result>(index, offsets + 1, newFilter, tuple, equalers...);
		}
	}

	template<typename Result, typename Index, typename RowFilter, typename Tuple>
	Result pvSelectRec(const Index& index, const size_t* /*offsets*/, const RowFilter& rowFilter,
		const Tuple& tuple) const
	{
		return pvMakeSelection(
			mIndexes.FindRaws(&GetColumnList(), index, tuple, VersionKeeper(&mCrew.GetChangeVersion())),
			rowFilter, static_cast<Result*>(nullptr));
	}

#ifdef _MSC_VER	//?
	template<typename Result, typename Index, typename RowFilter>
	Result pvSelectRec(const Index&, const size_t*, const RowFilter&, const OffsetItemTuple<>&) const
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
		return std::count_if(raws.GetBegin(), raws.GetEnd(),
			[this, &rowFilter] (Raw* raw) { return rowFilter(pvMakeConstRowReference(raw)); });
	}

	template<typename Raws>
	size_t pvMakeSelection(const Raws& raws, const EmptyRowFilter& /*rowFilter*/,
		size_t*) const MOMO_NOEXCEPT
	{
		return std::distance(raws.GetBegin(), raws.GetEnd());
	}

	RowHashPointer pvFindByUniqueHash(const UniqueHashIndex* uniqueHashIndex, const Row& row) const
	{
		const ColumnList* columnList = &GetColumnList();
		MOMO_CHECK(uniqueHashIndex != nullptr);
		MOMO_CHECK(&row.GetColumnList() == columnList);
		auto raws = mIndexes.FindRaws(columnList, *uniqueHashIndex, RowProxy::GetRaw(row),
			VersionKeeper(&mCrew.GetChangeVersion()));
		return RowHashPointerProxy(columnList, raws, VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	template<typename RowBoundsProxy, typename Index, typename... Items>
	RowBoundsProxy pvFindByHash(const Index* index, const Equaler<Items>&... equalers) const
	{
		static const size_t columnCount = sizeof...(equalers);
		const ColumnList& columnList = GetColumnList();
		std::array<size_t, columnCount> offsets = {{ columnList.GetOffset(equalers.GetColumn())... }};
		if (index == nullptr)
			index = mIndexes.GetHash(Indexes::GetSortedOffsets(offsets), index);
		else
			MOMO_EXTRA_CHECK(index == mIndexes.GetHash(Indexes::GetSortedOffsets(offsets), index));	//?
		if (index == nullptr)
			throw std::runtime_error("Index not found");
		return pvFindByHashRec<RowBoundsProxy>(*index, offsets.data(), OffsetItemTuple<>(), equalers...);
	}

	template<typename RowBoundsProxy, typename Index, typename Tuple, typename Item, typename... Items>
	RowBoundsProxy pvFindByHashRec(const Index& index, const size_t* offsets, const Tuple& tuple,
		const Equaler<Item>& equaler, const Equaler<Items>&... equalers) const
	{
		auto newTuple = std::tuple_cat(tuple,
			std::make_tuple(std::pair<size_t, const Item&>(*offsets, equaler.GetItemArg())));
		return pvFindByHashRec<RowBoundsProxy>(index, offsets + 1, newTuple, equalers...);
	}

	template<typename RowBoundsProxy, typename Index, typename Tuple>
	RowBoundsProxy pvFindByHashRec(const Index& index, const size_t* /*offsets*/,
		const Tuple& tuple) const
	{
		const ColumnList* columnList = &GetColumnList();
		return RowBoundsProxy(columnList,
			mIndexes.FindRaws(columnList, index, tuple, VersionKeeper(&mCrew.GetChangeVersion())),
			VersionKeeper(&mCrew.GetRemoveVersion()));
	}

private:
	Crew mCrew;
	Raws mRaws;
	RawMemPool mRawMemPool;
	Indexes mIndexes;
};

} // namespace experimental

} // namespace momo
