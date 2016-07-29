/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/DataTable.h

  namespace momo::experimental:
    struct DataTraits
    class DataTable

\**********************************************************/

#pragma once

#include "DataColumn.h"
#include "DataSelection.h"
#include "DataIndexes.h"

namespace momo
{

namespace experimental
{

struct DataTraits
{
	typedef MemPoolParamsVar<> RawMemPoolParams;

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
	typename TDataTraits = DataTraits,
	typename TMemManager = MemManagerDefault>
class DataTable
{
public:
	typedef TColumnList ColumnList;
	typedef TDataTraits DataTraits;
	typedef TMemManager MemManager;
	typedef typename ColumnList::Raw Raw;

	template<typename Type>
	using Column = typename ColumnList::template Column<Type>;

	typedef internal::DataRow<ColumnList> Row;
	typedef internal::DataRowRef<ColumnList> RowRef;
	typedef internal::DataConstRowRef<ColumnList> ConstRowRef;
	typedef internal::DataConstSelection<ColumnList, MemManager> ConstSelection;

private:
	typedef internal::DataIndexes<ColumnList, DataTraits, MemManager> Indexes;

	typedef momo::internal::MemManagerPtr<MemManager> MemManagerPtr;

	struct RawsSettings : public ArraySettings<>
	{
		static const CheckMode checkMode = CheckMode::assertion;
	};
	typedef Array<Raw*, MemManagerPtr, ArrayItemTraits<Raw*>, RawsSettings> Raws;

	//? MemPoolSettings
	typedef MemPool<typename DataTraits::RawMemPoolParams, MemManagerPtr> RawMemPool;

	typedef typename ConstSelection::Raws SelectionRaws;

	template<typename... Types>
	using OffsetItemTuple = typename Indexes::template OffsetItemTuple<Types...>;

	struct EmptyFilter
	{
		bool operator()(ConstRowRef /*rowRef*/) const noexcept
		{
			return true;
		}
	};

	class Crew
	{
	private:
		struct Data
		{
			ColumnList columnList;
			MemManager memManager;
			std::atomic<Raw*> freeRaws;
		};

	public:
		Crew(const ColumnList& columnList, MemManager&& memManager)
		{
			mData = memManager.template Allocate<Data>(sizeof(Data));
			mData->freeRaws = nullptr;
			try
			{
				new(&mData->columnList) ColumnList(columnList);
			}
			catch (...)
			{
				memManager.Deallocate(mData, sizeof(Data));
				throw;
			}
			new(&mData->memManager) MemManager(std::move(memManager));
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
				mData->columnList.~ColumnList();
				MemManager memManager = std::move(GetMemManager());
				GetMemManager().~MemManager();
				memManager.Deallocate(mData, sizeof(Data));
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
			return mData->memManager;
		}

		MemManager& GetMemManager() noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->memManager;
		}

		std::atomic<Raw*>& GetFreeRaws() noexcept
		{
			MOMO_ASSERT(!IsNull());
			return mData->freeRaws;
		}

	private:
		Data* mData;
	};

public:
	explicit DataTable(const ColumnList& columnList = ColumnList(),
		MemManager&& memManager = MemManager())
		: mCrew(columnList, std::move(memManager)),
		mRaws(MemManagerPtr(GetMemManager())),
		mRawMemPool(typename RawMemPool::Params(_GetRawSize()), MemManagerPtr(GetMemManager())),
		mIndexes(&GetColumnList(), GetMemManager())
	{
	}

	DataTable(DataTable&& table) noexcept
		: mCrew(std::move(table.mCrew)),
		mRaws(std::move(table.mRaws)),
		mRawMemPool(std::move(table.mRawMemPool)),
		mIndexes(std::move(table.mIndexes))
	{
	}

	DataTable(const DataTable&) = delete;

	~DataTable() noexcept
	{
		_FreeRaws();
	}

	DataTable& operator=(DataTable&& table) noexcept
	{
		DataTable(std::move(table)).Swap(*this);
		return *this;
	}

	DataTable& operator=(const DataTable&) = delete;

	void Swap(DataTable& table) noexcept
	{
		mCrew.Swap(table.mCrew);
		mRaws.Swap(table.mRaws);
		mRawMemPool.Swap(table.mRawMemPool);
		mIndexes.Swap(table.mIndexes);
	}

	MOMO_FRIEND_SWAP(DataTable)

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
		_FreeRaws();
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

