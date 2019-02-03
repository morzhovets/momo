/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/DataColumn.h

  macros:
    MOMO_DATA_COLUMN_STRUCT
    MOMO_DATA_COLUMN_STRING_TAG
    MOMO_DATA_COLUMN_STRING

  namespace momo::experimental:
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
	constexpr momo::experimental::DataColumn<decltype(std::declval<Struct&>().name), Struct> \
	name((uint64_t)offsetof(Struct, name))

#define MOMO_DATA_COLUMN_STRING_TAG(Tag, Type, name) \
	MOMO_STATIC_ASSERT(!std::is_class<Tag>::value || std::is_empty<Tag>::value); \
	constexpr momo::experimental::DataColumn<Type, Tag> \
	name(momo::experimental::internal::StrHasher::GetHashCode64(#name))

#define MOMO_DATA_COLUMN_STRING(Type, name) \
	MOMO_DATA_COLUMN_STRING_TAG(momo::experimental::DataStructDefault, Type, name)

namespace momo
{

namespace experimental
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

template<typename TStruct = DataStructDefault,
	typename TMemManager = MemManagerDefault>
class DataColumnTraits
{
public:
	typedef TStruct Struct;
	typedef TMemManager MemManager;

	template<typename Item>
	using Column = DataColumn<Item, Struct>;

	static const size_t logVertexCount = 8;
	static const size_t maxColumnCount = 200;
	static const size_t maxCodeParam = 255;

	static const size_t mutOffsetsInternalCapacity = 0;

private:
	template<typename Item>
	using ItemManager = momo::internal::ObjectManager<Item, MemManager>;

public:
	template<typename Item>
	static std::pair<size_t, size_t> GetVertices(const Column<Item>& column,
		size_t codeParam) noexcept
	{
		static const size_t vertexCount1 = ((size_t)1 << logVertexCount) - 1;
		uint64_t code64 = column.GetCode();
		size_t code = (size_t)(code64 + (code64 >> 32));
		code ^= (codeParam >> 4) ^ ((codeParam & 15) << 28);
		code += code >> 16;
		size_t vertex1 = code & vertexCount1;
		size_t vertex2 = (code >> logVertexCount) & vertexCount1;
		vertex2 ^= (vertex1 == vertex2) ? 1 : 0;
		return std::make_pair(vertex1, vertex2);
	}

	template<typename Item>
	static constexpr size_t GetSize(/*const Column<Item>& column*/) noexcept
	{
		return sizeof(Item);
	}

	template<typename Item>
	static constexpr size_t GetAlignment(/*const Column<Item>& column*/) noexcept
	{
		return MOMO_ALIGNMENT_OF(Item);
	}

	template<typename Item>
	static void Create(MemManager& memManager, Item* item /*, const Column<Item>& column*/)
	{
		(typename ItemManager<Item>::template Creator<>(memManager))(item);
	}

	template<typename Item>
	static void Destroy(MemManager* memManager, Item* item /*, const Column<Item>& column*/) noexcept
	{
		ItemManager<Item>::Destroyer::Destroy(memManager, *item);
	}

	template<typename Item>
	static void Copy(MemManager& memManager, const Item* srcItem, Item* dstItem
		/*, const Column<Item>& column*/)
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
	typename TSettings = DataSettings<>>
class DataColumnList
{
public:
	typedef TColumnTraits ColumnTraits;
	typedef TSettings Settings;
	typedef typename ColumnTraits::MemManager MemManager;

	template<typename Item>
	using Column = typename ColumnTraits::template Column<Item>;

	typedef char Raw;

private:
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
		Graph() noexcept
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

	typedef momo::internal::NestedArrayIntCap<ColumnTraits::mutOffsetsInternalCapacity,
		uint8_t, MemManager> MutOffsets;

	typedef std::function<void(MemManager&, Raw*)> CreateFunc;
	typedef std::function<void(MemManager*, Raw*)> DestroyFunc;
	typedef std::function<void(MemManager&, const Raw*, Raw*)> CopyFunc;

public:
	template<typename Item, typename... Items>
	explicit DataColumnList(const Column<Item>& column, const Column<Items>&... columns)
		: DataColumnList(MemManager(), column, columns...)
	{
	}

	template<typename Item, typename... Items>
	explicit DataColumnList(MemManager&& memManager, const Column<Item>& column,
		const Column<Items>&... columns)
		: mMutOffsets(std::move(memManager))
	{
		mCodeParam = 0;
		while (mCodeParam <= maxCodeParam && !pvFillAddends(column, columns...))
			++mCodeParam;
		if (mCodeParam > maxCodeParam)
			throw std::runtime_error("Cannot create DataColumnList");
		mMutOffsets.SetCount((mTotalSize + 7) / 8, (uint8_t)0);
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
		mMutOffsets(std::move(columnList.mMutOffsets)),
		mCreateFunc(std::move(columnList.mCreateFunc)),	//?
		mDestroyFunc(std::move(columnList.mDestroyFunc)),
		mCopyFunc(std::move(columnList.mCopyFunc))
	{
	}

	DataColumnList(const DataColumnList& columnList)
		: mCodeParam(columnList.mCodeParam),
		mAddends(columnList.mAddends),
		mTotalSize(columnList.mTotalSize),
		mAlignment(columnList.mAlignment),
		mMutOffsets(columnList.mMutOffsets),
		mCreateFunc(columnList.mCreateFunc),
		mDestroyFunc(columnList.mDestroyFunc),
		mCopyFunc(columnList.mCopyFunc)
	{
	}

	~DataColumnList() noexcept
	{
	}

	DataColumnList& operator=(const DataColumnList&) = delete;

	const MemManager& GetMemManager() const noexcept
	{
		return mMutOffsets.GetMemManager();
	}

	MemManager& GetMemManager() noexcept
	{
		return mMutOffsets.GetMemManager();
	}

	template<typename... Items>
	void SetMutable(const Column<Items>&... columns)
	{
		pvSetMutable(columns...);
	}

	bool IsMutable(size_t offset) const noexcept
	{
		return (mMutOffsets[offset / 8] & (uint8_t)(1 << (offset % 8))) != 0;
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
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(column, mCodeParam);
		size_t addend1 = mAddends[vertices.first];
		size_t addend2 = mAddends[vertices.second];
		MOMO_ASSERT(addend1 != 0 && addend2 != 0);
		size_t offset = addend1 + addend2;
		MOMO_ASSERT(offset < mTotalSize);
		MOMO_ASSERT(offset % ColumnTraits::template GetAlignment<Item>() == 0);
		return offset;
	}

	template<typename Item>
	Item& GetByOffset(Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < mTotalSize);
		MOMO_ASSERT(offset % ColumnTraits::template GetAlignment<Item>() == 0);
		return *reinterpret_cast<Item*>(raw + offset);
	}

	template<typename Item, typename ItemArg>
	void Assign(Raw* raw, size_t offset, ItemArg&& itemArg) const
	{
		ColumnTraits::Assign(std::forward<ItemArg>(itemArg), GetByOffset<Item>(raw, offset));
	}

	size_t GetNumber(const Raw* raw) const noexcept
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		return *reinterpret_cast<const size_t*>(raw + mTotalSize - sizeof(size_t));
	}

	void SetNumber(Raw* raw, size_t number) const noexcept
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		*reinterpret_cast<size_t*>(raw + mTotalSize - sizeof(size_t)) = number;
	}

private:
	template<typename... Items>
	bool pvFillAddends(const Column<Items>&... columns)
	{
		static const size_t columnCount = sizeof...(columns);
		MOMO_STATIC_ASSERT(columnCount <= maxColumnCount);
		Graph<2 * columnCount> graph;
		pvMakeGraph(graph, 0, 1, columns...);
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
		const Column<Item>& column, const Column<Items>&... columns)
	{
		pvCorrectOffset<Item>(offset);
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(column, mCodeParam);
		MOMO_EXTRA_CHECK(vertices.first != vertices.second);
		graph.AddEdge(vertices.first, vertices.second, offset);
		graph.AddEdge(vertices.second, vertices.first, offset);
		offset += ColumnTraits::template GetSize<Item>();
		maxAlignment = std::minmax(maxAlignment,
			ColumnTraits::template GetAlignment<Item>()).second;
		pvMakeGraph(graph, offset, maxAlignment, columns...);
	}

	template<size_t edgeCount>
	void pvMakeGraph(Graph<edgeCount>& /*graph*/, size_t offset, size_t maxAlignment) noexcept
	{
		if (Settings::keepRowNumber)
		{
			pvCorrectOffset<size_t>(offset);
			offset += sizeof(size_t);
			maxAlignment = std::minmax(maxAlignment, MOMO_ALIGNMENT_OF(size_t)).second;
		}
		mTotalSize = momo::internal::UIntMath<size_t>::Ceil(offset, maxAlignment);
		mAlignment = maxAlignment;
	}

	template<typename Item, typename... Items>
	void pvSetMutable(const Column<Item>& column, const Column<Items>&... columns)
	{
		size_t offset = GetOffset(column);
		mMutOffsets[offset / 8] |= (uint8_t)(1 << (offset % 8));
		pvSetMutable(columns...);
	}

	void pvSetMutable() noexcept
	{
	}

	template<typename Void, typename Item, typename... Items>
	static void pvCreate(MemManager& memManager, Raw* raw, size_t offset)
	{
		pvCorrectOffset<Item>(offset);
		ColumnTraits::Create(memManager, reinterpret_cast<Item*>(raw + offset));
		try
		{
			pvCreate<void, Items...>(memManager, raw, offset + ColumnTraits::template GetSize<Item>());
		}
		catch (...)
		{
			ColumnTraits::Destroy(&memManager, reinterpret_cast<Item*>(raw + offset));
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
		ColumnTraits::Destroy(memManager, reinterpret_cast<Item*>(raw + offset));
		pvDestroy<void, Items...>(memManager, raw, offset + ColumnTraits::template GetSize<Item>());
	}

	template<typename Void>
	static void pvDestroy(MemManager* /*memManager*/, Raw* /*raw*/, size_t /*offset*/) noexcept
	{
	}

	template<typename Void, typename Item, typename... Items>
	static void pvCopy(MemManager& memManager, const Raw* srcRaw, Raw* dstRaw, size_t offset)
	{
		pvCorrectOffset<Item>(offset);
		ColumnTraits::Copy(memManager, reinterpret_cast<const Item*>(srcRaw + offset),
			reinterpret_cast<Item*>(dstRaw + offset));
		try
		{
			pvCopy<void, Items...>(memManager, srcRaw, dstRaw,
				offset + ColumnTraits::template GetSize<Item>());
		}
		catch (...)
		{
			ColumnTraits::Destroy(&memManager, reinterpret_cast<Item*>(dstRaw + offset));
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
		static const size_t alignment = ColumnTraits::template GetAlignment<Item>();
		offset = ((offset + alignment - 1) / alignment) * alignment;
	}

private:
	size_t mCodeParam;
	Addends mAddends;
	size_t mTotalSize;
	size_t mAlignment;
	MutOffsets mMutOffsets;
	CreateFunc mCreateFunc;
	DestroyFunc mDestroyFunc;
	CopyFunc mCopyFunc;
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
	typedef momo::internal::ObjectManager<Raw, MemManager> RawManager;

	typedef std::bitset<sizeof(Struct)> MutOffsets;

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
		mMutOffsets(columnList.mMutOffsets)
	{
	}

	DataColumnListStatic(const DataColumnListStatic& columnList)
		: mMemManager(columnList.mMemManager),
		mMutOffsets(columnList.mMutOffsets)
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
		return mMutOffsets.test(offset);
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
		return (size_t)column.GetCode();	//?
	}

	template<typename Item>
	Item& GetByOffset(Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < sizeof(Struct));
		MOMO_ASSERT(offset % MOMO_ALIGNMENT_OF(Item) == 0);
		return *reinterpret_cast<Item*>(reinterpret_cast<char*>(raw) + offset);
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

private:
	template<typename Item, typename... Items>
	void pvSetMutable(const Column<Item>& column, const Column<Items>&... columns)
	{
		mMutOffsets.set(GetOffset(column));
		pvSetMutable(columns...);
	}

	void pvSetMutable() noexcept
	{
	}

private:
	MemManager mMemManager;
	MutOffsets mMutOffsets;
};

} // namespace experimental

} // namespace momo
