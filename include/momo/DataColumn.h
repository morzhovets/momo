/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataColumn.h

  macros:
    MOMO_DATA_COLUMN_STRUCT
    MOMO_DATA_COLUMN_STRING_TAG
    MOMO_DATA_COLUMN_STRING

  namespace momo:
    concept conceptDataColumnList
    class DataSettings
    struct DataStructDefault
    class DataColumnTraits
    class DataItemTraits
    class DataColumnList
    class DataColumnListStatic

\**********************************************************/

#pragma once

#include "Array.h"
#include "HashSet.h"

#ifndef MOMO_DISABLE_TYPE_INFO
#include <typeinfo>
#endif

#define MOMO_DATA_COLUMN_STRUCT(Struct, name) \
	constexpr momo::internal::DataColumn<decltype(std::declval<Struct&>().name), Struct> \
	name(uint64_t{offsetof(Struct, name)}, #name)

#define MOMO_DATA_COLUMN_STRING_TAG(Tag, Type, name) \
	static_assert(!std::is_class_v<Tag> || std::is_empty_v<Tag>); \
	constexpr momo::internal::DataColumn<Type, Tag> \
	name(momo::internal::StrHasher::GetHashCode64(#name), #name)

#define MOMO_DATA_COLUMN_STRING(Type, name) \
	MOMO_DATA_COLUMN_STRING_TAG(momo::DataStructDefault<>, Type, name)

namespace momo
{

namespace internal
{
	template<typename TColumn, typename TItemArg>
	class DataOperator
	{
	public:
		typedef TColumn Column;
		typedef TItemArg ItemArg;

	public:
		explicit DataOperator(const Column& column, ItemArg&& itemArg) noexcept
			: mColumn(column),
			mItemArg(std::forward<ItemArg>(itemArg))
		{
		}

		DataOperator(const DataOperator&) = delete;

		~DataOperator() noexcept = default;

		DataOperator& operator=(const DataOperator&) = delete;

		const Column& GetColumn() const noexcept
		{
			return mColumn;
		}

		ItemArg&& GetItemArg() const noexcept
		{
			return std::forward<ItemArg>(mItemArg);
		}

	private:
		const Column& mColumn;
		ItemArg&& mItemArg;
	};

	template<typename TColumn, typename TItemArg>
	class DataEqualer : public DataOperator<TColumn, TItemArg>
	{
	private:
		typedef DataOperator<TColumn, TItemArg> Operator;

	public:
		using Operator::Operator;
	};

	template<typename TColumn, typename TItemArg>
	class DataAssigner : public DataOperator<TColumn, TItemArg>
	{
	private:
		typedef DataOperator<TColumn, TItemArg> Operator;

	public:
		using Operator::Operator;
	};

	template<typename Item>
	struct DataMutable
	{
	};

	template<typename TItem, typename TStruct,
		typename TCode = uint64_t>
	class DataColumn : public DataColumn<DataMutable<TItem>, TStruct, TCode>
	{
	public:
		typedef TItem Item;
		typedef TStruct Struct;
		typedef TCode Code;

		typedef DataColumn BaseColumn;

		typedef DataColumn<DataMutable<Item>, Struct, Code> MutableColumn;

		typedef DataEqualer<DataColumn, const Item&> Equaler;

		template<typename ItemArg>
		using Assigner = DataAssigner<DataColumn, ItemArg>;

	public:
		constexpr explicit DataColumn(Code code, const char* name = "") noexcept
			: mCode(code),
			mName(name)
		{
		}

		//DataColumn(const DataColumn&) = delete;

		//~DataColumn() noexcept = default;

		//DataColumn& operator=(const DataColumn&) = delete;

		const BaseColumn& GetBaseColumn() const noexcept
		{
			return *this;
		}

		constexpr Code GetCode() const noexcept
		{
			return mCode;
		}

		constexpr const char* GetName() const noexcept
		{
			return mName;
		}

		constexpr const MutableColumn& Mutable() const noexcept
		{
			return *this;
		}

		constexpr bool IsMutable() const noexcept
		{
			return false;
		}

		Equaler operator==(const Item& item) const noexcept
		{
			return Equaler(*this, item);
		}

		template<typename ItemArg>
		Assigner<ItemArg> operator=(ItemArg&& itemArg) const noexcept
		{
			return Assigner<ItemArg>(*this, std::forward<ItemArg>(itemArg));
		}

	private:
		Code mCode;
		const char* mName;
	};

	template<typename TItem, typename TStruct, typename TCode>
	class DataColumn<DataMutable<TItem>, TStruct, TCode>
	{
	public:
		typedef TItem Item;
		typedef TStruct Struct;
		typedef TCode Code;

		typedef DataColumn<Item, Struct, Code> BaseColumn;

	public:
		const BaseColumn& GetBaseColumn() const noexcept
		{
			return static_cast<const BaseColumn&>(*this);
		}

		constexpr bool IsMutable() const noexcept
		{
			return true;
		}

	protected:
		explicit DataColumn() noexcept = default;

		DataColumn(const DataColumn&) noexcept = default;

		~DataColumn() noexcept = default;

		DataColumn& operator=(const DataColumn&) noexcept = default;
	};

	template<typename DataPtrVisitor, typename Void, typename ColumnInfo>
	concept conceptDataPtrVisitor =
		std::is_invocable_v<const DataPtrVisitor&, Void*> ||
		std::is_invocable_v<const DataPtrVisitor&, Void*, ColumnInfo>;

	template<typename TStruct, typename TCode>
	class DataColumnInfoBase
	{
	public:
		typedef TStruct Struct;
		typedef TCode Code;

		template<typename Item>
		using Column = DataColumn<Item, Struct, Code>;

	public:
		Code GetCode() const noexcept
		{
			return mCode;
		}

		const char* GetName() const noexcept
		{
			return mName;
		}

	protected:
		explicit DataColumnInfoBase(Code code, const char* name) noexcept
			: mCode(code),
			mName(name)
		{
		}

		template<typename ColumnInfo, typename Item, typename PtrVisitor>
		void ptVisit(Item* item, const PtrVisitor& ptrVisitor) const
		{
			if constexpr (std::is_invocable_v<const PtrVisitor&, Item*, ColumnInfo>)
				ptrVisitor(item, *static_cast<const ColumnInfo*>(this));
			else
				ptrVisitor(item);
		}

	private:
		Code mCode;
		const char* mName;
	};

#ifdef MOMO_DISABLE_TYPE_INFO
	template<typename TStruct,
		typename TCode = uint64_t>
	class DataColumnInfo : public DataColumnInfoBase<TStruct, TCode>
	{
	private:
		typedef DataColumnInfoBase<TStruct, TCode> ColumnInfoBase;

	public:
		template<typename Item>
		using Column = typename ColumnInfoBase::template Column<Item>;

		typedef std::tuple<> VisitableItems;

	public:
		template<typename Item>
		DataColumnInfo(const Column<Item>& column) noexcept
			: ColumnInfoBase(column.GetCode(), column.GetName())
		{
		}

		template<conceptDataPtrVisitor<const void, DataColumnInfo> PtrVisitor>
		void Visit(const void* item, const PtrVisitor& ptrVisitor) const
		{
			ColumnInfoBase::template ptVisit<DataColumnInfo>(item, ptrVisitor);
		}

		template<conceptDataPtrVisitor<void, DataColumnInfo> PtrVisitor>
		void Visit(void* item, const PtrVisitor& ptrVisitor) const
		{
			ColumnInfoBase::template ptVisit<DataColumnInfo>(item, ptrVisitor);
		}
	};
#else
	template<typename Struct>
	struct DataVisitableItemsGetter
	{
		typedef std::tuple<> VisitableItems;
	};

	template<typename Struct>
	requires requires { typename Struct::VisitableItems; }
	struct DataVisitableItemsGetter<Struct>
	{
		typedef typename Struct::VisitableItems VisitableItems;
	};

	template<typename TStruct,
		typename TCode = uint64_t>
	class DataColumnInfo : public DataColumnInfoBase<TStruct, TCode>
	{
	private:
		typedef DataColumnInfoBase<TStruct, TCode> ColumnInfoBase;

	public:
		using typename ColumnInfoBase::Struct;

		template<typename Item>
		using Column = typename ColumnInfoBase::template Column<Item>;

		typedef typename DataVisitableItemsGetter<Struct>::VisitableItems VisitableItems;

	public:
		template<typename Item>
		DataColumnInfo(const Column<Item>& column) noexcept
			: ColumnInfoBase(column.GetCode(), column.GetName()),
			mTypeInfo(typeid(Item))
		{
		}

		const std::type_info& GetTypeInfo() const noexcept
		{
			return mTypeInfo;
		}

		template<conceptDataPtrVisitor<const void, DataColumnInfo> PtrVisitor>
		void Visit(const void* item, const PtrVisitor& ptrVisitor) const
		{
			pvVisitRec<0>(item, ptrVisitor);
		}

		template<conceptDataPtrVisitor<void, DataColumnInfo> PtrVisitor>
		void Visit(void* item, const PtrVisitor& ptrVisitor) const
		{
			pvVisitRec<0>(item, ptrVisitor);
		}

	private:
		template<size_t index, typename Void, typename PtrVisitor>
		void pvVisitRec(Void* item, const PtrVisitor& ptrVisitor) const
		{
			if constexpr (index < std::tuple_size_v<VisitableItems>)
			{
				typedef std::tuple_element_t<index, VisitableItems> Item;
				typedef std::conditional_t<std::is_const_v<Void>, const Item*, Item*> ItemPtr;
				if (typeid(Item) == mTypeInfo)
					ColumnInfoBase::template ptVisit<DataColumnInfo>(static_cast<ItemPtr>(item), ptrVisitor);
				else
					pvVisitRec<index + 1>(item, ptrVisitor);
			}
			else
			{
				return ColumnInfoBase::template ptVisit<DataColumnInfo>(item, ptrVisitor);
			}
		}

	private:
		const std::type_info& mTypeInfo;
	};
#endif
}

template<typename DataColumnList,
	typename TestItem = void*>
concept conceptDataColumnList =
	std::is_nothrow_destructible_v<DataColumnList> &&
	std::is_nothrow_move_constructible_v<DataColumnList> &&
	conceptMemManager<typename DataColumnList::MemManager> &&
	requires (DataColumnList& columnList, typename DataColumnList::Raw* raw,
		typename DataColumnList::MemManager& memManager,
		const typename DataColumnList::template Column<TestItem>& column)
	{
		typename DataColumnList::Settings;
		{ columnList.GetMemManager() } noexcept
			-> std::same_as<typename DataColumnList::MemManager&>;
		{ std::as_const(columnList).GetTotalSize() } noexcept -> std::same_as<size_t>;
		{ std::as_const(columnList).GetAlignment() } noexcept -> std::same_as<size_t>;
		{ std::as_const(columnList).CreateRaw(memManager, raw) } -> std::same_as<void>;
		{ std::as_const(columnList).DestroyRaw(&memManager, raw) } noexcept -> std::same_as<void>;
		{ std::as_const(columnList).GetOffset(column) } -> std::same_as<size_t>;
		{ DataColumnList::template GetByOffset<TestItem>(raw, size_t{}) } noexcept
			-> std::same_as<TestItem&>;
	};

template<bool tKeepRowNumber = false>
class DataSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;

	static const bool keepRowNumber = tKeepRowNumber;

	typedef size_t RowNumber;	//?

	typedef ArraySettings<> TableRawsSettings;
	typedef ArraySettings<4, true, true> SelectionRawsSettings;
};

template<typename... TVisitableItems>
struct DataStructDefault
{
	typedef std::tuple<TVisitableItems...> VisitableItems;
};

template<typename TStruct = DataStructDefault<>,
	size_t tLogVertexCount = 8>
requires (4 <= tLogVertexCount && tLogVertexCount < 16)
class DataColumnTraits
{
public:
	typedef TStruct Struct;

	static const size_t logVertexCount = tLogVertexCount;

	static const size_t maxColumnCount = size_t{1} << (logVertexCount - 1);
	static const size_t maxCodeParam = 255;

	typedef internal::DataColumnInfo<Struct> ColumnInfo;

	template<typename Item>
	using Column = typename ColumnInfo::template Column<Item>;

	template<typename Item>
	using QualifiedColumn = Column<Item>;

	typedef typename ColumnInfo::Code ColumnCode;

	typedef HashTraitsOpen<ColumnCode> ColumnCodeHashTraits;

public:
	MOMO_FORCEINLINE static std::pair<size_t, size_t> GetVertices(ColumnCode columnCode,
		size_t codeParam) noexcept
	{
		static const size_t vertexCount1 = (size_t{1} << logVertexCount) - 1;
		size_t shortCode = static_cast<size_t>(columnCode + (columnCode >> 32));
		shortCode += shortCode >> 16;
		if constexpr (logVertexCount < 8)
			shortCode += shortCode >> 8;
		size_t vertex1 = (shortCode & vertexCount1) ^ (codeParam >> 4);
		size_t vertex2 = ((shortCode >> logVertexCount) & vertexCount1) ^ (codeParam & 15);
		vertex2 ^= (vertex1 == vertex2) ? 1 : 0;
		return { vertex1, vertex2 };
	}

	template<typename Item>
	static const typename QualifiedColumn<Item>::BaseColumn& GetBaseColumn(
		const QualifiedColumn<Item>& column) noexcept
	{
		return column.GetBaseColumn();
	}

	template<typename Item>
	static bool IsMutable(const QualifiedColumn<Item>& column) noexcept
	{
		return column.IsMutable();
	}
};

template<conceptMemManager TMemManager>
class DataItemTraits
{
public:
	typedef TMemManager MemManager;

private:
	template<typename Item>
	using ItemManager = internal::ObjectManager<Item, MemManager>;

public:
	template<typename Item>
	static constexpr size_t GetSize() noexcept
	{
		return sizeof(Item);
	}

	template<typename Item>
	static constexpr size_t GetAlignment() noexcept
	{
		return ItemManager<Item>::alignment;
	}

	template<typename Item>
	static void Create(MemManager& memManager, Item* item)
	{
		(typename ItemManager<Item>::template Creator<>(memManager))(item);
	}

	template<typename Item>
	static void Destroy(MemManager* memManager, Item& item) noexcept
	{
		ItemManager<Item>::Destroyer::Destroy(memManager, item);
	}

	template<typename Item>
	static void Copy(MemManager& memManager, const Item& srcItem, Item* dstItem)
	{
		ItemManager<Item>::Copy(memManager, srcItem, dstItem);
	}

	template<typename ItemArg, typename Item>
	static void Assign(ItemArg&& itemArg, Item& item)
	{
		item = std::forward<ItemArg>(itemArg);
	}
};

template<typename TColumnTraits = DataColumnTraits<>,
	conceptMemManager TMemManager = MemManagerDefault,
	typename TItemTraits = DataItemTraits<TMemManager>,
	typename TSettings = DataSettings<>>
class DataColumnList
{
public:
	typedef TColumnTraits ColumnTraits;
	typedef TMemManager MemManager;
	typedef TItemTraits ItemTraits;
	typedef TSettings Settings;

	typedef typename ColumnTraits::ColumnInfo ColumnInfo;

	template<typename Item>
	using Column = typename ColumnTraits::template Column<Item>;

	template<typename Item>
	using QualifiedColumn = typename ColumnTraits::template QualifiedColumn<Item>;

	typedef void Raw;

	class ColumnRecord : public ColumnInfo
	{
	public:
		explicit ColumnRecord(ColumnInfo columnInfo, size_t offset) noexcept	//?
			: ColumnInfo(columnInfo),
			mOffset(offset)
		{
		}

		size_t GetOffset() const noexcept
		{
			return mOffset;
		}

	private:
		size_t mOffset;
	};

private:
	static const size_t logVertexCount = ColumnTraits::logVertexCount;
	static const size_t maxColumnCount = ColumnTraits::maxColumnCount;
	static const size_t maxCodeParam = ColumnTraits::maxCodeParam;

	static const size_t vertexCount = size_t{1} << logVertexCount;

	// http://cmph.sourceforge.net/papers/chm92.pdf
	class Graph
	{
	private:
		struct Edge
		{
			size_t vertex;
			size_t value;
			Edge* nextEdge;
		};

		static const size_t maxEdgeCount = 2 * maxColumnCount;

	public:
		explicit Graph() noexcept
			: mEdgeNumber(0)
		{
			std::fill(mEdges.begin(), mEdges.end(), nullptr);
		}

		Graph(const Graph&) = delete;

		~Graph() noexcept = default;

		Graph& operator=(const Graph&) = delete;

		void AddEdges(size_t vertex1, size_t vertex2, size_t value) noexcept
		{
			MOMO_EXTRA_CHECK(vertex1 != vertex2);
			pvAddEdge(vertex1, vertex2, value);
			pvAddEdge(vertex2, vertex1, value);
		}

		bool HasEdge(size_t vertex) const noexcept
		{
			return mEdges[vertex] != nullptr;
		}

		bool FillAddends(size_t* addends, size_t vertex) const noexcept
		{
			size_t addend = addends[vertex];
			for (Edge* edge = mEdges[vertex]; edge != nullptr; edge = edge->nextEdge)
			{
				size_t vertex2 = edge->vertex;
				size_t& addend2 = addends[vertex2];
				if (addend2 == 0)
				{
					addend2 = edge->value - addend;
					if (!FillAddends(addends, vertex2))
						return false;
				}
				else
				{
					if (addend + addend2 != edge->value)
						return false;
				}
			}
			return true;
		}

	private:
		void pvAddEdge(size_t vertex1, size_t vertex2, size_t value) noexcept
		{
			MOMO_ASSERT(mEdgeNumber < vertexCount * 2);
			Edge* edge = &mEdgeStorage[mEdgeNumber];
			++mEdgeNumber;
			edge->vertex = vertex2;
			edge->value = value;
			edge->nextEdge = mEdges[vertex1];
			mEdges[vertex1] = edge;
		}

	private:
		size_t mEdgeNumber;
		std::array<Edge, maxEdgeCount> mEdgeStorage;
		std::array<Edge*, vertexCount> mEdges;
	};

	typedef std::array<size_t, vertexCount> Addends;

	typedef typename ColumnTraits::ColumnCode ColumnCode;

	typedef typename ColumnTraits::ColumnCodeHashTraits ColumnCodeHashTraits;

	typedef HashSet<ColumnCode, ColumnCodeHashTraits, MemManager,
		HashSetItemTraits<ColumnCode, MemManager>, internal::NestedHashSetSettings> ColumnCodeSet;

	typedef internal::MemManagerPtr<MemManager> MemManagerPtr;

	typedef internal::NestedArrayIntCap<0, ColumnRecord, MemManagerPtr> ColumnRecords;

	typedef void (*CreateFunc)(MemManager&, const ColumnRecord*, const DataColumnList*,
		const Raw*, Raw*);
	typedef void (*DestroyFunc)(MemManager*, const ColumnRecord*, Raw*) noexcept;

	struct FuncRecord
	{
		size_t columnIndex;
		CreateFunc createFunc;
		DestroyFunc destroyFunc;
	};

	typedef internal::NestedArrayIntCap<0, FuncRecord, MemManagerPtr> FuncRecords;

	typedef internal::NestedArrayIntCap<0, uint8_t, MemManagerPtr> MutableOffsets;

public:
	typedef typename ColumnRecords::ConstIterator ConstIterator;
	typedef ConstIterator Iterator;

public:
	DataColumnList()
		: DataColumnList(MemManager())
	{
	}

	explicit DataColumnList(MemManager memManager)
		: mCodeParam(0),
		mTotalSize(0),
		mAlignment(1),
		mColumnCodeSet(ColumnCodeHashTraits(), std::move(memManager)),
		mColumnRecords(MemManagerPtr(GetMemManager())),
		mFuncRecords(MemManagerPtr(GetMemManager())),
		mMutableOffsets(MemManagerPtr(GetMemManager()))
	{
		if constexpr (Settings::keepRowNumber)
			mTotalSize += sizeof(typename Settings::RowNumber);
		std::fill(mAddends.begin(), mAddends.end(), 0);
	}

	template<typename Item, typename... Items>
	DataColumnList(const QualifiedColumn<Item>& column, const QualifiedColumn<Items>&... columns)
		: DataColumnList()
	{
		Add(column, columns...);
	}

	DataColumnList(DataColumnList&& columnList) noexcept
		: mCodeParam(columnList.mCodeParam),
		mAddends(columnList.mAddends),
		mTotalSize(columnList.mTotalSize),
		mAlignment(columnList.mAlignment),
		mColumnCodeSet(std::move(columnList.mColumnCodeSet)),
		mColumnRecords(std::move(columnList.mColumnRecords)),
		mFuncRecords(std::move(columnList.mFuncRecords)),
		mMutableOffsets(std::move(columnList.mMutableOffsets))
	{
	}

	DataColumnList(const DataColumnList& columnList)
		: mCodeParam(columnList.mCodeParam),
		mAddends(columnList.mAddends),
		mTotalSize(columnList.mTotalSize),
		mAlignment(columnList.mAlignment),
		mColumnCodeSet(columnList.mColumnCodeSet),
		mColumnRecords(columnList.mColumnRecords, MemManagerPtr(GetMemManager())),
		mFuncRecords(columnList.mFuncRecords, MemManagerPtr(GetMemManager())),
		mMutableOffsets(columnList.mMutableOffsets, MemManagerPtr(GetMemManager()))
	{
	}

	~DataColumnList() noexcept = default;

	DataColumnList& operator=(const DataColumnList&) = delete;

	ConstIterator GetBegin() const noexcept
	{
		return mColumnRecords.GetBegin();
	}

	ConstIterator GetEnd() const noexcept
	{
		return mColumnRecords.GetEnd();
	}

	MOMO_FRIENDS_SIZE_BEGIN_END(DataColumnList)

	const MemManager& GetMemManager() const noexcept
	{
		return mColumnCodeSet.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mColumnCodeSet.GetMemManager();
	}

	size_t GetCount() const noexcept
	{
		return mColumnRecords.GetCount();
	}

	template<typename Item, typename... Items>
	void Add(const QualifiedColumn<Item>& column, const QualifiedColumn<Items>&... columns)
	{
		static const size_t columnCount = 1 + sizeof...(columns);
		std::array<bool, columnCount> columnMutables = {{ ColumnTraits::IsMutable(column),
			ColumnTraits::IsMutable(columns)... }};
		pvAdd(columnMutables.data(), GetBaseColumn(column), GetBaseColumn(columns)...);
	}

	template<typename Item>
	static decltype(auto) GetBaseColumn(const QualifiedColumn<Item>& column) noexcept
	{
		return ColumnTraits::GetBaseColumn(column);
	}

	bool IsMutable(size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < mTotalSize);
		return internal::UIntMath<uint8_t>::GetBit(mMutableOffsets.GetItems(), offset);
	}

	size_t GetTotalSize() const noexcept
	{
		return mTotalSize;
	}

	size_t GetAlignment() const noexcept
	{
		return mAlignment;
	}

	void CreateRaw(MemManager& memManager, Raw* raw) const
	{
		pvCreateRaw(memManager, nullptr, nullptr, raw);
	}

	void DestroyRaw(MemManager* memManager, Raw* raw) const noexcept
	{
		for (const FuncRecord& funcRec : mFuncRecords)
			funcRec.destroyFunc(memManager, &mColumnRecords[funcRec.columnIndex], raw);
	}

	void ImportRaw(MemManager& memManager, const DataColumnList& srcColumnList,
		const Raw* srcRaw, Raw* raw) const
	{
		pvCreateRaw(memManager, (&srcColumnList != this) ? &srcColumnList : nullptr, srcRaw, raw);
	}

	template<bool extraCheck = true>
	MOMO_FORCEINLINE size_t GetOffset(ColumnInfo columnInfo) const
	{
		ColumnCode columnCode = columnInfo.GetCode();
		MOMO_EXTRA_CHECK(!extraCheck || mColumnCodeSet.ContainsKey(columnCode));
		size_t offset = pvGetOffset(columnCode);
		MOMO_ASSERT(offset < mTotalSize);
		return offset;
	}

	template<typename Item>
	static Item& GetByOffset(Raw* raw, size_t offset) noexcept
	{
		//MOMO_ASSERT(offset < mTotalSize);
		//MOMO_ASSERT(offset % ItemTraits::template GetAlignment<Item>() == 0);
		return *internal::PtrCaster::Shift<Item>(raw, offset);
	}

	template<typename Item, typename ItemArg>
	void Assign(Raw* raw, size_t offset, ItemArg&& itemArg) const
	{
		ItemTraits::Assign(std::forward<ItemArg>(itemArg), GetByOffset<Item>(raw, offset));
	}

	size_t GetNumber(const Raw* raw) const noexcept
		requires (Settings::keepRowNumber)
	{
		return size_t{internal::MemCopyer::FromBuffer<typename Settings::RowNumber>(raw)};
	}

	void SetNumber(Raw* raw, size_t number) const noexcept
		requires (Settings::keepRowNumber)
	{
		auto rowNumber = static_cast<typename Settings::RowNumber>(number);
		MOMO_ASSERT(number == size_t{rowNumber});
		return internal::MemCopyer::ToBuffer(rowNumber, raw);
	}

	bool Contains(ColumnInfo columnInfo, size_t* resOffset = nullptr) const noexcept
	{
		ColumnCode columnCode = columnInfo.GetCode();
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(columnCode, mCodeParam);
		size_t addend1 = mAddends[vertices.first];
		size_t addend2 = mAddends[vertices.second];
		if (addend1 == 0 || addend2 == 0)
			return false;
		if (!mColumnCodeSet.ContainsKey(columnCode))
			return false;
		if (resOffset != nullptr)
			*resOffset = addend1 + addend2;
		return true;
	}

	template<internal::conceptDataPtrVisitor<const void, ColumnInfo> PtrVisitor>
	void VisitPointers(const Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers<const void>(raw, ptrVisitor);
	}

	template<internal::conceptDataPtrVisitor<void, ColumnInfo> PtrVisitor>
	void VisitPointers(Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers<void>(raw, ptrVisitor);
	}

private:
	template<typename... Items>
	void pvAdd(const bool* columnMutables, const Column<Items>&... columns)
	{
		static const size_t columnCount = sizeof...(columns);
		size_t initColumnCount = GetCount();
		if (columnCount + initColumnCount > maxColumnCount)
			throw std::logic_error("Too many columns");
		std::array<ColumnCode, columnCount> columnCodes = {{ ColumnInfo(columns).GetCode()... }};
		Addends addends;
		size_t offset;
		size_t maxAlignment;
		size_t codeParam = mCodeParam;
		while (true)
		{
			std::fill(addends.begin(), addends.end(), 0);
			offset = mTotalSize;
			maxAlignment = mAlignment;
			if (pvFillAddends<Items...>(addends, offset, maxAlignment, codeParam, columnCodes.data()))
				break;
			++codeParam;
			if (codeParam > maxCodeParam)
				throw std::runtime_error("Cannot add columns");
		}
		FuncRecord funcRec;
		funcRec.columnIndex = initColumnCount;
		funcRec.createFunc = [] (MemManager& memManager, const ColumnRecord* columnRecords,
			const DataColumnList* srcColumnList, const Raw* srcRaw, Raw* raw)
		{
			if (srcRaw == nullptr)
			{
				pvCreate<std::nullptr_t, std::nullptr_t, Items...>(memManager, columnRecords,
					nullptr, nullptr, raw);
			}
			else if (srcColumnList == nullptr)
			{
				pvCreate<std::nullptr_t, const Raw*, Items...>(memManager, columnRecords,
					nullptr, srcRaw, raw);
			}
			else
			{
				pvCreate<const DataColumnList*, const Raw*, Items...>(memManager, columnRecords,
					srcColumnList, srcRaw, raw);
			}
		};
		funcRec.destroyFunc = []
			(MemManager* memManager, const ColumnRecord* columnRecords, Raw* raw) noexcept
		{
			const ColumnRecord* columnRecordPtr = columnRecords;
			(ItemTraits::Destroy(memManager,
				*internal::PtrCaster::Shift<Items>(raw, (columnRecordPtr++)->GetOffset())), ...);
		};
		mColumnRecords.Reserve(initColumnCount + columnCount);
		mFuncRecords.Reserve(mFuncRecords.GetCount() + 1);
		mMutableOffsets.SetCount((offset + 7) / 8, uint8_t{0});
		try
		{
			mColumnCodeSet.Insert(columnCodes.begin(), columnCodes.end());
		}
		catch (...)
		{
			for (ColumnCode columnCode : columnCodes)
				mColumnCodeSet.Remove(columnCode);	// no throw
			//mMutableOffsets.SetCount((mTotalSize + 7) / 8);
			throw;
		}
		mCodeParam = codeParam;
		mAddends = addends;
		mTotalSize = offset;
		mAlignment = maxAlignment;
		pvAddColumns(columnCodes.data(), columnMutables, columns...);
		mFuncRecords.AddBackNogrow(std::move(funcRec));
	}

	template<typename... Items>
	bool pvFillAddends(Addends& addends, size_t& offset, size_t& maxAlignment, size_t codeParam,
		const ColumnCode* columnCodes) const
	{
		Graph graph;
		for (const ColumnRecord& columnRec : mColumnRecords)
		{
			std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(columnRec.GetCode(),
				codeParam);
			graph.AddEdges(vertices.first, vertices.second, columnRec.GetOffset());
		}
		return pvFillAddends<Items...>(addends, graph, offset, maxAlignment, codeParam, columnCodes);
	}

	template<typename... Items>
	static bool pvFillAddends(Addends& addends, Graph& graph, size_t& offset, size_t& maxAlignment,
		size_t codeParam, const ColumnCode* columnCodes)
	{
		const ColumnCode* columnCodePtr = columnCodes;
		(pvAddEdge<Items>(graph, offset, maxAlignment, codeParam, *columnCodePtr++), ...);
		for (size_t v = 0; v < vertexCount; ++v)
		{
			if (!graph.HasEdge(v) || addends[v] != 0)
				continue;
			addends[v] = size_t{1} << (8 * sizeof(size_t) - 1);
			if (!graph.FillAddends(addends.data(), v))
				return false;
		}
		return true;
	}

	template<typename Item>
	static void pvAddEdge(Graph& graph, size_t& offset, size_t& maxAlignment,
		size_t codeParam, ColumnCode columnCode)
	{
		static const size_t size = ItemTraits::template GetSize<Item>();
		static const size_t alignment = ItemTraits::template GetAlignment<Item>();
		static_assert(internal::ObjectAlignmenter<Item>::Check(alignment, size));
		offset = internal::UIntMath<>::Ceil(offset, alignment);
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(columnCode, codeParam);
		graph.AddEdges(vertices.first, vertices.second, offset);
		offset += size;
		maxAlignment = std::minmax(maxAlignment, size_t{alignment}).second;
	}

	template<typename... Items>
	void pvAddColumns(const ColumnCode* columnCodes, const bool* columnMutables,
		const Column<Items>&... columns) noexcept
	{
		const ColumnCode* columnCodePtr = columnCodes;
		const bool* columnMutablePtr = columnMutables;
		(pvAddColumn(*columnCodePtr++, *columnMutablePtr++, columns), ...);
	}

	template<typename Item>
	void pvAddColumn(ColumnCode columnCode, bool columnMutable, const Column<Item>& column) noexcept
	{
		size_t offset = pvGetOffset(columnCode);
		mColumnRecords.AddBackNogrow(ColumnRecord(column, offset));
		if (columnMutable)
			internal::UIntMath<uint8_t>::SetBit(mMutableOffsets.GetItems(), offset);
	}

	MOMO_FORCEINLINE size_t pvGetOffset(ColumnCode columnCode) const noexcept
	{
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(columnCode, mCodeParam);
		size_t addend1 = mAddends[vertices.first];
		size_t addend2 = mAddends[vertices.second];
		MOMO_ASSERT(addend1 != 0 && addend2 != 0);
		return addend1 + addend2;
	}

	template<typename DataColumnListPtr, typename RawPtr, typename Item, typename... Items>
	static void pvCreate(MemManager& memManager, const ColumnRecord* columnRecordPtr,
		DataColumnListPtr srcColumnList, RawPtr srcRaw, Raw* raw)
	{
		size_t offset = columnRecordPtr->GetOffset();
		Item* item = internal::PtrCaster::Shift<Item>(raw, offset);
		const Item* srcItem = nullptr;
		if constexpr (!std::is_null_pointer_v<RawPtr>)
		{
			if constexpr (std::is_null_pointer_v<DataColumnListPtr>)
			{
				srcItem = internal::PtrCaster::Shift<const Item>(srcRaw, offset);
			}
			else
			{
				size_t srcOffset;
				if (srcColumnList->Contains(*columnRecordPtr, &srcOffset))
					srcItem = internal::PtrCaster::Shift<const Item>(srcRaw, srcOffset);
			}
		}
		if (srcItem == nullptr)
			ItemTraits::Create(memManager, item);
		else
			ItemTraits::Copy(memManager, *srcItem, item);
		try
		{
			pvCreate<DataColumnListPtr, RawPtr, Items...>(memManager, columnRecordPtr + 1,
				srcColumnList, srcRaw, raw);
		}
		catch (...)
		{
			ItemTraits::Destroy(&memManager, *item);
			throw;
		}
	}

	template<typename DataColumnListPtr, typename RawPtr>
	static void pvCreate(MemManager& /*memManager*/, const ColumnRecord* /*columnRecordPtr*/,
		DataColumnListPtr /*srcColumnList*/, RawPtr /*srcRaw*/, Raw* /*raw*/) noexcept
	{
	}

	void pvCreateRaw(MemManager& memManager, const DataColumnList* srcColumnList,
		const Raw* srcRaw, Raw* raw) const
	{
		size_t funcIndex = 0;
		try
		{
			size_t funcCount = mFuncRecords.GetCount();
			for (; funcIndex < funcCount; ++funcIndex)
			{
				const FuncRecord& funcRec = mFuncRecords[funcIndex];
				funcRec.createFunc(memManager, &mColumnRecords[funcRec.columnIndex],
					srcColumnList, srcRaw, raw);
			}
		}
		catch (...)
		{
			for (size_t i = 0; i < funcIndex; ++i)
			{
				const FuncRecord& funcRec = mFuncRecords[i];
				funcRec.destroyFunc(&memManager, &mColumnRecords[funcRec.columnIndex], raw);
			}
			throw;
		}
	}

	template<typename Void, typename PtrVisitor>
	void pvVisitPointers(Void* raw, const PtrVisitor& ptrVisitor) const
	{
		for (const ColumnRecord& columnRec : mColumnRecords)
		{
			Void* item = internal::PtrCaster::Shift<Void>(raw, columnRec.GetOffset());
			columnRec.Visit(item, ptrVisitor);	//?
		}
	}

private:
	size_t mCodeParam;
	Addends mAddends;
	size_t mTotalSize;
	size_t mAlignment;
	ColumnCodeSet mColumnCodeSet;
	ColumnRecords mColumnRecords;
	FuncRecords mFuncRecords;
	MutableOffsets mMutableOffsets;
};

template<typename TStruct,
	conceptMemManager TMemManager = MemManagerDefault,
	typename TSettings = DataSettings<>>
requires std::is_class_v<TStruct>
class DataColumnListStatic
{
public:
	typedef TStruct Struct;
	typedef TMemManager MemManager;
	typedef TSettings Settings;

	typedef internal::DataColumnInfo<Struct> ColumnInfo;

	template<typename Item>
	using Column = typename ColumnInfo::template Column<Item>;

	typedef Struct Raw;

private:
	typedef internal::ObjectManager<Raw, MemManager> RawManager;

	typedef internal::NestedArrayIntCap<0, ColumnInfo, MemManager> VisitableColumns;

	typedef std::array<uint8_t, (sizeof(Struct) + 7) / 8> MutableOffsets;

public:
	explicit DataColumnListStatic(MemManager memManager = MemManager())
		: mVisitableColumns(std::move(memManager))
	{
		ResetMutable();
	}

	DataColumnListStatic(DataColumnListStatic&& columnList) noexcept
		: mVisitableColumns(std::move(columnList.mVisitableColumns)),
		mMutableOffsets(columnList.mMutableOffsets)
	{
	}

	DataColumnListStatic(const DataColumnListStatic& columnList)
		: mVisitableColumns(columnList.mVisitableColumns),
		mMutableOffsets(columnList.mMutableOffsets)
	{
	}

	~DataColumnListStatic() noexcept = default;

	DataColumnListStatic& operator=(const DataColumnListStatic&) = delete;

	const MemManager& GetMemManager() const noexcept
	{
		return mVisitableColumns.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mVisitableColumns.GetMemManager();
	}

	template<typename... Items>
	void SetMutable(const Column<Items>&... columns)
	{
		(pvSetMutable(GetOffset(columns)), ...);
	}

	void ResetMutable() noexcept
	{
		std::fill(mMutableOffsets.begin(), mMutableOffsets.end(), uint8_t{0});
	}

	bool IsMutable(size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < sizeof(Struct));
		return internal::UIntMath<uint8_t>::GetBit(mMutableOffsets.data(), offset);
	}

	size_t GetTotalSize() const noexcept
	{
		size_t totalSize = sizeof(Struct);
		if constexpr (Settings::keepRowNumber)
			totalSize += sizeof(typename Settings::RowNumber);
		return totalSize;
	}

	size_t GetAlignment() const noexcept
	{
		return RawManager::alignment;
	}

	void CreateRaw(MemManager& memManager, Raw* raw) const
	{
		(typename RawManager::template Creator<>(memManager))(raw);
	}

	void DestroyRaw(MemManager* memManager, Raw* raw) const noexcept
	{
		RawManager::Destroyer::Destroy(memManager, *raw);
	}

	void ImportRaw(MemManager& memManager, const DataColumnListStatic& /*srcColumnList*/,
		const Raw* srcRaw, Raw* raw) const
	{
		RawManager::Copy(memManager, *srcRaw, raw);
	}

	template<bool extraCheck = true>
	MOMO_FORCEINLINE size_t GetOffset(ColumnInfo columnInfo) const noexcept
	{
		size_t offset = pvGetOffset(columnInfo);
		MOMO_ASSERT(offset < sizeof(Struct));
		return offset;
	}

	template<typename Item>
	static Item& GetByOffset(Raw* raw, size_t offset) noexcept
	{
		//MOMO_ASSERT(offset < sizeof(Struct));
		//MOMO_ASSERT(offset % internal::ObjectAlignmenter<Item>::alignment == 0);
		return *internal::PtrCaster::Shift<Item>(raw, offset);
	}

	template<typename Item, typename ItemArg>
	void Assign(Raw* raw, size_t offset, ItemArg&& itemArg) const
	{
		GetByOffset<Item>(raw, offset) = std::forward<ItemArg>(itemArg);
	}

	size_t GetNumber(const Raw* raw) const noexcept
		requires (Settings::keepRowNumber)
	{
		return size_t{internal::MemCopyer::FromBuffer<typename Settings::RowNumber>(
			internal::PtrCaster::Shift<const void>(raw, sizeof(Struct)))};
	}

	void SetNumber(Raw* raw, size_t number) const noexcept
		requires (Settings::keepRowNumber)
	{
		auto rowNumber = static_cast<typename Settings::RowNumber>(number);
		MOMO_ASSERT(number == size_t{rowNumber});
		internal::MemCopyer::ToBuffer(rowNumber, internal::PtrCaster::Shift<void>(raw, sizeof(Struct)));
	}

	bool Contains(ColumnInfo columnInfo, size_t* resOffset = nullptr) const noexcept
	{
		if (resOffset != nullptr)
			*resOffset = pvGetOffset(columnInfo);
		return true;
	}

	template<typename... Items>
	void PrepareForVisitors(const Column<Items>&... columns)	//?
	{
		static const size_t columnCount = sizeof...(columns);
		mVisitableColumns.Reserve(columnCount);
		mVisitableColumns.Clear(false);
		(mVisitableColumns.AddBackNogrow(ColumnInfo(columns)), ...);
	}

	template<internal::conceptDataPtrVisitor<const void, ColumnInfo> PtrVisitor>
	void VisitPointers(const Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers<const void>(raw, ptrVisitor);
	}

	template<internal::conceptDataPtrVisitor<void, ColumnInfo> PtrVisitor>
	void VisitPointers(Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers<void>(raw, ptrVisitor);
	}

private:
	void pvSetMutable(size_t offset) noexcept
	{
		internal::UIntMath<uint8_t>::SetBit(mMutableOffsets.data(), offset);
	}

	MOMO_FORCEINLINE size_t pvGetOffset(ColumnInfo columnInfo) const noexcept
	{
		return static_cast<size_t>(columnInfo.GetCode());	//?
	}

	template<typename Void, typename PtrVisitor>
	void pvVisitPointers(Void* raw, const PtrVisitor& ptrVisitor) const
	{
		if (mVisitableColumns.IsEmpty())
			throw std::logic_error("Not prepared for visitors");
		for (const ColumnInfo& columnInfo : mVisitableColumns)
		{
			Void* item = internal::PtrCaster::Shift<Void>(raw, pvGetOffset(columnInfo));
			columnInfo.Visit(item, ptrVisitor);
		}
	}

private:
	VisitableColumns mVisitableColumns;
	MutableOffsets mMutableOffsets;
};

} // namespace momo