	template<typename Type, typename RType, typename... Args>
	Row NewRow(const Column<Type>& column, RType&& item, Args&&... args)
	{
		Row row = NewRow();
		_FillRaw(row.GetRaw(), column, std::forward<RType>(item), std::forward<Args>(args)...);
		return row;
	}

	Row NewRow()
	{
		_FreeNewRaws();
		return Row(&GetColumnList(), _CreateRaw(), &mCrew.GetFreeRaws());
	}

	template<typename Type, typename RType, typename... Args>
	RowRef AddRow(const Column<Type>& column, RType&& item, Args&&... args)
	{
		return AddRow(NewRow(column, std::forward<RType>(item), std::forward<Args>(args)...));
	}

	RowRef AddRow()
	{
		return AddRow(NewRow());
	}

	RowRef AddRow(Row&& row)
	{
		MOMO_ASSERT(&row.GetColumnList() == &GetColumnList());
		mRaws.Reserve(mRaws.GetCount() + 1);
		mIndexes.AddRaw(row.GetRaw());
		Raw* raw = row.ExtractRaw();
		mRaws.AddBackNogrow(raw);
		return RowRef(&GetColumnList(), raw);
	}

	void RemoveRow(size_t rowNumber, bool keepOrder = true)
	{
		MOMO_ASSERT(rowNumber < GetCount());
		Raw* raw = mRaws[rowNumber];
		mIndexes.RemoveRaw(raw);
		_FreeRaw(raw);
		if (keepOrder)
		{
			mRaws.Remove(rowNumber, 1);
		}
		else
		{
			mRaws[rowNumber] = mRaws.GetBackItem();
			mRaws.RemoveBack();
		}
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
		return _Select<ConstSelection>(filter, column, item, args...);
	}

	ConstSelection Select() const
	{
		return Select(EmptyFilter());
	}

	template<typename Filter>
	ConstSelection Select(const Filter& filter) const
	{
		return _MakeSelection<ConstSelection>(mRaws, filter);
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
		return _Select<size_t>(filter, column, item, args...);
	}

	size_t SelectCount() const
	{
		return SelectCount(EmptyFilter());
	}

	template<typename Filter>
	size_t SelectCount(const Filter& filter) const
	{
		return _MakeSelection<size_t>(mRaws, filter);
	}

	//FindUnique
	//FindMulti

private:
	size_t _GetRawSize() const noexcept
	{
		return std::minmax(GetColumnList().GetTotalSize(), sizeof(void*)).second;
	}

	Raw* _CreateRaw()
	{
		Raw* raw = mRawMemPool.template Allocate<Raw>();
		try
		{
			GetColumnList().CreateRaw(raw);
		}
		catch (...)
		{
			mRawMemPool.Deallocate(raw);
			throw;
		}
		return raw;
	}

	void _FreeRaws() noexcept
	{
		if (mCrew.IsNull())
			return;
		_FreeNewRaws();
		for (Raw* raw : mRaws)
			_FreeRaw(raw);
	}

	void _FreeRaw(Raw* raw) noexcept
	{
		GetColumnList().DestroyRaw(raw);
		mRawMemPool.Deallocate(raw);
	}

	void _FreeNewRaws() noexcept
	{
		Raw* headRaw = mCrew.GetFreeRaws().exchange(nullptr);
		while (headRaw != nullptr)
		{
			Raw* nextRaw = *reinterpret_cast<Raw**>(headRaw);
			mRawMemPool.Deallocate(headRaw);
			headRaw = nextRaw;
		}
	}

	template<typename Type, typename RType, typename... Args>
	void _FillRaw(Raw* raw, const Column<Type>& column, RType&& item, Args&&... args)
	{
		size_t offset = GetColumnList().GetOffset(column);
		GetColumnList().template Assign<Type>(raw, offset, std::forward<RType>(item));
		_FillRaw(raw, std::forward<Args>(args)...);
	}

	void _FillRaw(Raw* /*raw*/) noexcept
	{
	}

