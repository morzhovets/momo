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
    class DataColumnList
    class DataColumnListStatic

\**********************************************************/

#pragma once

#include "Array.h"

#include <bitset>

#define MOMO_DATA_COLUMN_STRUCT(Struct, name) \
	constexpr momo::DataColumn<decltype(std::declval<Struct&>().name), Struct> \
	name((uint64_t)offsetof(Struct, name))

#define MOMO_DATA_COLUMN_STRING_TAG(Tag, Type, name) \
	MOMO_STATIC_ASSERT(!std::is_class<Tag>::value || std::is_empty<Tag>::value); \
	constexpr momo::DataColumn<Type, Tag> name(momo::internal::StrHasher::GetHashCode64(#name))

#define MOMO_DATA_COLUMN_STRING(Type, name) \
	MOMO_DATA_COLUMN_STRING_TAG(momo::DataStructDefault, Type, name)

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
				: (GetHashCode64(str + 1) ^ (uint64_t)(unsigned char)*str) * fnvPrime64;
		}
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
	constexpr explicit DataColumn(uint64_t code) noexcept
		: mCode(code)
	{
	}

	constexpr uint64_t GetCode() const noexcept
	{
		return mCode;
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

struct DataStructDefault
{
};

template<typename TStruct = DataStructDefault>
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

public:
	template<typename Item>
	static ColumnCode GetColumnCode(const Column<Item>& column) noexcept
	{
		return column.GetCode();
	}

	static std::pair<size_t, size_t> GetVertices(ColumnCode code, size_t codeParam) noexcept
	{
		static const size_t vertexCount1 = ((size_t)1 << logVertexCount) - 1;
		size_t shortCode = (size_t)(code + (code >> 32));
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
		return MOMO_ALIGNMENT_OF(Item);
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

	typedef void Raw;

private:
	typedef typename ColumnTraits::ColumnCode ColumnCode;

	static const size_t logVertexCount = ColumnTraits::logVertexCount;
	static const size_t maxColumnCount = ColumnTraits::maxColumnCount;
	static const size_t maxCodeParam = ColumnTraits::maxCodeParam;

	static const size_t vertexCount = 1 << logVertexCount;

	template<size_t edgeCount>
	class Graph
	{
	private:
		struct Edge
		{
			size_t vertex;
			size_t value;
			Edge* nextEdge;
		};

	public:
		explicit Graph() noexcept
		{
			mEdgeNumber = 0;
			std::fill(mEdges.begin(), mEdges.end(), nullptr);
		}

		Graph(const Graph&) = delete;

		~Graph() noexcept
		{
		}

		Graph& operator=(const Graph&) = delete;

		void AddEdge(size_t vertex1, size_t vertex2, size_t value) noexcept
		{
			MOMO_ASSERT(mEdgeNumber < vertexCount * 2);
			Edge* edge = &mEdgeStorage[mEdgeNumber];
			++mEdgeNumber;
			edge->vertex = vertex2;
			edge->value = value;
			edge->nextEdge = mEdges[vertex1];
			mEdges[vertex1] = edge;
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
		size_t mEdgeNumber;
		std::array<Edge, edgeCount> mEdgeStorage;
		std::array<Edge*, vertexCount> mEdges;
	};

	typedef std::array<size_t, vertexCount> Addends;

	typedef internal::NestedArrayIntCap<ColumnTraits::mutableOffsetsInternalCapacity,
		uint8_t, MemManager> MutableOffsets;

	typedef std::function<void(MemManager&, Raw*)> CreateFunc;
	typedef std::function<void(MemManager*, Raw*)> DestroyFunc;
	typedef std::function<void(MemManager&, const Raw*, Raw*)> CopyFunc;

	typedef internal::NestedArrayIntCap<maxColumnCount, ColumnCode,
		internal::MemManagerDummy> ColumnCodes;

public:
	template<typename Item, typename... Items>
	explicit DataColumnList(const Column<Item>& column, const Column<Items>&... columns)
		: DataColumnList(MemManager(), column, columns...)
	{
	}

	template<typename Item, typename... Items>
	explicit DataColumnList(MemManager&& memManager, const Column<Item>& column,
		const Column<Items>&... columns)
		: mCodeParam(0),
		mMutableOffsets(std::move(memManager)),
		mColumnCodes({ ColumnTraits::GetColumnCode(column), ColumnTraits::GetColumnCode(columns)... })
	{
		while (mCodeParam <= maxCodeParam && !pvFillAddends<Item, Items...>())
			++mCodeParam;
		if (mCodeParam > maxCodeParam)
			throw std::runtime_error("Cannot create momo::DataColumnList");
		mMutableOffsets.SetCount((mTotalSize + 7) / 8, (uint8_t)0);
		mCreateFunc = [] (MemManager& memManager, Raw* raw)
			{ pvCreate<void, Item, Items...>(memManager, raw, 0); };
		mDestroyFunc = [] (MemManager* memManager, Raw* raw)
			{ pvDestroy<void, Item, Items...>(memManager, raw, 0); };
		mCopyFunc = [] (MemManager& memManager, const Raw* srcRaw, Raw* dstRaw)
			{ pvCopy<void, Item, Items...>(memManager, srcRaw, dstRaw, 0); };
	}

	DataColumnList(DataColumnList&& columnList) noexcept
		: mCodeParam(columnList.mCodeParam),
		mAddends(columnList.mAddends),
		mTotalSize(columnList.mTotalSize),
		mAlignment(columnList.mAlignment),
		mMutableOffsets(std::move(columnList.mMutableOffsets)),
		mCreateFunc(std::move(columnList.mCreateFunc)),	//?
		mDestroyFunc(std::move(columnList.mDestroyFunc)),
		mCopyFunc(std::move(columnList.mCopyFunc)),
		mColumnCodes(std::move(columnList.mColumnCodes))
	{
	}

	DataColumnList(const DataColumnList& columnList)
		: mCodeParam(columnList.mCodeParam),
		mAddends(columnList.mAddends),
		mTotalSize(columnList.mTotalSize),
		mAlignment(columnList.mAlignment),
		mMutableOffsets(columnList.mMutableOffsets),
		mCreateFunc(columnList.mCreateFunc),
		mDestroyFunc(columnList.mDestroyFunc),
		mCopyFunc(columnList.mCopyFunc),
		mColumnCodes(columnList.mColumnCodes)
	{
	}

	~DataColumnList() noexcept
	{
	}

	DataColumnList& operator=(const DataColumnList&) = delete;

	const MemManager& GetMemManager() const noexcept
	{
		return mMutableOffsets.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mMutableOffsets.GetMemManager();
	}

	template<typename... Items>
	void SetMutable(const Column<Items>&... columns)
	{
		pvSetMutable(columns...);
	}

	bool IsMutable(size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < mTotalSize);
		return (mMutableOffsets[offset / 8] & (uint8_t)(1 << (offset % 8))) != 0;
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
		mCreateFunc(GetMemManager(), raw);
	}

	void DestroyRaw(Raw* raw) const noexcept
	{
		mDestroyFunc(nullptr, raw);
	}

	void DestroyRaw(Raw* raw) noexcept
	{
		mDestroyFunc(&GetMemManager(), raw);
	}

	void CopyRaw(const Raw* srcRaw, Raw* dstRaw)
	{
		mCopyFunc(GetMemManager(), srcRaw, dstRaw);
	}

	template<typename Item>
	size_t GetOffset(const Column<Item>& column) const
	{
		ColumnCode code = ColumnTraits::GetColumnCode(column);
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(code, mCodeParam);
		size_t addend1 = mAddends[vertices.first];
		size_t addend2 = mAddends[vertices.second];
		MOMO_ASSERT(addend1 != 0 && addend2 != 0);
		size_t offset = addend1 + addend2;
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
		return *internal::BitCaster::PtrToPtr<const size_t>(raw, mTotalSize - sizeof(size_t));
	}

	void SetNumber(Raw* raw, size_t number) const noexcept
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		*internal::BitCaster::PtrToPtr<size_t>(raw, mTotalSize - sizeof(size_t)) = number;
	}

	template<typename Item>
	bool Contains(const Column<Item>& column) const noexcept
	{
		ColumnCode code = ColumnTraits::GetColumnCode(column);
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(code, mCodeParam);
		if (mAddends[vertices.first] == 0 || mAddends[vertices.second] == 0)
			return false;
		return std::find(mColumnCodes.GetBegin(), mColumnCodes.GetEnd(), code)
			!= mColumnCodes.GetEnd();
	}

private:
	template<typename... Items>
	bool pvFillAddends()
	{
		static const size_t columnCount = sizeof...(Items);
		MOMO_STATIC_ASSERT(columnCount <= maxColumnCount);
		static const size_t edgeCount = 2 * columnCount;
		Graph<edgeCount> graph;
		pvMakeGraph<edgeCount, Items...>(graph, 0, 1, mColumnCodes.GetItems());
		std::fill(mAddends.begin(), mAddends.end(), 0);
		for (size_t v = 0; v < vertexCount; ++v)
		{
			if (!graph.HasEdge(v) || mAddends[v] != 0)
				continue;
			mAddends[v] = (size_t)1 << (8 * sizeof(size_t) - 1);
			if (!graph.FillAddends(mAddends.data(), v))
				return false;
		}
		return true;
	}

	template<size_t edgeCount, typename Item, typename... Items>
	void pvMakeGraph(Graph<edgeCount>& graph, size_t offset, size_t maxAlignment,
		const ColumnCode* codes)
	{
		pvCorrectOffset<Item>(offset);
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(*codes, mCodeParam);
		MOMO_EXTRA_CHECK(vertices.first != vertices.second);
		graph.AddEdge(vertices.first, vertices.second, offset);
		graph.AddEdge(vertices.second, vertices.first, offset);
		offset += ItemTraits::template GetSize<Item>();
		maxAlignment = std::minmax(maxAlignment,
			ItemTraits::template GetAlignment<Item>()).second;
		pvMakeGraph<edgeCount, Items...>(graph, offset, maxAlignment, codes + 1);
	}

	template<size_t edgeCount>
	void pvMakeGraph(Graph<edgeCount>& /*graph*/, size_t offset, size_t maxAlignment,
		const ColumnCode* /*codes*/) noexcept
	{
		if (Settings::keepRowNumber)
		{
			pvCorrectOffset<size_t>(offset);
			offset += sizeof(size_t);
			maxAlignment = std::minmax(maxAlignment, MOMO_ALIGNMENT_OF(size_t)).second;
		}
		mTotalSize = internal::UIntMath<size_t>::Ceil(offset, maxAlignment);
		mAlignment = maxAlignment;
	}

	template<typename Item, typename... Items>
	void pvSetMutable(const Column<Item>& column, const Column<Items>&... columns)
	{
		size_t offset = GetOffset(column);
		mMutableOffsets[offset / 8] |= (uint8_t)(1 << (offset % 8));
		pvSetMutable(columns...);
	}

	void pvSetMutable() noexcept
	{
	}

	template<typename Void, typename Item, typename... Items>
	static void pvCreate(MemManager& memManager, Raw* raw, size_t offset)
	{
		pvCorrectOffset<Item>(offset);
		ItemTraits::Create(memManager, internal::BitCaster::PtrToPtr<Item>(raw, offset));
		try
		{
			pvCreate<void, Items...>(memManager, raw,
				offset + ItemTraits::template GetSize<Item>());
		}
		catch (...)
		{
			ItemTraits::Destroy(&memManager, internal::BitCaster::PtrToPtr<Item>(raw, offset));
			throw;
		}
	}

	template<typename Void>
	static void pvCreate(MemManager& /*memManager*/, Raw* /*raw*/, size_t /*offset*/) noexcept
	{
	}

	template<typename Void, typename Item, typename... Items>
	static void pvDestroy(MemManager* memManager, Raw* raw, size_t offset) noexcept
	{
		pvCorrectOffset<Item>(offset);
		ItemTraits::Destroy(memManager, internal::BitCaster::PtrToPtr<Item>(raw, offset));
		pvDestroy<void, Items...>(memManager, raw, offset + ItemTraits::template GetSize<Item>());
	}

	template<typename Void>
	static void pvDestroy(MemManager* /*memManager*/, Raw* /*raw*/, size_t /*offset*/) noexcept
	{
	}

	template<typename Void, typename Item, typename... Items>
	static void pvCopy(MemManager& memManager, const Raw* srcRaw, Raw* dstRaw, size_t offset)
	{
		pvCorrectOffset<Item>(offset);
		ItemTraits::Copy(memManager, internal::BitCaster::PtrToPtr<const Item>(srcRaw, offset),
			internal::BitCaster::PtrToPtr<Item>(dstRaw, offset));
		try
		{
			pvCopy<void, Items...>(memManager, srcRaw, dstRaw,
				offset + ItemTraits::template GetSize<Item>());
		}
		catch (...)
		{
			ItemTraits::Destroy(&memManager, internal::BitCaster::PtrToPtr<Item>(dstRaw, offset));
			throw;
		}
	}

	template<typename Void>
	static void pvCopy(MemManager& /*memManager*/, const Raw* /*srcRaw*/, Raw* /*dstRaw*/,
		size_t /*offset*/) noexcept
	{
	}

	template<typename Item>
	static void pvCorrectOffset(size_t& offset) noexcept
	{
		static const size_t alignment = ItemTraits::template GetAlignment<Item>();
		offset = ((offset + alignment - 1) / alignment) * alignment;
	}

private:
	size_t mCodeParam;
	Addends mAddends;
	size_t mTotalSize;
	size_t mAlignment;
	MutableOffsets mMutableOffsets;
	CreateFunc mCreateFunc;
	DestroyFunc mDestroyFunc;
	CopyFunc mCopyFunc;
	ColumnCodes mColumnCodes;
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

	typedef std::bitset<sizeof(Struct)> MutableOffsets;

	template<typename Void, bool keepRowNumber>
	struct Number;

	template<typename Void>
	struct Number<Void, true>
	{
		size_t rowNumber;
	};

	template<typename Void>
	struct Number<Void, false>
	{
	};

	struct StructNumber : public Struct, public Number<void, Settings::keepRowNumber>
	{
	};

public:
	explicit DataColumnListStatic(MemManager&& memManager = MemManager())
		: mMemManager(std::move(memManager))
	{
	}

	DataColumnListStatic(DataColumnListStatic&& columnList) noexcept
		: mMemManager(std::move(columnList.mMemManager)),
		mMutableOffsets(columnList.mMutableOffsets)
	{
	}

	DataColumnListStatic(const DataColumnListStatic& columnList)
		: mMemManager(columnList.mMemManager),
		mMutableOffsets(columnList.mMutableOffsets)
	{
	}

	~DataColumnListStatic() noexcept
	{
	}

	DataColumnListStatic& operator=(const DataColumnListStatic&) = delete;

	const MemManager& GetMemManager() const noexcept
	{
		return mMemManager;
	}

	MemManager& GetMemManager() noexcept
	{
		return mMemManager;
	}

	template<typename... Items>
	void SetMutable(const Column<Items>&... columns)
	{
		pvSetMutable(columns...);
	}

	bool IsMutable(size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < sizeof(Struct));
		return mMutableOffsets.test(offset);
	}

	size_t GetTotalSize() const noexcept
	{
		return sizeof(StructNumber);
	}

	size_t GetAlignment() const noexcept
	{
		return MOMO_ALIGNMENT_OF(StructNumber);
	}

	void CreateRaw(Raw* raw)
	{
		(typename RawManager::template Creator<>(mMemManager))(raw);
	}

	void DestroyRaw(Raw* raw) const noexcept
	{
		RawManager::Destroyer::Destroy(nullptr, *raw);
	}

	void DestroyRaw(Raw* raw) noexcept
	{
		RawManager::Destroy(mMemManager, *raw);
	}

	void CopyRaw(const Raw* srcRaw, Raw* dstRaw)
	{
		RawManager::Copy(mMemManager, *srcRaw, dstRaw);
	}

	template<typename Item>
	size_t GetOffset(const Column<Item>& column) const noexcept
	{
		size_t offset = (size_t)column.GetCode();	//?
		MOMO_ASSERT(offset < sizeof(Struct));
		MOMO_ASSERT(offset % MOMO_ALIGNMENT_OF(Item) == 0);
		return offset;
	}

	template<typename Item>
	static Item& GetByOffset(Raw* raw, size_t offset) noexcept
	{
		//MOMO_ASSERT(offset < sizeof(Struct));
		//MOMO_ASSERT(offset % MOMO_ALIGNMENT_OF(Item) == 0);
		return *internal::BitCaster::PtrToPtr<Item>(raw, offset);
	}

	template<typename Item, typename ItemArg>
	void Assign(Raw* raw, size_t offset, ItemArg&& itemArg) const
	{
		GetByOffset<Item>(raw, offset) = std::forward<ItemArg>(itemArg);
	}

	size_t GetNumber(const Raw* raw) const noexcept
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		return static_cast<const StructNumber*>(raw)->rowNumber;
	}

	void SetNumber(Raw* raw, size_t number) const noexcept
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		static_cast<StructNumber*>(raw)->rowNumber = number;
	}

	template<typename Item>
	bool Contains(const Column<Item>& /*column*/) const noexcept
	{
		return true;
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

private:
	MemManager mMemManager;
	MutableOffsets mMutableOffsets;
};

} // namespace momo
