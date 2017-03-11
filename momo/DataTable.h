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
	typedef typename ColumnList::Raw Raw;

	template<typename Type>
	using Column = typename ColumnList::template Column<Type>;

	typedef internal::DataRow<ColumnList> Row;
	typedef internal::DataRowRef<ColumnList> RowRef;
	typedef typename RowRef::ConstRowRef ConstRowRef;

	typedef internal::DataSelection<RowRef, MemManager> Selection;
	typedef typename Selection::ConstSelection ConstSelection;

private:
	typedef internal::DataIndexes<ColumnList, DataTraits> Indexes;

	typedef momo::internal::MemManagerPtr<MemManager> MemManagerPtr;

	typedef Array<Raw*, MemManagerPtr> Raws;

	//? MemPoolSettings
	typedef MemPool<typename DataTraits::RawMemPoolParams, MemManagerPtr> RawMemPool;

	typedef typename Selection::Raws SelectionRaws;

	template<typename... Types>
	using OffsetItemTuple = typename Indexes::template OffsetItemTuple<Types...>;

	struct EmptyFilter
	{
		bool operator()(ConstRowRef /*rowRef*/) const MOMO_NOEXCEPT
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
		: DataTable(ColumnList(table.GetColumnList()))
	{
		ColumnList& columnList = mCrew.GetColumnList();
		mRaws.Reserve(table.GetCount());
		try
		{
			for (const Raw* srcRaw : table.mRaws)
			{
				Raw* dstRaw = mRawMemPool.template Allocate<Raw>();
				try
				{
					columnList.CopyRaw(srcRaw, dstRaw);
					try
					{
						mIndexes.AddRaw(dstRaw);
					}
					catch (...)
					{
						columnList.DestroyRaw(dstRaw);
						throw;
					}
				}
				catch (...)
				{
					mRawMemPool.Deallocate(dstRaw);
					throw;
				}
				columnList.SetNumber(dstRaw, mRaws.GetCount());
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

	MOMO_FRIEND_SWAP(DataTable)

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
		pvFreeRaws();
		mRaws.Clear();
		mIndexes.Clear();
	}

	const ConstRowRef operator[](size_t index) const
	{
		return ConstRowRef(&GetColumnList(), mRaws[index]);
	}

	const RowRef operator[](size_t index)
	{
		return RowRef(&GetColumnList(), mRaws[index]);
	}

	template<typename Type, typename TypeArg, typename... Args>
	Row NewRow(const Column<Type>& column, TypeArg&& itemArg, Args&&... args)
	{
		Row row = NewRow();
		pvFillRaw(row.GetRaw(), column, std::forward<TypeArg>(itemArg), std::forward<Args>(args)...);
		return row;
	}

	Row NewRow()
	{
		pvFreeNewRaws();
		return Row(&GetColumnList(), pvCreateRaw(), &mCrew.GetFreeRaws());
	}

	template<typename Type, typename TypeArg, typename... Args>
	RowRef AddRow(const Column<Type>& column, TypeArg&& itemArg, Args&&... args)
	{
		return AddRow(NewRow(column, std::forward<TypeArg>(itemArg), std::forward<Args>(args)...));
	}

	RowRef AddRow()
	{
		return AddRow(NewRow());
	}

	RowRef AddRow(Row&& row)
	{
		const ColumnList& columnList = GetColumnList();
		MOMO_ASSERT(&row.GetColumnList() == &columnList);
		mRaws.Reserve(mRaws.GetCount() + 1);
		mIndexes.AddRaw(row.GetRaw());
		Raw* raw = row.ExtractRaw();
		columnList.SetNumber(raw, mRaws.GetCount());
		mRaws.AddBackNogrow(raw);
		return RowRef(&columnList, raw);
	}

	Row ExtractRow(ConstRowRef rowRef)
	{
		MOMO_ASSERT(&rowRef.GetColumnList() == &GetColumnList());
		return ExtractRow(rowRef.GetNumber());
	}
	
	Row ExtractRow(size_t rowNumber)
	{
		MOMO_ASSERT(rowNumber < GetCount());
		const ColumnList& columnList = GetColumnList();
		Raw* raw = mRaws[rowNumber];
		mIndexes.RemoveRaw(raw);
		Row row(&columnList, raw, &mCrew.GetFreeRaws());
		mRaws.Remove(rowNumber, 1);
		for (size_t i = rowNumber, count = mRaws.GetCount(); i < count; ++i)
			columnList.SetNumber(mRaws[i], i);
		return row;
	}

	RowRef UpdateRow(ConstRowRef rowRef, Row&& row)
	{
		MOMO_ASSERT(&rowRef.GetColumnList() == &GetColumnList());
		return UpdateRow(rowRef.GetNumber(), std::move(row));
	}

	RowRef UpdateRow(size_t rowNumber, Row&& row)
	{
		MOMO_ASSERT(rowNumber < GetCount());
		const ColumnList& columnList = GetColumnList();
		Raw*& raw = mRaws[rowNumber];
		mIndexes.UpdateRaw(raw, row.GetRaw());
		pvFreeRaw(raw);
		raw = row.ExtractRaw();
		columnList.SetNumber(raw, rowNumber);
		return RowRef(&columnList, raw);
	}

	template<typename... Types>
	bool HasUniqueHashIndex(const Column<Types>&... columns)
	{
		return mIndexes.HasUniqueHash(columns...);
	}

	template<typename... Types>
	bool HasMultiHashIndex(const Column<Types>&... columns)
	{
		return mIndexes.HasMultiHash(columns...);
	}

	template<typename... Types>
	bool AddUniqueHashIndex(const Column<Types>&... columns)
	{
		return mIndexes.AddUniqueHash(mRaws, columns...);
	}

	template<typename... Types>
	bool AddMultiHashIndex(const Column<Types>&... columns)
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
		return pvSelectRec<Selection>(mRaws, filter);
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
		return pvSelectRec<Selection>(mRaws, filter);
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
		return pvSelectRec<size_t>(mRaws, filter);
	}

	//FindUnique
	//FindMulti

private:
	size_t pvGetRawSize() const MOMO_NOEXCEPT
	{
		return std::minmax(GetColumnList().GetTotalSize(), sizeof(void*)).second;
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

	template<typename Result, typename Filter, typename Type, typename... Args>
	Result pvSelect(const Filter& filter, const Column<Type>& column, const Type& item,
		const Args&... args) const
	{
		static const size_t columnCount = 1 + sizeof...(Args) / 2;
		std::array<size_t, columnCount> offsets;
		pvGetOffsets(offsets.data(), column, item, args...);
		std::array<size_t, columnCount> sortedOffsets = Indexes::GetSortedOffsets(offsets);
		const auto* uniqueHash = mIndexes.FindFitUniqueHash(sortedOffsets);
		if (uniqueHash != nullptr)
			return pvSelectRec<Result>(*uniqueHash, filter, OffsetItemTuple<>(), column, item, args...);
		const auto* multiHash = mIndexes.FindFitMultiHash(sortedOffsets);
		if (multiHash != nullptr)
			return pvSelectRec<Result>(*multiHash, filter, OffsetItemTuple<>(), column, item, args...);
		auto newFilter = [&offsets, &filter, &column, &item, &args...] (ConstRowRef rowRef)
			{ return pvIsSatisfied(rowRef, offsets.data(), column, item, args...) && filter(rowRef); };
		return pvSelectRec<Result>(mRaws, newFilter);
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
	static bool pvIsSatisfied(ConstRowRef rowRef, const size_t* offsets,
		const Column<Type>& /*column*/, const Type& item, const Args&... args)
	{
		return DataTraits::IsEqual(rowRef.template GetByOffset<Type>(*offsets), item)
			&& pvIsSatisfied(rowRef, offsets + 1, args...);
	}

	static bool pvIsSatisfied(ConstRowRef /*rowRef*/, const size_t* /*offsets*/) MOMO_NOEXCEPT
	{
		return true;
	}

	template<typename Result, typename Index, typename Filter, typename Tuple, typename Type, typename... Args>
	Result pvSelectRec(const Index& index, const Filter& filter, const Tuple& tuple,
		const Column<Type>& column, const Type& item, const Args&... args) const
	{
		size_t offset = GetColumnList().GetOffset(column);
		if (Indexes::HasOffset(index, offset))
		{
			auto newTuple = std::tuple_cat(tuple,
				std::make_tuple(std::pair<size_t, const Type&>(offset, item)));
			return pvSelectRec<Result>(index, filter, newTuple, args...);
		}
		else
		{
			auto newFilter = [&filter, offset, &item] (ConstRowRef rowRef)
			{
				return DataTraits::IsEqual(rowRef.template GetByOffset<Type>(offset), item)
					&& filter(rowRef);
			};
			return pvSelectRec<Result>(index, newFilter, tuple, args...);
		}
	}

	template<typename Result, typename Index, typename Filter, typename Tuple>
	Result pvSelectRec(const Index& index, const Filter& filter, const Tuple& tuple) const
	{
		return pvSelectRec<Result>(mIndexes.FindRaws(index, tuple), filter);
	}

#ifdef _MSC_VER	//?
	template<typename Result, typename Index, typename Filter>
	Result pvSelectRec(const Index&, const Filter&, const OffsetItemTuple<>&) const
	{
		throw std::exception();
	}
#endif

	template<typename Result, typename Raws, typename Filter>
	Result pvSelectRec(const Raws& raws, const Filter& filter) const
	{
		return pvSelectRec(raws, filter, static_cast<Result*>(nullptr));
	}

	template<typename Raws, typename Filter>
	Selection pvSelectRec(const Raws& raws, const Filter& filter, Selection*) const
	{
		const ColumnList* columnList = &GetColumnList();
		MemManager memManager = GetMemManager();
		SelectionRaws selRaws(std::move(memManager));
		for (Raw* raw : raws)
		{
			if (filter(ConstRowRef(columnList, raw)))
				selRaws.AddBack(raw);
		}
		return Selection(columnList, std::move(selRaws));
	}

	template<typename Raws>
	Selection pvSelectRec(const Raws& raws, const EmptyFilter& /*filter*/, Selection*) const
	{
		MemManager memManager = GetMemManager();
		return Selection(&GetColumnList(),
			SelectionRaws(raws.GetBegin(), raws.GetEnd(), std::move(memManager)));
	}

	template<typename Raws, typename Filter>
	size_t pvSelectRec(const Raws& raws, const Filter& filter, size_t*) const
	{
		const ColumnList* columnList = &GetColumnList();
		return std::count_if(raws.GetBegin(), raws.GetEnd(),
			[&filter, columnList] (Raw* raw) { return filter(ConstRowRef(columnList, raw)); });
	}

	template<typename Raws>
	size_t pvSelectRec(const Raws& raws, const EmptyFilter& /*filter*/, size_t*) const MOMO_NOEXCEPT
	{
		return std::distance(raws.GetBegin(), raws.GetEnd());
	}

private:
	Crew mCrew;
	Raws mRaws;
	RawMemPool mRawMemPool;
	Indexes mIndexes;
};

} // namespace experimental

} // namespace momo
