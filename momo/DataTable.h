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
	typedef MemPoolParams<> RawMemPoolParams;

	template<typename Type>
	static size_t GetHashCode(const Type& item)
	{
		return std::hash<Type>()(item);
	}

	template<typename Type>
	static bool IsEqual(const Type& item1, const Type& item2)
	{
		return item1 == item2;
	}
};

template<typename TColumnList,
	typename TDataTraits = DataTraits>
class DataTable
{
public:
	typedef TColumnList ColumnList;
	typedef TDataTraits DataTraits;
	typedef typename ColumnList::MemManager MemManager;
	typedef typename ColumnList::Settings Settings;
	typedef typename ColumnList::Raw Raw;

	template<typename Type>
	using Column = typename ColumnList::template Column<Type>;

	typedef internal::DataRow<ColumnList> Row;
	typedef internal::DataRowReference<ColumnList> RowReference;
	typedef typename RowReference::ConstReference ConstRowReference;

	typedef internal::DataSelection<RowReference> Selection;
	typedef typename Selection::ConstSelection ConstSelection;

private:
	typedef momo::internal::MemManagerPtr<MemManager> MemManagerPtr;

	typedef Array<Raw*, MemManagerPtr, ArrayItemTraits<Raw*, MemManagerPtr>,
		momo::internal::NestedArraySettings<typename Settings::RawsSettings>> Raws;

	typedef internal::DataIndexes<ColumnList, DataTraits> Indexes;

	typedef typename Indexes::UniqueIndexViolation UniqueIndexViolation;
	typedef typename Indexes::UniqueHash UniqueHashIndex;
	typedef typename Indexes::MultiHash MultiHashIndex;

public:
	typedef internal::DataRowIterator<RowReference, typename Raws::ConstIterator> Iterator;
	typedef typename Iterator::ConstIterator ConstIterator;

	typedef internal::DataRowBounds<RowReference,
		typename UniqueHashIndex::RawBounds> UniqueHashRowBounds;
	typedef typename UniqueHashRowBounds::ConstBounds UniqueHashConstRowBounds;

	typedef internal::DataRowBounds<RowReference,
		typename MultiHashIndex::RawBounds> MultiHashRowBounds;
	typedef typename MultiHashRowBounds::ConstBounds MultiHashConstRowBounds;

	struct TryResult
	{
		RowReference rowRef;
		const void* uniqueHashIndex;
	};

private:
	typedef momo::internal::BoolConstant<Settings::keepRowNumber> KeepRowNumber;

	//? MemPoolSettings
	typedef MemPool<typename DataTraits::RawMemPoolParams, MemManagerPtr> RawMemPool;

	template<typename... Types>
	using OffsetItemTuple = typename Indexes::template OffsetItemTuple<Types...>;

	struct EmptyFilter
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
			FreeRaws freeRaws;
		};

	public:
		explicit Crew(ColumnList&& columnList)
		{
			mData = columnList.GetMemManager().template Allocate<Data>(sizeof(Data));
			new(&mData->columnList) ColumnList(std::move(columnList));
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
	};

	struct ConstRowReferenceProxy : public ConstRowReference
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstRowReference)
	};

	struct RowReferenceProxy : public RowReference
	{
		MOMO_DECLARE_PROXY_CONSTRUCTOR(RowReference)
	};

	struct SelectionProxy : public Selection
	{
		typedef typename Selection::Raws Raws;
		MOMO_DECLARE_PROXY_CONSTRUCTOR(Selection)
	};

