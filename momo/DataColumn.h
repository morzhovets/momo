/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataColumn.h

  macros:
    MOMO_DATA_COLUMN_STRUCT
    MOMO_DATA_COLUMN_STRING_TAG
    MOMO_DATA_COLUMN_STRING

  namespace momo:
    enum class DataOperatorType
    class DataOperator
    class DataColumn
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

#include <bitset>
#include <typeinfo>

#define MOMO_DATA_COLUMN_STRUCT(Struct, name) \
	constexpr momo::DataColumn<decltype(std::declval<Struct&>().name), Struct> \
	name(uint64_t{offsetof(Struct, name)}, #name)

#define MOMO_DATA_COLUMN_STRING_TAG(Tag, Type, name) \
	MOMO_STATIC_ASSERT(!std::is_class<Tag>::value || std::is_empty<Tag>::value); \
	constexpr momo::DataColumn<Type, Tag> name(momo::internal::StrHasher::GetHashCode64(#name), #name)

#define MOMO_DATA_COLUMN_STRING(Type, name) \
	MOMO_DATA_COLUMN_STRING_TAG(momo::DataStructDefault<>, Type, name)

namespace momo
{

namespace internal
{
	class StrHasher
	{
	private:
		static const uint64_t fnvBasis64 = 14695981039346656037ull;
		static const uint64_t fnvPrime64 = 1099511628211ull;

	public:
		// Fowler-Noll-Vo hash function (1a)
		constexpr static uint64_t GetHashCode64(const char* str) noexcept
		{
			return (*str == '\0') ? fnvBasis64
				: (GetHashCode64(str + 1) ^ uint64_t{static_cast<unsigned char>(*str)}) * fnvPrime64;
		}
	};

	template<typename Struct,
		typename = void>
	struct DataVisitableItemsGetter
	{
		typedef std::tuple<> VisitableItems;
	};

	template<typename Struct>
	struct DataVisitableItemsGetter<Struct, Void<typename Struct::VisitableItems>>
	{
		typedef typename Struct::VisitableItems VisitableItems;
	};

	class DataColumnRecord
	{
	public:
		template<typename VisitableItems, typename PtrVisitor>
		void Visit(const void* item, const PtrVisitor& ptrVisitor) const
		{
			pvVisit<0, VisitableItems>(item, ptrVisitor);
		}

		template<typename VisitableItems, typename PtrVisitor>
		void Visit(void* item, const PtrVisitor& ptrVisitor) const
		{
			pvVisit<0, VisitableItems>(item, ptrVisitor);
		}

	private:
		template<size_t index, typename VisitableItems, typename Void, typename PtrVisitor>
		internal::EnableIf<(index < std::tuple_size<VisitableItems>::value)> pvVisit(Void* item,
			const PtrVisitor& ptrVisitor) const
		{
			typedef typename std::tuple_element<index, VisitableItems>::type Item;
			typedef typename std::conditional<std::is_const<Void>::value,
				const Item*, Item*>::type ItemPtr;
			if (typeid(Item) == type)
				ptrVisitor(static_cast<ItemPtr>(item), name);
			else
				pvVisit<index + 1, VisitableItems>(item, ptrVisitor);
		}

		template<size_t index, typename VisitableItems, typename Void, typename PtrVisitor>
		internal::EnableIf<(index == std::tuple_size<VisitableItems>::value)> pvVisit(Void* item,
			const PtrVisitor& ptrVisitor) const
		{
			ptrVisitor(item, name);
		}

	public:
		const std::type_info& type;
		const char* name;
		size_t offset;
	};
}

enum class DataOperatorType
{
	equal,
	assign,
};

template<DataOperatorType tType, typename TColumn, typename TItemArg>
class DataOperator
{
public:
	typedef TColumn Column;
	typedef TItemArg ItemArg;

	static const DataOperatorType type = tType;

public:
	explicit DataOperator(const Column& column, ItemArg&& itemArg) noexcept
		: mColumn(column),
		mItemArg(std::forward<ItemArg>(itemArg))
	{
	}

#ifndef MOMO_HAS_GUARANTEED_COPY_ELISION
	DataOperator(DataOperator&& oper) noexcept
		: mColumn(oper.mColumn),
		mItemArg(std::forward<ItemArg>(oper.mItemArg))
	{
		//MOMO_ASSERT(false);
	}
#endif

	DataOperator(const DataOperator&) = delete;

	~DataOperator() noexcept
	{
	}

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

template<typename TItem, typename TStruct>
class DataColumn
{
public:
	typedef TItem Item;
	typedef TStruct Struct;

	typedef DataOperator<DataOperatorType::equal, DataColumn, const Item&> Equaler;

	template<typename ItemArg>
	using Assigner = DataOperator<DataOperatorType::assign, DataColumn, ItemArg>;

public:
	constexpr explicit DataColumn(uint64_t code, const char* name = "") noexcept
		: mCode(code),
		mName(name)
	{
	}

	constexpr uint64_t GetCode() const noexcept
	{
		return mCode;
	}

	constexpr const char* GetName() const noexcept
	{
		return mName;
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
	uint64_t mCode;
	const char* mName;
};

template<bool tKeepRowNumber = true>
class DataSettings
{
public:
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;
	static const bool checkVersion = MOMO_CHECK_ITERATOR_VERSION;

	static const bool keepRowNumber = tKeepRowNumber;

	typedef ArraySettings<> TableRawsSettings;
	typedef ArraySettings<4, true, true> SelectionRawsSettings;
};

template<typename... TVisitableItems>
struct DataStructDefault
{
	typedef std::tuple<TVisitableItems...> VisitableItems;
};

template<typename TStruct = DataStructDefault<>>
class DataColumnTraits
{
public:
	typedef TStruct Struct;

	template<typename Item>
	using Column = DataColumn<Item, Struct>;

	typedef uint64_t ColumnCode;

	static const size_t logVertexCount = 8;
	static const size_t maxColumnCount = 200;
	static const size_t maxCodeParam = 255;

	static const size_t mutableOffsetsInternalCapacity = 0;

	typedef HashTraitsOpen<ColumnCode> ColumnCodeHashTraits;

	typedef typename internal::DataVisitableItemsGetter<Struct>::VisitableItems VisitableItems;

public:
	template<typename Item>
	static ColumnCode GetColumnCode(const Column<Item>& column) noexcept
	{
		return column.GetCode();
	}

	template<typename Item>
	static const char* GetColumnName(const Column<Item>& column) noexcept
	{
		return column.GetName();
	}

	static std::pair<size_t, size_t> GetVertices(ColumnCode code, size_t codeParam) noexcept
	{
		static const size_t vertexCount1 = (size_t{1} << logVertexCount) - 1;
		size_t shortCode = static_cast<size_t>(code + (code >> 32));
		shortCode ^= (codeParam >> 4) ^ ((codeParam & 15) << 28);
		shortCode += shortCode >> 16;
		size_t vertex1 = shortCode & vertexCount1;
		size_t vertex2 = (shortCode >> logVertexCount) & vertexCount1;
		vertex2 ^= (vertex1 == vertex2) ? 1 : 0;
		return std::make_pair(vertex1, vertex2);
	}
};

template<typename TMemManager>
class DataItemTraits
{
public:
	typedef TMemManager MemManager;

private:
	template<typename Item>
	using ItemManager = internal::ObjectManager<Item, MemManager>;

public:
	template<typename Item>
	static size_t GetSize() noexcept
	{
		return sizeof(Item);
	}

	template<typename Item>
	static size_t GetAlignment() noexcept
	{
		return ItemManager<Item>::alignment;
	}

	template<typename Item>
	static void Create(MemManager& memManager, Item* item)
	{
		(typename ItemManager<Item>::template Creator<>(memManager))(item);
	}

	template<typename Item>
	static void Destroy(MemManager* memManager, Item* item) noexcept
	{
		ItemManager<Item>::Destroyer::Destroy(memManager, *item);
	}

	template<typename Item>
	static void Copy(MemManager& memManager, const Item* srcItem, Item* dstItem)
	{
		ItemManager<Item>::Copy(memManager, *srcItem, dstItem);
	}

	template<typename ItemArg, typename Item>
	static void Assign(ItemArg&& itemArg, Item& item)
	{
		item = std::forward<ItemArg>(itemArg);
	}
};

template<typename TColumnTraits = DataColumnTraits<>,
	typename TMemManager = MemManagerDefault,
	typename TItemTraits = DataItemTraits<TMemManager>,
	typename TSettings = DataSettings<>>
class DataColumnList
{
public:
	typedef TColumnTraits ColumnTraits;
	typedef TMemManager MemManager;
	typedef TItemTraits ItemTraits;
	typedef TSettings Settings;

	template<typename Item>
	using Column = typename ColumnTraits::template Column<Item>;

	typedef internal::DataColumnRecord ColumnRecord;

	typedef void Raw;

private:
	static const size_t logVertexCount = ColumnTraits::logVertexCount;
	static const size_t maxColumnCount = ColumnTraits::maxColumnCount;
	static const size_t maxCodeParam = ColumnTraits::maxCodeParam;

	static const size_t vertexCount = 1 << logVertexCount;

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

		~Graph() noexcept
		{
		}

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

	typedef std::function<void(MemManager&, size_t, Raw*)> CreateFunc;
	typedef std::function<void(MemManager*, size_t, Raw*)> DestroyFunc;
	typedef std::function<void(MemManager&, size_t, const Raw*, Raw*)> CopyFunc;

	struct FuncRecord
	{
		size_t offset;
		CreateFunc createFunc;
		DestroyFunc destroyFunc;
		CopyFunc copyFunc;
	};

	typedef internal::NestedArrayIntCap<0, FuncRecord, MemManagerPtr> FuncRecords;

	typedef internal::NestedArrayIntCap<ColumnTraits::mutableOffsetsInternalCapacity,
		uint8_t, MemManagerPtr> MutableOffsets;

	typedef typename ColumnTraits::VisitableItems VisitableItems;

public:
	typedef typename ColumnRecords::ConstIterator ConstIterator;
	typedef ConstIterator Iterator;

public:
	explicit DataColumnList(MemManager&& memManager = MemManager())
		: mCodeParam(0),
		mTotalSize(Settings::keepRowNumber ? sizeof(size_t) : 0),
		mAlignment(Settings::keepRowNumber ? internal::AlignmentOf<size_t>::value : 1),
		mColumnCodeSet(ColumnCodeHashTraits(), std::move(memManager)),
		mColumnRecords(MemManagerPtr(GetMemManager())),
		mFuncRecords(MemManagerPtr(GetMemManager())),
		mMutableOffsets(MemManagerPtr(GetMemManager()))
	{
		std::fill(mAddends.begin(), mAddends.end(), 0);
	}

	template<typename Item, typename... Items>
	explicit DataColumnList(const Column<Item>& column, const Column<Items>&... columns)
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

	~DataColumnList() noexcept
	{
	}

	DataColumnList& operator=(const DataColumnList&) = delete;

	ConstIterator GetBegin() const noexcept
	{
		return mColumnRecords.GetBegin();
	}

	ConstIterator GetEnd() const noexcept
	{
		return mColumnRecords.GetEnd();
	}

	MOMO_FRIENDS_BEGIN_END(const DataColumnList&, ConstIterator)

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
	void Add(const Column<Item>& column, const Column<Items>&... columns)
	{
		static const size_t columnCount = 1 + sizeof...(columns);
		if (columnCount + GetCount() > maxColumnCount)
			throw std::runtime_error("Too many columns");
		std::array<ColumnCode, columnCount> columnCodes = {{ ColumnTraits::GetColumnCode(column),
			ColumnTraits::GetColumnCode(columns)... }};
		Addends addends;
		size_t offset;
		size_t alignment;
		size_t codeParam = mCodeParam;
		while (true)
		{
			std::fill(addends.begin(), addends.end(), 0);
			offset = mTotalSize;
			alignment = mAlignment;
			if (pvFillAddends<Item, Items...>(addends, offset, alignment, codeParam,
				columnCodes.data()))
			{
				break;
			}
			++codeParam;
			if (codeParam > maxCodeParam)
				throw std::runtime_error("Cannot add columns");
		}
		FuncRecord funcRec;
		funcRec.offset = mTotalSize;
		funcRec.createFunc = [] (MemManager& memManager, size_t offset, Raw* raw)
			{ pvCreate<void, Item, Items...>(memManager, offset, raw); };
		funcRec.destroyFunc = [] (MemManager* memManager, size_t offset, Raw* raw)
			{ pvDestroy<void, Item, Items...>(memManager, offset, raw); };
		funcRec.copyFunc = [] (MemManager& memManager, size_t offset, const Raw* srcRaw, Raw* dstRaw)
			{ pvCopy<void, Item, Items...>(memManager, offset, srcRaw, dstRaw); };
		mMutableOffsets.SetCount((offset + 7) / 8, uint8_t{0});
		mColumnRecords.Reserve(mColumnRecords.GetCount() + columnCount);
		mFuncRecords.Reserve(mFuncRecords.GetCount() + 1);
		try
		{
			mColumnCodeSet.Insert(columnCodes.begin(), columnCodes.end());
		}
		catch (...)
		{
			for (ColumnCode code : columnCodes)
				mColumnCodeSet.Remove(code);	// no throw
			//mMutableOffsets.SetCount((mTotalSize + 7) / 8);
			throw;
		}
		mCodeParam = codeParam;
		mAddends = addends;
		mTotalSize = offset;
		mAlignment = alignment;
		pvAddColumnRecords(column, columns...);
		mFuncRecords.AddBackNogrow(std::move(funcRec));
	}

	template<typename... Items>
	void SetMutable(const Column<Items>&... columns)
	{
		pvSetMutable(columns...);
	}

	void ResetMutable() noexcept
	{
		std::fill(mMutableOffsets.GetBegin(), mMutableOffsets.GetEnd(), uint8_t{0});
	}

	bool IsMutable(size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < mTotalSize);
		return (mMutableOffsets[offset / 8] & static_cast<uint8_t>(1 << (offset % 8))) != 0;
	}

	size_t GetTotalSize() const noexcept
	{
		return mTotalSize;
	}

	size_t GetAlignment() const noexcept
	{
		return mAlignment;
	}

	void CreateRaw(Raw* raw)
	{
		MemManager& memManager = GetMemManager();
		size_t funcIndex = 0;
		try
		{
			size_t funcCount = mFuncRecords.GetCount();
			for (; funcIndex < funcCount; ++funcIndex)
			{
				const FuncRecord& funcRec = mFuncRecords[funcIndex];
				funcRec.createFunc(memManager, funcRec.offset, raw);
			}
		}
		catch (...)
		{
			for (size_t i = 0; i < funcIndex; ++i)
			{
				const FuncRecord& funcRec = mFuncRecords[i];
				funcRec.destroyFunc(&memManager, funcRec.offset, raw);
			}
			throw;
		}
	}

	void DestroyRaw(Raw* raw) const noexcept
	{
		for (const auto& funcRec : mFuncRecords)
			funcRec.destroyFunc(nullptr, funcRec.offset, raw);
	}

	void DestroyRaw(Raw* raw) noexcept
	{
		MemManager& memManager = GetMemManager();
		for (const auto& funcRec : mFuncRecords)
			funcRec.destroyFunc(&memManager, funcRec.offset, raw);
	}

	void CopyRaw(const Raw* srcRaw, Raw* dstRaw)
	{
		MemManager& memManager = GetMemManager();
		size_t funcIndex = 0;
		try
		{
			size_t funcCount = mFuncRecords.GetCount();
			for (; funcIndex < funcCount; ++funcIndex)
			{
				const FuncRecord& funcRec = mFuncRecords[funcIndex];
				funcRec.copyFunc(memManager, funcRec.offset, srcRaw, dstRaw);
			}
		}
		catch (...)
		{
			for (size_t i = 0; i < funcIndex; ++i)
			{
				const FuncRecord& funcRec = mFuncRecords[i];
				funcRec.destroyFunc(&memManager, funcRec.offset, dstRaw);
			}
			throw;
		}
	}

	template<typename Item, bool extraCheck = true>
	size_t GetOffset(const Column<Item>& column) const
	{
		ColumnCode code = ColumnTraits::GetColumnCode(column);
		MOMO_EXTRA_CHECK(!extraCheck || mColumnCodeSet.ContainsKey(code));
		size_t offset = pvGetOffset(code);
		MOMO_ASSERT(offset < mTotalSize);
		MOMO_ASSERT(offset % ItemTraits::template GetAlignment<Item>() == 0);
		return offset;
	}

	template<typename Item>
	static Item& GetByOffset(Raw* raw, size_t offset) noexcept
	{
		//MOMO_ASSERT(offset < mTotalSize);
		//MOMO_ASSERT(offset % ItemTraits::template GetAlignment<Item>() == 0);
		return *internal::BitCaster::PtrToPtr<Item>(raw, offset);
	}

	template<typename Item, typename ItemArg>
	void Assign(Raw* raw, size_t offset, ItemArg&& itemArg) const
	{
		ItemTraits::Assign(std::forward<ItemArg>(itemArg), GetByOffset<Item>(raw, offset));
	}

	size_t GetNumber(const Raw* raw) const noexcept
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		return *internal::BitCaster::PtrToPtr<const size_t>(raw, 0);
	}

	void SetNumber(Raw* raw, size_t number) const noexcept
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		*internal::BitCaster::PtrToPtr<size_t>(raw, 0) = number;
	}

	template<typename Item>
	bool Contains(const Column<Item>& column) const noexcept
	{
		ColumnCode code = ColumnTraits::GetColumnCode(column);
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(code, mCodeParam);
		if (mAddends[vertices.first] == 0 || mAddends[vertices.second] == 0)
			return false;
		return mColumnCodeSet.ContainsKey(code);
	}

	template<typename PtrVisitor>
	void VisitPointers(const Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers<const void>(raw, ptrVisitor);
	}

	template<typename PtrVisitor>
	void VisitPointers(Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers<void>(raw, ptrVisitor);
	}

private:
	template<typename... Items>
	bool pvFillAddends(Addends& addends, size_t& offset, size_t& alignment, size_t codeParam,
		const ColumnCode* columnCodes) const
	{
		Graph graph;
		for (ColumnCode code : mColumnCodeSet)
		{
			std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(code, codeParam);
			graph.AddEdges(vertices.first, vertices.second, pvGetOffset(code));
		}
		return pvFillAddends<Items...>(addends, graph, offset, alignment, codeParam, columnCodes);
	}

	template<typename... Items>
	static bool pvFillAddends(Addends& addends, Graph& graph, size_t& offset, size_t& alignment,
		size_t codeParam, const ColumnCode* columnCodes)
	{
		pvAddEdges<void, Items...>(graph, offset, alignment, codeParam, columnCodes);
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

	template<typename Void, typename Item, typename... Items>
	static void pvAddEdges(Graph& graph, size_t& offset, size_t& alignment, size_t codeParam,
		const ColumnCode* columnCodes)
	{
		pvCorrectOffset<Item>(offset);
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(*columnCodes, codeParam);
		graph.AddEdges(vertices.first, vertices.second, offset);
		offset += ItemTraits::template GetSize<Item>();
		alignment = std::minmax(alignment,
			ItemTraits::template GetAlignment<Item>()).second;
		pvAddEdges<Void, Items...>(graph, offset, alignment, codeParam, columnCodes + 1);
	}

	template<typename Void>
	static void pvAddEdges(Graph& /*graph*/, size_t& /*offset*/, size_t& /*alignment*/,
		size_t /*codeParam*/, const ColumnCode* /*columnCodes*/)
	{
	}

	template<typename Item, typename... Items>
	void pvAddColumnRecords(const Column<Item>& column, const Column<Items>&... columns) noexcept
	{
		size_t offset = pvGetOffset(ColumnTraits::GetColumnCode(column));
		const char* columnName = ColumnTraits::GetColumnName(column);
		mColumnRecords.AddBackNogrow(ColumnRecord{ typeid(Item), columnName, offset });
		pvAddColumnRecords(columns...);
	}

	void pvAddColumnRecords() noexcept
	{
	}

	template<typename Item, typename... Items>
	void pvSetMutable(const Column<Item>& column, const Column<Items>&... columns)
	{
		size_t offset = GetOffset(column);
		mMutableOffsets[offset / 8] |= static_cast<uint8_t>(1 << (offset % 8));
		pvSetMutable(columns...);
	}

	void pvSetMutable() noexcept
	{
	}

	size_t pvGetOffset(ColumnCode code) const noexcept
	{
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(code, mCodeParam);
		size_t addend1 = mAddends[vertices.first];
		size_t addend2 = mAddends[vertices.second];
		MOMO_ASSERT(addend1 != 0 && addend2 != 0);
		return addend1 + addend2;
	}

	template<typename Void, typename Item, typename... Items>
	static void pvCreate(MemManager& memManager, size_t offset, Raw* raw)
	{
		pvCorrectOffset<Item>(offset);
		ItemTraits::Create(memManager, internal::BitCaster::PtrToPtr<Item>(raw, offset));
		try
		{
			pvCreate<void, Items...>(memManager,
				offset + ItemTraits::template GetSize<Item>(), raw);
		}
		catch (...)
		{
			ItemTraits::Destroy(&memManager, internal::BitCaster::PtrToPtr<Item>(raw, offset));
			throw;
		}
	}

	template<typename Void>
	static void pvCreate(MemManager& /*memManager*/, size_t /*offset*/, Raw* /*raw*/) noexcept
	{
	}

	template<typename Void, typename Item, typename... Items>
	static void pvDestroy(MemManager* memManager, size_t offset, Raw* raw) noexcept
	{
		pvCorrectOffset<Item>(offset);
		ItemTraits::Destroy(memManager, internal::BitCaster::PtrToPtr<Item>(raw, offset));
		pvDestroy<void, Items...>(memManager, offset + ItemTraits::template GetSize<Item>(), raw);
	}

	template<typename Void>
	static void pvDestroy(MemManager* /*memManager*/, size_t /*offset*/, Raw* /*raw*/) noexcept
	{
	}

	template<typename Void, typename Item, typename... Items>
	static void pvCopy(MemManager& memManager, size_t offset, const Raw* srcRaw, Raw* dstRaw)
	{
		pvCorrectOffset<Item>(offset);
		ItemTraits::Copy(memManager, internal::BitCaster::PtrToPtr<const Item>(srcRaw, offset),
			internal::BitCaster::PtrToPtr<Item>(dstRaw, offset));
		try
		{
			pvCopy<void, Items...>(memManager, offset + ItemTraits::template GetSize<Item>(),
				srcRaw, dstRaw);
		}
		catch (...)
		{
			ItemTraits::Destroy(&memManager, internal::BitCaster::PtrToPtr<Item>(dstRaw, offset));
			throw;
		}
	}

	template<typename Void>
	static void pvCopy(MemManager& /*memManager*/, size_t /*offset*/, const Raw* /*srcRaw*/,
		Raw* /*dstRaw*/) noexcept
	{
	}

	template<typename Item>
	static void pvCorrectOffset(size_t& offset) noexcept
	{
		size_t alignment = ItemTraits::template GetAlignment<Item>();
		offset = internal::UIntMath<>::Ceil(offset, alignment);
	}

	template<typename Void, typename PtrVisitor>
	void pvVisitPointers(Void* raw, const PtrVisitor& ptrVisitor) const
	{
		for (const auto& columnRec : mColumnRecords)
		{
			Void* item = internal::BitCaster::PtrToPtr<Void>(raw, columnRec.offset);
			columnRec.Visit<VisitableItems>(item, ptrVisitor);
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
	typename TMemManager = MemManagerDefault,
	typename TSettings = DataSettings<>>
class DataColumnListStatic
{
public:
	typedef TStruct Struct;
	typedef TMemManager MemManager;
	typedef TSettings Settings;

	template<typename Item>
	using Column = DataColumn<Item, Struct>;

	typedef Struct Raw;

	MOMO_STATIC_ASSERT(std::is_class<Struct>::value);

private:
	typedef internal::ObjectManager<Raw, MemManager> RawManager;

	typedef internal::DataColumnRecord ColumnRecord;
	typedef internal::NestedArrayIntCap<0, ColumnRecord, MemManager> ColumnRecords;

	typedef std::bitset<sizeof(Struct)> MutableOffsets;

	typedef typename internal::DataVisitableItemsGetter<Struct>::VisitableItems VisitableItems;

public:
	explicit DataColumnListStatic(MemManager&& memManager = MemManager())
		: mColumnRecords(std::move(memManager))
	{
	}

	DataColumnListStatic(DataColumnListStatic&& columnList) noexcept
		: mColumnRecords(std::move(columnList.mColumnRecords)),
		mMutableOffsets(columnList.mMutableOffsets)
	{
	}

	DataColumnListStatic(const DataColumnListStatic& columnList)
		: mColumnRecords(columnList.mColumnRecords),
		mMutableOffsets(columnList.mMutableOffsets)
	{
	}

	~DataColumnListStatic() noexcept
	{
	}

	DataColumnListStatic& operator=(const DataColumnListStatic&) = delete;

	const MemManager& GetMemManager() const noexcept
	{
		return mColumnRecords.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mColumnRecords.GetMemManager();
	}

	template<typename... Items>
	void SetMutable(const Column<Items>&... columns)
	{
		pvSetMutable(columns...);
	}

	void ResetMutable() noexcept
	{
		mMutableOffsets.reset();
	}

	bool IsMutable(size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < sizeof(Struct));
		return mMutableOffsets.test(offset);
	}

	size_t GetTotalSize() const noexcept
	{
		size_t totalSize = sizeof(Struct);
		if (Settings::keepRowNumber)
		{
			totalSize = internal::UIntMath<>::Ceil(totalSize,
				internal::AlignmentOf<size_t>::value);
			totalSize += sizeof(size_t);
		}
		return totalSize;
	}

	size_t GetAlignment() const noexcept
	{
		size_t alignment = RawManager::alignment;
		if (Settings::keepRowNumber)
			alignment = std::minmax(alignment, size_t{internal::AlignmentOf<size_t>::value}).second;
		return alignment;
	}

	void CreateRaw(Raw* raw)
	{
		(typename RawManager::template Creator<>(GetMemManager()))(raw);
	}

	void DestroyRaw(Raw* raw) const noexcept
	{
		RawManager::Destroyer::Destroy(nullptr, *raw);
	}

	void DestroyRaw(Raw* raw) noexcept
	{
		RawManager::Destroy(GetMemManager(), *raw);
	}

	void CopyRaw(const Raw* srcRaw, Raw* dstRaw)
	{
		RawManager::Copy(GetMemManager(), *srcRaw, dstRaw);
	}

	template<typename Item, bool extraCheck = true>
	size_t GetOffset(const Column<Item>& column) const noexcept
	{
		size_t offset = static_cast<size_t>(column.GetCode());	//?
		MOMO_ASSERT(offset < sizeof(Struct));
		MOMO_ASSERT(offset % internal::AlignmentOf<Item>::value == 0);
		return offset;
	}

	template<typename Item>
	static Item& GetByOffset(Raw* raw, size_t offset) noexcept
	{
		//MOMO_ASSERT(offset < sizeof(Struct));
		//MOMO_ASSERT(offset % internal::AlignmentOf<Item>::value == 0);
		return *internal::BitCaster::PtrToPtr<Item>(raw, offset);
	}

	template<typename Item, typename ItemArg>
	void Assign(Raw* raw, size_t offset, ItemArg&& itemArg) const
	{
		GetByOffset<Item>(raw, offset) = std::forward<ItemArg>(itemArg);
	}

	size_t GetNumber(const Raw* raw) const noexcept
	{
		return *internal::BitCaster::PtrToPtr<const size_t>(raw, pvGetNumberOffset());
	}

	void SetNumber(Raw* raw, size_t number) const noexcept
	{
		*internal::BitCaster::PtrToPtr<size_t>(raw, pvGetNumberOffset()) = number;
	}

	template<typename Item>
	bool Contains(const Column<Item>& /*column*/) const noexcept
	{
		return true;
	}

	template<typename Item, typename... Items>
	void PrepareForVisitors(const Column<Item>& column, const Column<Items>&... columns)
	{
		static const size_t columnCount = 1 + sizeof...(columns);
		mColumnRecords.Reserve(columnCount);
		mColumnRecords.Clear(false);
		pvAddColumnRecords(column, columns...);
	}

	template<typename PtrVisitor>
	void VisitPointers(const Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers<const void>(raw, ptrVisitor);
	}

	template<typename PtrVisitor>
	void VisitPointers(Raw* raw, const PtrVisitor& ptrVisitor) const
	{
		pvVisitPointers<void>(raw, ptrVisitor);
	}

private:
	template<typename Item, typename... Items>
	void pvSetMutable(const Column<Item>& column, const Column<Items>&... columns)
	{
		mMutableOffsets.set(GetOffset(column));
		pvSetMutable(columns...);
	}

	void pvSetMutable() noexcept
	{
	}

	static size_t pvGetNumberOffset() noexcept
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		return internal::UIntMath<>::Ceil(sizeof(Struct), internal::AlignmentOf<size_t>::value);
	}

	template<typename Item, typename... Items>
	void pvAddColumnRecords(const Column<Item>& column, const Column<Items>&... columns) noexcept
	{
		mColumnRecords.AddBackNogrow(ColumnRecord{ typeid(Item), column.GetName(), GetOffset(column) });
		pvAddColumnRecords(columns...);
	}

	void pvAddColumnRecords() noexcept
	{
	}

	template<typename Void, typename PtrVisitor>
	void pvVisitPointers(Void* raw, const PtrVisitor& ptrVisitor) const
	{
		if (mColumnRecords.IsEmpty())
			throw std::runtime_error("Not prepared for visitors");
		for (const auto& columnRec : mColumnRecords)
		{
			Void* item = internal::BitCaster::PtrToPtr<Void>(raw, columnRec.offset);
			columnRec.Visit<VisitableItems>(item, ptrVisitor);
		}
	}

private:
	ColumnRecords mColumnRecords;
	MutableOffsets mMutableOffsets;
};

} // namespace momo
