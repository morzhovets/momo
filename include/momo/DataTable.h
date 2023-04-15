/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataTable.h

  namespace momo:
    concept conceptDataTraits
    class DataTraits
    class DataTable

\**********************************************************/

#pragma once

#include "DataColumn.h"
#include "DataRow.h"
#include "DataSelection.h"
#include "DataIndexes.h"
#include "Array.h"
#include "MemPool.h"

namespace momo
{

template<typename DataTraits>
concept conceptDataTraits =
	internal::conceptMemPoolParamsBlockSizeAlignment<typename DataTraits::RawMemPoolParams>;

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
		if (std::less<Item>()(item1, item2))
			return -1;
		return (item1 == item2) ? 0 : 1;
	}

	template<typename Iterator, typename Comparer, typename MemManager>
	static void Sort(Iterator begin, size_t count, const Comparer& comparer,
		MemManager& /*memManager*/)
	{
		std::sort(begin, internal::UIntMath<>::Next(begin, count), comparer);	//?
	}
};

/*!
	\brief
	`momo::DataTable` is similar to `Boost.MultiIndex`, but its API looks like `ADO.NET DataTable`.
*/

template<conceptDataColumnList TColumnList = DataColumnList<>,
	conceptDataTraits TDataTraits = DataTraits>
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

	typedef internal::DataRowReference<ColumnList> RowReference;
	typedef typename RowReference::ConstReference ConstRowReference;

	typedef internal::DataSelection<RowReference, DataTraits> Selection;
	typedef typename Selection::ConstSelection ConstSelection;

private:
	typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

	typedef internal::MemPoolLazy<typename DataTraits::RawMemPoolParams, MemManagerPtr> RawMemPool;

	typedef Array<Raw*, MemManagerPtr, ArrayItemTraits<Raw*, MemManagerPtr>,
		internal::NestedArraySettings<typename Settings::TableRawsSettings>> Raws;

	typedef internal::DataIndexes<ColumnList, DataTraits> Indexes;

	typedef internal::DataRawIterator<Raws, Settings> RawIterator;

	typedef internal::ArrayBounds<RawIterator> RawBounds;
	typedef internal::DataRowBounds<RawBounds, ConstRowReference> ConstRowBounds;