public:
	explicit DataTable(ColumnList&& columnList = ColumnList())
		: mCrew(std::move(columnList)),
		mRaws(MemManagerPtr(GetMemManager())),
		mRawMemPool(typename RawMemPool::Params(pvGetRawSize()), MemManagerPtr(GetMemManager())),
		mIndexes(&GetColumnList(), GetMemManager())
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
		: DataTable(table, EmptyFilter())
	{
	}

	template<typename Filter>
	DataTable(const DataTable& table, const Filter& filter)
		: DataTable(ColumnList(table.GetColumnList()))
	{
		if (std::is_same<Filter, EmptyFilter>::value)
			mRaws.Reserve(table.GetCount());
		try
		{
			for (ConstRowReference srcRowRef : table)
			{
				if (!filter(srcRowRef))
					continue;
				mRaws.Reserve(mRaws.GetCount() + 1);
				Raw* dstRaw = pvCopyRaw(srcRowRef.GetRaw());
				try
				{
					mIndexes.AddRaw(dstRaw);
				}
				catch (...)
				{
					pvFreeRaw(dstRaw);
					throw;
				}
				pvSetNumber(dstRaw, mRaws.GetCount(), KeepRowNumber());
				mRaws.AddBackNogrow(dstRaw);
			}
		}
		catch (...)
		{
			pvFreeRaws();
			throw;
		}
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
		return ConstIterator(&GetColumnList(), mRaws.GetBegin());
	}

	Iterator GetBegin() MOMO_NOEXCEPT
	{
		return Iterator(&GetColumnList(), mRaws.GetBegin());
	}

	ConstIterator GetEnd() const MOMO_NOEXCEPT
	{
		return ConstIterator(&GetColumnList(), mRaws.GetEnd());
	}

	Iterator GetEnd() MOMO_NOEXCEPT
	{
		return Iterator(&GetColumnList(), mRaws.GetEnd());
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
		return pvNewRow(raw);
	}

	template<typename Type, typename TypeArg, typename... Args>
	Row NewRow(const Column<Type>& column, TypeArg&& itemArg, Args&&... args)
	{
		Row row = NewRow();
		pvFillRaw(row.GetRaw(), column, std::forward<TypeArg>(itemArg), std::forward<Args>(args)...);
		return row;
	}

	Row NewRow(const Row& row)
	{
		MOMO_CHECK(&row.GetColumnList() == &GetColumnList());
		return pvNewRow(pvCopyRaw(row.GetRaw()));
	}

	Row NewRow(ConstRowReference rowRef)
	{
		MOMO_CHECK(&rowRef.GetColumnList() == &GetColumnList());
		return pvNewRow(pvCopyRaw(rowRef.GetRaw()));
	}

	RowReference AddRow(Row&& row)
	{
		MOMO_CHECK(&row.GetColumnList() == &GetColumnList());
		mRaws.Reserve(mRaws.GetCount() + 1);
		mIndexes.AddRaw(row.GetRaw());
		Raw* raw = row.ExtractRaw();
		pvSetNumber(raw, mRaws.GetCount(), KeepRowNumber());
		mRaws.AddBackNogrow(raw);
		return pvMakeRowReference(raw);
	}

	template<typename Type, typename TypeArg, typename... Args>
	RowReference AddRow(const Column<Type>& column, TypeArg&& itemArg, Args&&... args)
	{
		return AddRow(NewRow(column, std::forward<TypeArg>(itemArg), std::forward<Args>(args)...));
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

	RowReference InsertRow(size_t rowNumber, Row&& row)
	{
		MOMO_CHECK(rowNumber <= GetCount());
		Raw* raw = row.GetRaw();
		RowReference rowRef = AddRow(std::move(row));
		for (size_t i = rowNumber, count = mRaws.GetCount(); i < count; ++i)
		{
			pvSetNumber(raw, i, KeepRowNumber());
			std::swap(raw, mRaws[i]);
		}
		return rowRef;
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
		for (size_t i = rowNumber, count = mRaws.GetCount(); i < count; ++i)
			pvSetNumber(mRaws[i], i, KeepRowNumber());
		return pvNewRow(raw);
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
		raw = row.ExtractRaw();
		pvSetNumber(raw, rowNumber, KeepRowNumber());
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

	template<typename... Types>
	const void* GetUniqueHashIndex(const Column<Types>&... columns) const
	{
		return mIndexes.GetUniqueHash(columns...);
	}

	template<typename... Types>
	const void* GetMultiHashIndex(const Column<Types>&... columns) const
	{
		return mIndexes.GetMultiHash(columns...);
	}

	template<typename... Types>
	const void* AddUniqueHashIndex(const Column<Types>&... columns)
	{
		return mIndexes.AddUniqueHash(mRaws, columns...);
	}

	template<typename... Types>
	const void* AddMultiHashIndex(const Column<Types>&... columns)
	{
		return mIndexes.AddMultiHash(mRaws, columns...);
	}

	template<typename... Types>
	bool RemoveUniqueHashIndex(const Column<Types>&... columns)
	{
		return mIndexes.RemoveUniqueHash(columns...);
	}

	template<typename... Types>
	bool RemoveMultiHashIndex(const Column<Types>&... columns)
	{
		return mIndexes.RemoveMultiHash(columns...);
	}

	ConstSelection SelectEmpty() const
	{
		MemManager memManager = GetMemManager();
		return ConstSelection(&GetColumnList(), SelectionRaws(std::move(memManager)));
	}

	Selection SelectEmpty()
	{
		MemManager memManager = GetMemManager();
		return Selection(&GetColumnList(), SelectionRaws(std::move(memManager)));
	}

	template<typename Type, typename... Args>
	ConstSelection Select(const Column<Type>& column, const Type& item, const Args&... args) const
	{
		return Select(EmptyFilter(), column, item, args...);
	}

	template<typename Filter, typename Type, typename... Args>
	ConstSelection Select(const Filter& filter, const Column<Type>& column, const Type& item,
		const Args&... args) const
	{
		return pvSelect<Selection>(filter, column, item, args...);
	}

	ConstSelection Select() const
	{
		return Select(EmptyFilter());
	}

	template<typename Filter>
	ConstSelection Select(const Filter& filter) const
	{
		return pvMakeSelection(mRaws, filter, static_cast<Selection*>(nullptr));
	}

	template<typename Type, typename... Args>
	Selection Select(const Column<Type>& column, const Type& item, const Args&... args)
	{
		return Select(EmptyFilter(), column, item, args...);
	}

	template<typename Filter, typename Type, typename... Args>
	Selection Select(const Filter& filter, const Column<Type>& column, const Type& item,
		const Args&... args)
	{
		return pvSelect<Selection>(filter, column, item, args...);
	}

	Selection Select()
	{
		return Select(EmptyFilter());
	}

	template<typename Filter>
	Selection Select(const Filter& filter)
	{
		return pvMakeSelection(mRaws, filter, static_cast<Selection*>(nullptr));
	}

	template<typename Type, typename... Args>
	size_t SelectCount(const Column<Type>& column, const Type& item, const Args&... args) const
	{
		return SelectCount(EmptyFilter(), column, item, args...);
	}

	template<typename Filter, typename Type, typename... Args>
	size_t SelectCount(const Filter& filter, const Column<Type>& column, const Type& item,
		const Args&... args) const
	{
		return pvSelect<size_t>(filter, column, item, args...);
	}

	size_t SelectCount() const
	{
		return SelectCount(EmptyFilter());
	}

	template<typename Filter>
	size_t SelectCount(const Filter& filter) const
	{
		return pvMakeSelection(mRaws, filter, static_cast<size_t*>(nullptr));
	}

	UniqueHashConstRowBounds FindByUniqueHash(const void* uniqueHashIndex, const Row& row) const
	{
		return pvFindByUniqueHash<UniqueHashConstRowBounds>(
			static_cast<const UniqueHashIndex*>(uniqueHashIndex), row);
	}

	UniqueHashRowBounds FindByUniqueHash(const void* uniqueHashIndex, const Row& row)
	{
		return pvFindByUniqueHash<UniqueHashRowBounds>(
			static_cast<const UniqueHashIndex*>(uniqueHashIndex), row);
	}

	template<typename Type, typename... Args>
	UniqueHashConstRowBounds FindByUniqueHash(const void* uniqueHashIndex,
		const Column<Type>& column, const Type& item, const Args&... args) const
	{
		return pvFindByHash<UniqueHashConstRowBounds>(static_cast<const UniqueHashIndex*>(uniqueHashIndex),
			column, item, args...);
	}

	template<typename Type, typename... Args>
	UniqueHashRowBounds FindByUniqueHash(const void* uniqueHashIndex,
		const Column<Type>& column, const Type& item, const Args&... args)
	{
		return pvFindByHash<UniqueHashRowBounds>(static_cast<const UniqueHashIndex*>(uniqueHashIndex),
			column, item, args...);
	}

	template<typename Type, typename... Args>
	MultiHashConstRowBounds FindByMultiHash(const void* multiHashIndex,
		const Column<Type>& column, const Type& item, const Args&... args) const
	{
		return pvFindByHash<MultiHashConstRowBounds>(static_cast<const MultiHashIndex*>(multiHashIndex),
			column, item, args...);
	}

	template<typename Type, typename... Args>
	MultiHashRowBounds FindByMultiHash(const void* multiHashIndex,
		const Column<Type>& column, const Type& item, const Args&... args)
	{
		return pvFindByHash<MultiHashRowBounds>(static_cast<const MultiHashIndex*>(multiHashIndex),
			column, item, args...);
	}

private:
	size_t pvGetRawSize() const MOMO_NOEXCEPT
	{
		return std::minmax(GetColumnList().GetTotalSize(), sizeof(void*)).second;
	}

	ConstRowReference pvMakeConstRowReference(Raw* raw) const MOMO_NOEXCEPT
	{
		return ConstRowReferenceProxy(&GetColumnList(), raw);
	}

	RowReference pvMakeRowReference(Raw* raw) const MOMO_NOEXCEPT
	{
		return RowReferenceProxy(&GetColumnList(), raw);
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

	Row pvNewRow(Raw* raw) MOMO_NOEXCEPT
	{
		pvFreeNewRaws();
		return RowProxy(&GetColumnList(), raw, &mCrew.GetFreeRaws());
	}

	template<typename Type, typename TypeArg, typename... Args>
	void pvFillRaw(Raw* raw, const Column<Type>& column, TypeArg&& itemArg, Args&&... args)
	{
		size_t offset = GetColumnList().GetOffset(column);
		GetColumnList().template Assign<Type>(raw, offset, std::forward<TypeArg>(itemArg));
		pvFillRaw(raw, std::forward<Args>(args)...);
	}

	void pvFillRaw(Raw* /*raw*/) MOMO_NOEXCEPT
	{
	}

	void pvSetNumber(Raw* raw, size_t number, std::true_type /*keepRowNumber*/) MOMO_NOEXCEPT
	{
		GetColumnList().SetNumber(raw, number);
	}

	void pvSetNumber(Raw* /*raw*/, size_t /*number*/, std::false_type /*keepRowNumber*/) MOMO_NOEXCEPT
	{
	}

	template<typename Result, typename Filter, typename Type, typename... Args>
	Result pvSelect(const Filter& filter, const Column<Type>& column, const Type& item,
		const Args&... args) const
	{
		static const size_t columnCount = 1 + sizeof...(Args) / 2;
		std::array<size_t, columnCount> offsets;
		pvGetOffsets(offsets.data(), column, item, args...);
		std::array<size_t, columnCount> sortedOffsets = Indexes::GetSortedOffsets(offsets);
		const UniqueHashIndex* uniqueHash = mIndexes.GetFitUniqueHash(sortedOffsets);
		if (uniqueHash != nullptr)
			return pvSelectRec<Result>(*uniqueHash, offsets.data(), filter, OffsetItemTuple<>(), column, item, args...);
		const MultiHashIndex* multiHash = mIndexes.GetFitMultiHash(sortedOffsets);
		if (multiHash != nullptr)
			return pvSelectRec<Result>(*multiHash, offsets.data(), filter, OffsetItemTuple<>(), column, item, args...);
		auto newFilter = [&offsets, &filter, &column, &item, &args...] (ConstRowReference rowRef)
			{ return pvIsSatisfied(rowRef, offsets.data(), column, item, args...) && filter(rowRef); };
		return pvMakeSelection(mRaws, newFilter, static_cast<Result*>(nullptr));
	}

	template<typename Type, typename... Args>
	void pvGetOffsets(size_t* offsets, const Column<Type>& column, const Type& /*item*/,
		const Args&... args) const
	{
		*offsets = GetColumnList().GetOffset(column);
		pvGetOffsets(offsets + 1, args...);
	}

	void pvGetOffsets(size_t* /*offsets*/) const MOMO_NOEXCEPT
	{
	}

	template<typename Type, typename... Args>
	static bool pvIsSatisfied(ConstRowReference rowRef, const size_t* offsets,
		const Column<Type>& /*column*/, const Type& item, const Args&... args)
	{
		return DataTraits::IsEqual(rowRef.template GetByOffset<Type>(*offsets), item)
			&& pvIsSatisfied(rowRef, offsets + 1, args...);
	}

	static bool pvIsSatisfied(ConstRowReference /*rowRef*/, const size_t* /*offsets*/) MOMO_NOEXCEPT
	{
		return true;
	}

	template<typename Result, typename Index, typename Filter, typename Tuple, typename Type, typename... Args>
	Result pvSelectRec(const Index& index, const size_t* offsets, const Filter& filter, const Tuple& tuple,
		const Column<Type>& /*column*/, const Type& item, const Args&... args) const
	{
		size_t offset = *offsets;
		if (Indexes::HasOffset(index, offset))
		{
			auto newTuple = std::tuple_cat(tuple,
				std::make_tuple(std::pair<size_t, const Type&>(offset, item)));
			return pvSelectRec<Result>(index, offsets + 1, filter, newTuple, args...);
		}
		else
		{
			auto newFilter = [&filter, offset, &item] (ConstRowReference rowRef)
			{
				return DataTraits::IsEqual(rowRef.template GetByOffset<Type>(offset), item)
					&& filter(rowRef);
			};
			return pvSelectRec<Result>(index, offsets + 1, newFilter, tuple, args...);
		}
	}

	template<typename Result, typename Index, typename Filter, typename Tuple>
	Result pvSelectRec(const Index& index, const size_t* /*offsets*/, const Filter& filter,
		const Tuple& tuple) const
	{
		return pvMakeSelection(mIndexes.FindRaws(index, tuple), filter, static_cast<Result*>(nullptr));
	}

#ifdef _MSC_VER	//?
	template<typename Result, typename Index, typename Filter>
	Result pvSelectRec(const Index&, const size_t*, const Filter&, const OffsetItemTuple<>&) const
	{
		throw std::exception();
	}
#endif

	template<typename Raws, typename Filter>
	Selection pvMakeSelection(const Raws& raws, const Filter& filter, Selection*) const
	{
		MemManager memManager = GetMemManager();
		typename SelectionProxy::Raws selRaws(std::move(memManager));
		for (Raw* raw : raws)
		{
			if (filter(pvMakeConstRowReference(raw)))
				selRaws.AddBack(raw);
		}
		return SelectionProxy(&GetColumnList(), std::move(selRaws));
	}

	template<typename Raws>
	Selection pvMakeSelection(const Raws& raws, const EmptyFilter& /*filter*/, Selection*) const
	{
		MemManager memManager = GetMemManager();
		typename SelectionProxy::Raws selRaws(raws.GetBegin(), raws.GetEnd(), std::move(memManager));
		return SelectionProxy(&GetColumnList(), std::move(selRaws));
	}

	template<typename Raws, typename Filter>
	size_t pvMakeSelection(const Raws& raws, const Filter& filter, size_t*) const
	{
		return std::count_if(raws.GetBegin(), raws.GetEnd(),
			[this, &filter] (Raw* raw) { return filter(pvMakeConstRowReference(raw)); });
	}

	template<typename Raws>
	size_t pvMakeSelection(const Raws& raws, const EmptyFilter& /*filter*/, size_t*) const MOMO_NOEXCEPT
	{
		return std::distance(raws.GetBegin(), raws.GetEnd());
	}

	template<typename RowBounds>
	RowBounds pvFindByUniqueHash(const UniqueHashIndex* uniqueHashIndex, const Row& row) const
	{
		MOMO_CHECK(uniqueHashIndex != nullptr);
		MOMO_CHECK(&row.GetColumnList() == &GetColumnList());
		auto raws = mIndexes.FindRaws(*uniqueHashIndex, RowProxy::GetRaw(row));
		return RowBounds(&GetColumnList(), raws);
	}

	template<typename RowBounds, typename Index, typename Type, typename... Args>
	RowBounds pvFindByHash(const Index* index, const Column<Type>& column, const Type& item,
		const Args&... args) const
	{
		static const size_t columnCount = 1 + sizeof...(Args) / 2;
		std::array<size_t, columnCount> offsets;
		pvGetOffsets(offsets.data(), column, item, args...);
		if (index == nullptr)
			index = mIndexes.GetHash(Indexes::GetSortedOffsets(offsets), index);
		else
			MOMO_EXTRA_CHECK(index == mIndexes.GetHash(Indexes::GetSortedOffsets(offsets), index));	//?
		if (index == nullptr)
			throw std::runtime_error("Index not found");
		return pvFindByHashRec<RowBounds>(*index, offsets.data(), OffsetItemTuple<>(), column, item, args...);
	}

	template<typename RowBounds, typename Index, typename Tuple, typename Type, typename... Args>
	RowBounds pvFindByHashRec(const Index& index, const size_t* offsets, const Tuple& tuple,
		const Column<Type>& /*column*/, const Type& item, const Args&... args) const
	{
		auto newTuple = std::tuple_cat(tuple,
			std::make_tuple(std::pair<size_t, const Type&>(*offsets, item)));
		return pvFindByHashRec<RowBounds>(index, offsets + 1, newTuple, args...);
	}

	template<typename RowBounds, typename Index, typename Tuple>
	RowBounds pvFindByHashRec(const Index& index, const size_t* /*offsets*/, const Tuple& tuple) const
	{
		return RowBounds(&GetColumnList(), mIndexes.FindRaws(index, tuple));
	}

private:
	Crew mCrew;
	Raws mRaws;
	RawMemPool mRawMemPool;
	Indexes mIndexes;
};

} // namespace experimental

} // namespace momo