	template<typename Result, typename Filter, typename Type, typename... Args>
	Result _Select(const Filter& filter, const Column<Type>& column, const Type& item,
		const Args&... args) const
	{
		static const size_t columnCount = 1 + sizeof...(Args) / 2;
		size_t offsets[columnCount];
		_GetOffsets(offsets, column, item, args...);
		size_t sortedOffsets[columnCount];
		Indexes::GetSortedOffsets(offsets, sortedOffsets);
		const auto* uniqueHash = mIndexes.FindFitUniqueHash(sortedOffsets);
		if (uniqueHash != nullptr)
			return _SelectRec<Result>(*uniqueHash, filter, OffsetItemTuple<>(), column, item, args...);
		const auto* multiHash = mIndexes.FindFitMultiHash(sortedOffsets);
		if (multiHash != nullptr)
			return _SelectRec<Result>(*multiHash, filter, OffsetItemTuple<>(), column, item, args...);
		auto newFilter = [&offsets, &filter, &column, &item, &args...] (ConstRowRef rowRef)
			{ return _IsSatisfied(rowRef, offsets, column, item, args...) && filter(rowRef); };
		return _MakeSelection<Result>(mRaws, newFilter);
	}

	template<typename Type, typename... Args>
	void _GetOffsets(size_t* offsets, const Column<Type>& column, const Type& /*item*/,
		const Args&... args) const
	{
		*offsets = GetColumnList().GetOffset(column);
		_GetOffsets(offsets + 1, args...);
	}

	void _GetOffsets(size_t* /*offsets*/) const noexcept
	{
	}

	template<typename Type, typename... Args>
	static bool _IsSatisfied(ConstRowRef rowRef, const size_t* offsets,
		const Column<Type>& /*column*/, const Type& item, const Args&... args)
	{
		return DataTraits::IsEqual(rowRef.template GetByOffset<Type>(*offsets), item)
			&& _IsSatisfied(rowRef, offsets + 1, args...);
	}

	static bool _IsSatisfied(ConstRowRef /*rowRef*/, const size_t* /*offsets*/) noexcept
	{
		return true;
	}

	template<typename Result, typename Index, typename Filter, typename... Types, typename Type, typename... Args>
	Result _SelectRec(const Index& index, const Filter& filter, const OffsetItemTuple<Types...>& key,
		const Column<Type>& column, const Type& item, const Args&... args) const
	{
		size_t offset = GetColumnList().GetOffset(column);
		if (Indexes::HasOffset(index, offset))
		{
			OffsetItemTuple<Types..., Type> newKey = std::tuple_cat(key,
				std::make_tuple(std::pair<size_t, const Type&>(offset, item)));
			return _SelectRec<Result>(index, filter, newKey, args...);
		}
		else
		{
			auto newFilter = [&filter, offset, &item] (ConstRowRef rowRef)
			{
				return DataTraits::IsEqual(rowRef.template GetByOffset<Type>(offset), item)
					&& filter(rowRef);
			};
			return _SelectRec<Result>(index, newFilter, key, args...);
		}
	}

	template<typename Result, typename Index, typename Filter, typename... Types>
	Result _SelectRec(const Index& index, const Filter& filter,
		const OffsetItemTuple<Types...>& key) const
	{
		return _MakeSelection<Result>(mIndexes.FindRaws(index, key), filter);
	}

	template<typename Result, typename Raws, typename Filter>
	Result _MakeSelection(const Raws& raws, const Filter& filter) const
	{
		return _MakeSelection(raws, filter, static_cast<const Result*>(nullptr));
	}

	template<typename Raws, typename Filter>
	ConstSelection _MakeSelection(const Raws& raws, const Filter& filter, const ConstSelection*) const
	{
		const ColumnList* columnList = &GetColumnList();
		MemManager memManager = GetMemManager();
		SelectionRaws selRaws(std::move(memManager));
		for (Raw* raw : raws)
		{
			if (filter(ConstRowRef(columnList, raw)))
				selRaws.AddBack(raw);
		}
		return ConstSelection(columnList, std::move(selRaws));
	}

	template<typename Raws>
	ConstSelection _MakeSelection(const Raws& raws, const EmptyFilter& /*filter*/,
		const ConstSelection*) const
	{
		MemManager memManager = GetMemManager();
		return ConstSelection(&GetColumnList(),
			SelectionRaws(raws.GetBegin(), raws.GetEnd(), std::move(memManager)));
	}

	template<typename Raws, typename Filter>
	size_t _MakeSelection(const Raws& raws, const Filter& filter, const size_t*) const
	{
		const ColumnList* columnList = &GetColumnList();
		return std::count_if(raws.GetBegin(), raws.GetEnd(),
			[&filter, columnList] (Raw* raw) { return filter(ConstRowRef(columnList, raw)); });
	}

	template<typename Raws>
	size_t _MakeSelection(const Raws& raws, const EmptyFilter& /*filter*/, const size_t*) const noexcept
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