public:
	typedef internal::DataRow<ColumnList, RawMemPool> Row;

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

	static const size_t invalidNumber = internal::UIntConst::maxSize;

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

		typedef typename RawMemPool::Params RawMemPoolParams;

		class Data
		{
		public:
			explicit Data(ColumnList&& columnList) noexcept
				: columnList(std::move(columnList)),
				changeVersion(0),
				removeVersion(0)
			{
			}

		public:
			ColumnList columnList;
			size_t changeVersion;
			size_t removeVersion;
			internal::ObjectBuffer<RawMemPool, alignof(RawMemPool)> rawMemPoolBuffer;
		};

	public:
		explicit Crew(ColumnList&& columnList)
		{
			RawMemPoolParams rawMemPoolParams(
				std::minmax(columnList.GetTotalSize(), sizeof(void*)).second,
				columnList.GetAlignment());
			mData = MemManagerProxy::template Allocate<Data>(columnList.GetMemManager(),
				sizeof(Data));
			std::construct_at(mData, std::move(columnList));
			std::construct_at(&mData->rawMemPoolBuffer, std::move(rawMemPoolParams),
				MemManagerPtr(GetMemManager()));	//? nothrow
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
				std::destroy_at(&mData->rawMemPoolBuffer);
				ColumnList columnList = std::move(mData->columnList);
				std::destroy_at(mData);
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

		RawMemPool& GetRawMemPool() noexcept
		{
			MOMO_ASSERT(!IsNull());
			return *&mData->rawMemPoolBuffer;
		}

	private:
		Data* mData;
	};

	struct RowProxy : public Row
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Row)
		MOMO_DECLARE_PROXY_FUNCTION(Row, GetRaw)
		MOMO_DECLARE_PROXY_FUNCTION(Row, ExtractRaw)
	};

	struct ConstRowReferenceProxy : public ConstRowReference
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstRowReference)
		MOMO_DECLARE_PROXY_FUNCTION(ConstRowReference, GetRaw)
	};

	struct RowReferenceProxy : public RowReference
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(RowReference)
		//MOMO_DECLARE_PROXY_FUNCTION(RowReference, GetRaw)
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
		requires (requires { ColumnList(); }) && (!requires { ColumnList({}); })
		: DataTable(ColumnList())
	{
	}

	explicit DataTable(ColumnList&& columnList)
		: mCrew(std::move(columnList)),
		mRaws(MemManagerPtr(GetMemManager())),
		mIndexes(GetMemManager())
	{
	}

	DataTable(DataTable&& table) noexcept
		: mCrew(std::move(table.mCrew)),
		mRaws(std::move(table.mRaws)),
		mIndexes(std::move(table.mIndexes))
	{
	}

	DataTable(const DataTable& table)
		: DataTable(table, EmptyRowFilter())
	{
	}

	template<typename RowFilter>
	requires std::predicate<const RowFilter&, ConstRowReference>
	explicit DataTable(const DataTable& table, const RowFilter& rowFilter, bool copyIndexes = true)
		: DataTable(ColumnList(table.GetColumnList()))
	{
		if (copyIndexes)
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
	MOMO_FRIENDS_SIZE_BEGIN_END(DataTable)

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
	Row NewRow(Assigner<Item, ItemArg> assigner, Assigner<Items, ItemArgs>... assigners)
	{
		return pvNewRow(assigner, assigners...);
	}

	Row NewRow(const Row& row)
	{
		return pvMakeRow(pvImportRaw(row.GetColumnList(), row.GetRaw()));
	}

	Row NewRow(ConstRowReference rowRef)
	{
		return pvMakeRow(pvImportRaw(rowRef.GetColumnList(), rowRef.GetRaw()));
	}

	RowReference AddRow(Row&& row)
	{
		TryResult res = TryAddRow(std::move(row));
		if (res.uniqueHashIndex != UniqueHashIndex::empty)
			throw UniqueIndexViolation(res);
		return res.rowReference;
	}

	template<typename Item, typename ItemArg, typename... Items, typename... ItemArgs>
	RowReference AddRow(Assigner<Item, ItemArg> assigner, Assigner<Items, ItemArgs>... assigners)
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

	template<typename Item, typename ItemArg, typename... Items, typename... ItemArgs>
	TryResult TryAddRow(Assigner<Item, ItemArg> assigner, Assigner<Items, ItemArgs>... assigners)
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

	template<typename Item, typename ItemArg, typename... Items, typename... ItemArgs>
	RowReference InsertRow(size_t rowNumber, Assigner<Item, ItemArg> assigner,
		Assigner<Items, ItemArgs>... assigners)
	{
		return InsertRow(rowNumber, pvNewRow(assigner, assigners...));
	}

	TryResult TryInsertRow(size_t rowNumber, Row&& row)
	{
		MOMO_CHECK(rowNumber <= GetCount());
		TryResult res = TryAddRow(std::move(row));
		if (res.uniqueHashIndex == UniqueHashIndex::empty)
		{
			std::rotate(internal::UIntMath<>::Next(mRaws.GetBegin(), rowNumber),
				std::prev(mRaws.GetEnd()), mRaws.GetEnd());
			pvSetNumbers(rowNumber);
		}
		return res;
	}

	template<typename Item, typename ItemArg, typename... Items, typename... ItemArgs>
	TryResult TryInsertRow(size_t rowNumber, Assigner<Item, ItemArg> assigner,
		Assigner<Items, ItemArgs>... assigners)
	{
		return TryInsertRow(rowNumber, pvNewRow(assigner, assigners...));
	}

	void RemoveRow(ConstRowReference rowRef)
	{
		MOMO_CHECK(&rowRef.GetColumnList() == &GetColumnList());
		pvDestroyRaw(pvExtractRaw(rowRef));
	}

	void RemoveRow(size_t rowNumber, bool keepRowOrder = true)
	{
		MOMO_CHECK(rowNumber < GetCount());
		pvDestroyRaw(pvExtractRaw(rowNumber, keepRowOrder));
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
		TryResult res = pvTryUpdateRow(rowRef, column, std::move(newItem));
		if (res.uniqueHashIndex != UniqueHashIndex::empty)
			throw UniqueIndexViolation(res);
		return res.rowReference;
	}

	template<typename Item>
	RowReference UpdateRow(ConstRowReference rowRef, const Column<Item>& column, const Item& newItem)
	{
		TryResult res = pvTryUpdateRow(rowRef, column, newItem);
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
		pvDestroyRaw(raw);
		raw = RowProxy::ExtractRaw(row);
		pvSetNumber(raw, rowNumber);
		++mCrew.GetChangeVersion();
		++mCrew.GetRemoveVersion();
		return { pvMakeRowReference(raw), UniqueHashIndex::empty };
	}

	template<typename Item>
	TryResult TryUpdateRow(ConstRowReference rowRef, const Column<Item>& column, Item&& newItem)
	{
		return pvTryUpdateRow(rowRef, column, std::move(newItem));
	}

	template<typename Item>
	TryResult TryUpdateRow(ConstRowReference rowRef, const Column<Item>& column, const Item& newItem)
	{
		return pvTryUpdateRow(rowRef, column, newItem);
	}

	template<internal::conceptDataRowIterator<ConstRowReference> RowIterator>
	void AssignRows(RowIterator begin, RowIterator end)
	{
		pvAssignRows(begin, end);
	}

	template<internal::conceptDataRowIterator<ConstRowReference> RowIterator>
	void RemoveRows(RowIterator begin, RowIterator end)
	{
		pvRemoveRows(begin, end);
	}

	template<typename RowFilter>
	requires std::predicate<const RowFilter&, ConstRowReference>
	size_t RemoveRows(const RowFilter& rowFilter)
	{
		size_t initCount = GetCount();
		pvRemoveRows(rowFilter);
		return initCount - GetCount();
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

	template<typename... Items>
	ConstSelection Select(Equaler<Items>... equalers) const
	{
		return pvSelect<Selection>(EmptyRowFilter(), equalers...);
	}

	template<typename RowFilter, typename... Items>
	requires std::predicate<const RowFilter&, ConstRowReference>
	ConstSelection Select(const RowFilter& rowFilter, Equaler<Items>... equalers) const
	{
		return pvSelect<Selection>(rowFilter, equalers...);
	}

	template<typename... Items>
	Selection Select(Equaler<Items>... equalers)
	{
		return pvSelect<Selection>(EmptyRowFilter(), equalers...);
	}

	template<typename RowFilter, typename... Items>
	requires std::predicate<const RowFilter&, ConstRowReference>
	Selection Select(const RowFilter& rowFilter, Equaler<Items>... equalers)
	{
		return pvSelect<Selection>(rowFilter, equalers...);
	}

	template<typename... Items>
	size_t SelectCount(Equaler<Items>... equalers) const
	{
		return pvSelect<size_t>(EmptyRowFilter(), equalers...);
	}

	template<typename RowFilter, typename... Items>
	requires std::predicate<const RowFilter&, ConstRowReference>
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
	DataTable Project(ColumnList&& resColumnList, const Column<Item>& column,
		const Column<Items>&... columns) const
	{
		return pvProject<false>(std::move(resColumnList), EmptyRowFilter(), column, columns...);
	}

	template<typename RowFilter, typename Item, typename... Items>
	requires std::predicate<const RowFilter&, ConstRowReference>
	DataTable Project(ColumnList&& resColumnList, const RowFilter& rowFilter,
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
	requires std::predicate<const RowFilter&, ConstRowReference>
	DataTable ProjectDistinct(ColumnList&& resColumnList, const RowFilter& rowFilter,
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

private:
	template<typename Rows, typename RowFilter>
	void pvFill(const Rows& rows, const RowFilter& rowFilter)
	{
		const ColumnList& columnList = GetColumnList();
		if constexpr (std::is_same_v<RowFilter, EmptyRowFilter>)
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

	Raw* pvCreateRaw()
	{
		Raw* raw = mCrew.GetRawMemPool().template Allocate<Raw>();
		try
		{
			GetColumnList().CreateRaw(GetMemManager(), raw);
		}
		catch (...)
		{
			mCrew.GetRawMemPool().Deallocate(raw);
			throw;
		}
		return raw;
	}

	Raw* pvImportRaw(const ColumnList& srcColumnList, const Raw* srcRaw)
	{
		Raw* raw = mCrew.GetRawMemPool().template Allocate<Raw>();
		try
		{
			GetColumnList().ImportRaw(GetMemManager(), srcColumnList, srcRaw, raw);
		}
		catch (...)
		{
			mCrew.GetRawMemPool().Deallocate(raw);
			throw;
		}
		return raw;
	}

	void pvDestroyRaws() noexcept
	{
		if (mCrew.IsNull())
			return;
		for (Raw* raw : mRaws)
			pvDestroyRaw(raw);
	}

	void pvDestroyRaw(Raw* raw) noexcept
	{
		GetColumnList().DestroyRaw(&GetMemManager(), raw);
		mCrew.GetRawMemPool().Deallocate(raw);
	}

	Row pvMakeRow(Raw* raw) noexcept
	{
		return RowProxy(&GetColumnList(), raw, &mCrew.GetRawMemPool());
	}

	template<typename... Items, typename... ItemArgs>
	Row pvNewRow(const Assigner<Items, ItemArgs>&... assigners)
	{
		Raw* raw = pvCreateRaw();
		try
		{
			(pvFillRaw(raw, assigners), ...);
		}
		catch (...)
		{
			pvDestroyRaw(raw);
			throw;
		}
		return pvMakeRow(raw);
	}

	template<typename Item, typename ItemArg>
	void pvFillRaw(Raw* raw, const Assigner<Item, ItemArg>& assigner) const
	{
		const ColumnList& columnList = GetColumnList();
		size_t offset = columnList.GetOffset(assigner.GetColumn());
		columnList.template Assign<Item>(raw, offset, std::forward<ItemArg>(assigner.GetItemArg()));
	}

	void pvSetNumbers(size_t beginNumber = 0) noexcept
	{
		for (size_t i = beginNumber, count = mRaws.GetCount(); i < count; ++i)
			pvSetNumber(mRaws[i], i);
	}

	void pvSetNumber(Raw* raw, size_t number) noexcept
	{
		if constexpr (Settings::keepRowNumber)
			GetColumnList().SetNumber(raw, number);
	}

	Raw* pvExtractRaw(ConstRowReference rowRef)
	{
		size_t number;
		if constexpr (Settings::keepRowNumber)
		{
			number = rowRef.GetNumber();
		}
		else
		{
			const Raw* raw = rowRef.GetRaw();
			number = mRaws.GetCount() - 1;
			while (mRaws[number] != raw)
				--number;
		}
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
	TryResult pvTryUpdateRow(ConstRowReference rowRef, const Column<Item>& column, RItem&& newItem)
	{
		const ColumnList& columnList = GetColumnList();
		MOMO_CHECK(&rowRef.GetColumnList() == &columnList);
		rowRef.GetRaw();	// check
		Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
		size_t offset = GetColumnList().GetOffset(column);
		auto assigner = [&columnList, raw, offset, &newItem] ()
			{ columnList.template Assign<Item>(raw, offset, std::forward<RItem>(newItem)); };
		auto res = mIndexes.UpdateRaw(raw, offset, std::as_const(newItem), assigner);
		if (res.raw != nullptr)
			return { pvMakeRowReference(res.raw), res.uniqueHashIndex };
		++mCrew.GetChangeVersion();
		return { pvMakeRowReference(raw), UniqueHashIndex::empty };
	}

	template<typename RowIterator>
	void pvAssignRows(RowIterator begin, RowIterator end)
		requires (Settings::keepRowNumber)
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

	template<typename RowIterator>
	void pvAssignRows(RowIterator begin, RowIterator end)
		requires (!Settings::keepRowNumber)
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

	template<typename RowIterator>
	void pvRemoveRows(RowIterator begin, RowIterator end)
		requires (Settings::keepRowNumber)
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

	template<typename RowIterator>
	void pvRemoveRows(RowIterator begin, RowIterator end)
		requires (!Settings::keepRowNumber)
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

	template<typename RowFilter>
	void pvRemoveRows(const RowFilter& rowFilter)
		requires (Settings::keepRowNumber)
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

	template<typename RowFilter>
	void pvRemoveRows(const RowFilter& rowFilter)
		requires (!Settings::keepRowNumber)
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
	std::array<size_t, columnCount> pvGetOffsets(const Equaler<Items>&... equalers) const
	{
		return pvGetOffsets(equalers.GetColumn()...);
	}

	template<size_t columnCount>
	void pvCheckImmutable(const std::array<size_t, columnCount>& offsets) const
	{
		for ([[maybe_unused]] size_t offset : offsets)
			MOMO_CHECK(!GetColumnList().IsMutable(offset));
	}

	Selection pvSelectEmpty() const
	{
		MemManager memManager = GetMemManager();
		return SelectionProxy(&GetColumnList(), typename SelectionProxy::Raws(std::move(memManager)),
			VersionKeeper(&mCrew.GetRemoveVersion()));
	}

	template<typename Result, typename RowFilter, typename Item, typename... Items,
		size_t columnCount = 1 + sizeof...(Items)>
	requires (columnCount > DataTraits::selectEqualerMaxCount)
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
		size_t columnCount = sizeof...(Items)>
	requires (0 < columnCount && columnCount <= DataTraits::selectEqualerMaxCount)
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
		{
			Raw* raw = ConstRowReferenceProxy::GetRaw(rowRef);
			const size_t* offsetPtr = offsets.data();
			return (pvIsSatisfied(raw, equalers, *offsetPtr++) && ...) && rowFilter(rowRef);
		};
		return pvMakeSelection(mRaws, newRowFilter, static_cast<Result*>(nullptr));
	}

	template<typename Result, typename RowFilter>
	Result pvSelect(const RowFilter& rowFilter) const
	{
		return pvMakeSelection(mRaws, rowFilter, static_cast<Result*>(nullptr));
	}

	template<typename Item>
	static bool pvIsSatisfied(Raw* raw, const Equaler<Item>& equaler, size_t offset)
	{
		const Item& item = ColumnList::template GetByOffset<const Item>(raw, offset);
		return DataTraits::IsEqual(item, equaler.GetItemArg());
	}

	template<typename Result, typename Index, typename RowFilter, typename Tuple, typename Item,
		typename... Items>
	Result pvSelectRec(Index index, const size_t* offsetPtr, const RowFilter& rowFilter,
		Tuple&& tuple, const Equaler<Item>& equaler, const Equaler<Items>&... equalers) const
	{
		size_t offset = *offsetPtr;
		if (mIndexes.ContainsOffset(index, offset))
		{
			auto newTuple = std::tuple_cat(std::move(tuple),
				std::make_tuple(std::pair<size_t, const Item&>(offset, equaler.GetItemArg())));
			return pvSelectRec<Result>(index, offsetPtr + 1, rowFilter, std::move(newTuple),
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
			return pvSelectRec<Result>(index, offsetPtr + 1, newRowFilter, std::move(tuple),
				equalers...);
		}
	}

	template<typename Result, typename Index, typename RowFilter, typename Tuple>
	Result pvSelectRec(Index index, const size_t* /*offsetPtr*/, const RowFilter& rowFilter,
		Tuple&& tuple) const
	{
		return pvMakeSelection(mIndexes.FindRaws(index, tuple, VersionKeeper(&mCrew.GetChangeVersion())),
			rowFilter, static_cast<Result*>(nullptr));
	}

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
		OffsetItemTuple<Items...> tuple = { { 0, equalers.GetItemArg() }... };
		const size_t* offsetPtr = offsets.data();
		std::apply([&offsetPtr] (auto&... pairs) { ((pairs.first = *offsetPtr++), ...); }, tuple);
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
		if constexpr (distinct)
			resTable.mIndexes.template AddUniqueHashIndex<Items...>(resTable.mRaws, resOffsets);
		for (Raw* raw : mRaws)
		{
			if (!rowFilter(pvMakeConstRowReference(raw)))
				continue;
			resTable.mRaws.Reserve(resTable.mRaws.GetCount() + 1);
			Raw* resRaw = resTable.pvCreateRaw();
			resTable.mRaws.AddBackNogrow(resRaw);
			resTable.template pvAssign<Items...>(raw, offsets.data(), resRaw, resOffsets.data());
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

	template<typename... Items>
	void pvAssign(Raw* srcRaw, const size_t* srcOffsets, Raw* dstRaw, const size_t* dstOffsets) const
	{
		const ColumnList& columnList = GetColumnList();
		const size_t* srcOffsetPtr = srcOffsets;
		const size_t* dstOffsetPtr = dstOffsets;
		(columnList.template Assign<Items>(dstRaw, *dstOffsetPtr++,
			ColumnList::template GetByOffset<const Items>(srcRaw, *srcOffsetPtr++)), ...);
	}

private:
	Crew mCrew;
	Raws mRaws;
	Indexes mIndexes;
};

} // namespace momo
