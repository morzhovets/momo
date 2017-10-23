/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/DataColumn.h

  namespace momo::experimental:
    struct DataSettings
    class DataColumn
    class DataColumnTraits
    class DataColumnList
    class DataColumnListStatic

\**********************************************************/

#pragma once

#include "Array.h"

#include <bitset>

#define MOMO_DECLARE_DATA_COLUMN(Struct, name) \
	constexpr momo::experimental::DataColumn<decltype(std::declval<Struct&>().name), Struct> \
	name(offsetof(Struct, name))

namespace momo
{

namespace experimental
{

template<bool tKeepRowNumber = true>
struct DataSettings
{
	static const CheckMode checkMode = CheckMode::bydefault;
	static const ExtraCheckMode extraCheckMode = ExtraCheckMode::bydefault;

	static const bool keepRowNumber = tKeepRowNumber;

	typedef ArraySettings<> RawsSettings;
};

template<typename TType, typename TStruct>
class DataColumn
{
public:
	typedef TType Type;
	typedef TStruct Struct;

public:
	//constexpr DataColumn(Type Struct::*field) MOMO_NOEXCEPT

	explicit constexpr DataColumn(size_t offset) MOMO_NOEXCEPT
		: mOffset(offset)
	{
	}

	constexpr size_t GetOffset() const MOMO_NOEXCEPT
	{
		return mOffset;
	}

private:
	size_t mOffset;
};

template<typename TStruct,
	typename TMemManager = MemManagerDefault>
class DataColumnTraits
{
public:
	typedef TStruct Struct;
	typedef TMemManager MemManager;

	template<typename Type>
	using Column = DataColumn<Type, Struct>;

	static const size_t logVertexCount = 8;
	static const size_t maxColumnCount = 200;

	static const size_t maxCodeParam = 255;

private:
	template<typename Type>
	using ItemManager = momo::internal::ObjectManager<Type, MemManager>;

public:
	template<typename Type>
	static std::pair<size_t, size_t> GetVertices(const Column<Type>& column,
		size_t codeParam) MOMO_NOEXCEPT
	{
		static const size_t vertexCount1 = ((size_t)1 << logVertexCount) - 1;
		size_t offset = column.GetOffset();
		MOMO_ASSERT(offset < (1 << 24));
		size_t code = (offset << 4) ^ (codeParam >> 4) ^ ((codeParam & 15) << 28);
		code = (code * 5) ^ (code >> 16);
		size_t vertex1 = code & vertexCount1;
		size_t vertex2 = (code >> logVertexCount) & vertexCount1;
		vertex2 += (vertex1 == vertex2) ? 1 : 0;
		return std::make_pair(vertex1, vertex2);
	}

	template<typename Type>
	static constexpr size_t GetSize(/*const Column<Type>& column*/) MOMO_NOEXCEPT
	{
		return sizeof(Type);
	}

	template<typename Type>
	static constexpr size_t GetAlignment(/*const Column<Type>& column*/) MOMO_NOEXCEPT
	{
		return MOMO_ALIGNMENT_OF(Type);
	}

	template<typename Type>
	static void Create(MemManager& memManager, Type* item /*, const Column<Type>& column*/)
	{
		(typename ItemManager<Type>::template Creator<>(memManager))(item);
	}

	template<typename Type>
	static void Destroy(MemManager* memManager, Type* item /*, const Column<Type>& column*/) MOMO_NOEXCEPT
	{
		ItemManager<Type>::Destroyer::Destroy(memManager, *item);
	}

	template<typename Type>
	static void Copy(MemManager& memManager, const Type* srcItem, Type* dstItem
		/*, const Column<Type>& column*/)
	{
		ItemManager<Type>::Copy(memManager, *srcItem, dstItem);
	}

	template<typename TypeArg, typename Type>
	static void Assign(TypeArg&& itemArg, Type& item)
	{
		item = std::forward<TypeArg>(itemArg);
	}
};

template<typename TColumnTraits,
	typename TSettings = DataSettings<>>
class DataColumnList
{
public:
	typedef TColumnTraits ColumnTraits;
	typedef TSettings Settings;
	typedef typename ColumnTraits::MemManager MemManager;

	template<typename Type>
	using Column = typename ColumnTraits::template Column<Type>;

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
		Graph() MOMO_NOEXCEPT
		{
			mEdgeNumber = 0;
			std::fill(mEdges.begin(), mEdges.end(), nullptr);
		}

		Graph(const Graph&) = delete;

		~Graph() MOMO_NOEXCEPT
		{
		}

		Graph& operator=(const Graph&) = delete;

		void AddEdge(size_t vertex1, size_t vertex2, size_t value) MOMO_NOEXCEPT
		{
			MOMO_ASSERT(mEdgeNumber < vertexCount * 2);
			Edge* edge = &mEdgeStorage[mEdgeNumber];
			++mEdgeNumber;
			edge->vertex = vertex2;
			edge->value = value;
			edge->nextEdge = mEdges[vertex1];
			mEdges[vertex1] = edge;
		}

		bool HasEdge(size_t vertex) const MOMO_NOEXCEPT
		{
			return mEdges[vertex] != nullptr;
		}

		bool FillAddends(size_t* addends, size_t vertex) const MOMO_NOEXCEPT
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

	static const size_t mutOffsetsIntCapacity = maxColumnCount * sizeof(void*);
	typedef momo::internal::NestedArrayIntCap<mutOffsetsIntCapacity, uint8_t, MemManager> MutOffsets;

	typedef std::function<void(MemManager&, Raw*)> CreateFunc;
	typedef std::function<void(MemManager*, Raw*)> DestroyFunc;
	typedef std::function<void(MemManager&, const Raw*, Raw*)> CopyFunc;

public:
	template<typename... Types>
	explicit DataColumnList(const Column<Types>&... columns)
		: DataColumnList(MemManager(), columns...)
	{
	}

	template<typename... Types>
	explicit DataColumnList(MemManager&& memManager, const Column<Types>&... columns)
		: mMutOffsets(std::move(memManager))
	{
		mCodeParam = 0;
		while (mCodeParam <= maxCodeParam && !pvFillAddends(columns...))
			++mCodeParam;
		if (mCodeParam > maxCodeParam)
			throw std::runtime_error("Cannot create DataColumnList");
		mMutOffsets.SetCount((mTotalSize + 7) / 8, (uint8_t)0);
		mCreateFunc = [] (MemManager& memManager, Raw* raw)
			{ pvCreate<void, Types...>(memManager, raw, 0); };
		mDestroyFunc = [] (MemManager* memManager, Raw* raw)
			{ pvDestroy<void, Types...>(memManager, raw, 0); };
		mCopyFunc = [] (MemManager& memManager, const Raw* srcRaw, Raw* dstRaw)
			{ pvCopy<void, Types...>(memManager, srcRaw, dstRaw, 0); };
	}

	DataColumnList(DataColumnList&& columnList) MOMO_NOEXCEPT
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

	~DataColumnList() MOMO_NOEXCEPT
	{
	}

	DataColumnList& operator=(const DataColumnList&) = delete;

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return mMutOffsets.GetMemManager();
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return mMutOffsets.GetMemManager();
	}

	template<typename... Types>
	void SetMutable(const Column<Types>&... columns)
	{
		pvSetMutable(columns...);
	}

	bool IsMutable(size_t offset) const MOMO_NOEXCEPT
	{
		return (mMutOffsets[offset / 8] & (uint8_t)(1 << (offset % 8))) != 0;
	}

	size_t GetTotalSize() const MOMO_NOEXCEPT
	{
		return mTotalSize;
	}

	size_t GetAlignment() const MOMO_NOEXCEPT
	{
		return mAlignment;
	}

	void CreateRaw(Raw* raw)
	{
		mCreateFunc(GetMemManager(), raw);
	}

	void DestroyRaw(Raw* raw) const MOMO_NOEXCEPT
	{
		mDestroyFunc(nullptr, raw);
	}

	void DestroyRaw(Raw* raw) MOMO_NOEXCEPT
	{
		mDestroyFunc(&GetMemManager(), raw);
	}

	void CopyRaw(const Raw* srcRaw, Raw* dstRaw)
	{
		mCopyFunc(GetMemManager(), srcRaw, dstRaw);
	}

	template<typename Type>
	size_t GetOffset(const Column<Type>& column) const
	{
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(column, mCodeParam);
		size_t addend1 = mAddends[vertices.first];
		size_t addend2 = mAddends[vertices.second];
		MOMO_ASSERT(addend1 != 0 && addend2 != 0);
		size_t offset = addend1 + addend2;
		MOMO_ASSERT(offset < mTotalSize);
		MOMO_ASSERT(offset % ColumnTraits::template GetAlignment<Type>() == 0);
		return offset;
	}

	template<typename Type>
	Type& GetByOffset(Raw* raw, size_t offset) const MOMO_NOEXCEPT
	{
		MOMO_ASSERT(offset < mTotalSize);
		MOMO_ASSERT(offset % ColumnTraits::template GetAlignment<Type>() == 0);
		return *reinterpret_cast<Type*>(raw + offset);
	}

	template<typename Type, typename TypeArg>
	void Assign(Raw* raw, size_t offset, TypeArg&& itemArg) const
	{
		ColumnTraits::Assign(std::forward<TypeArg>(itemArg), GetByOffset<Type>(raw, offset));
	}

	size_t GetNumber(const Raw* raw) const MOMO_NOEXCEPT
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		return *reinterpret_cast<const size_t*>(raw + mTotalSize - sizeof(size_t));
	}

	void SetNumber(Raw* raw, size_t number) const MOMO_NOEXCEPT
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		*reinterpret_cast<size_t*>(raw + mTotalSize - sizeof(size_t)) = number;
	}

private:
	template<typename... Types>
	bool pvFillAddends(const Column<Types>&... columns)
	{
		static const size_t columnCount = sizeof...(columns);
		MOMO_STATIC_ASSERT(0 < columnCount && columnCount < maxColumnCount);
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

	template<size_t edgeCount, typename Type, typename... Types>
	void pvMakeGraph(Graph<edgeCount>& graph, size_t offset, size_t maxAlignment,
		const Column<Type>& column, const Column<Types>&... columns)
	{
		pvCorrectOffset<Type>(offset);
		std::pair<size_t, size_t> vertices = ColumnTraits::GetVertices(column, mCodeParam);
		MOMO_EXTRA_CHECK(vertices.first != vertices.second);
		graph.AddEdge(vertices.first, vertices.second, offset);
		graph.AddEdge(vertices.second, vertices.first, offset);
		offset += ColumnTraits::template GetSize<Type>();
		maxAlignment = std::minmax(maxAlignment,
			ColumnTraits::template GetAlignment<Type>()).second;
		pvMakeGraph(graph, offset, maxAlignment, columns...);
	}

	template<size_t edgeCount>
	void pvMakeGraph(Graph<edgeCount>& /*graph*/, size_t offset, size_t maxAlignment) MOMO_NOEXCEPT
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

	template<typename Type, typename... Types>
	void pvSetMutable(const Column<Type>& column, const Column<Types>&... columns)
	{
		size_t offset = GetOffset(column);
		mMutOffsets[offset / 8] |= (uint8_t)(1 << (offset % 8));
		pvSetMutable(columns...);
	}

	void pvSetMutable() MOMO_NOEXCEPT
	{
	}

	template<typename Void, typename Type, typename... Types>
	static void pvCreate(MemManager& memManager, Raw* raw, size_t offset)
	{
		pvCorrectOffset<Type>(offset);
		ColumnTraits::Create(memManager, reinterpret_cast<Type*>(raw + offset));
		try
		{
			pvCreate<void, Types...>(memManager, raw, offset + ColumnTraits::template GetSize<Type>());
		}
		catch (...)
		{
			ColumnTraits::Destroy(&memManager, reinterpret_cast<Type*>(raw + offset));
			throw;
		}
	}

	template<typename Void>
	static void pvCreate(MemManager& /*memManager*/, Raw* /*raw*/, size_t /*offset*/) MOMO_NOEXCEPT
	{
	}

	template<typename Void, typename Type, typename... Types>
	static void pvDestroy(MemManager* memManager, Raw* raw, size_t offset) MOMO_NOEXCEPT
	{
		pvCorrectOffset<Type>(offset);
		ColumnTraits::Destroy(memManager, reinterpret_cast<Type*>(raw + offset));
		pvDestroy<void, Types...>(memManager, raw, offset + ColumnTraits::template GetSize<Type>());
	}

	template<typename Void>
	static void pvDestroy(MemManager* /*memManager*/, Raw* /*raw*/, size_t /*offset*/) MOMO_NOEXCEPT
	{
	}

	template<typename Void, typename Type, typename... Types>
	static void pvCopy(MemManager& memManager, const Raw* srcRaw, Raw* dstRaw, size_t offset)
	{
		pvCorrectOffset<Type>(offset);
		ColumnTraits::Copy(memManager, reinterpret_cast<const Type*>(srcRaw + offset),
			reinterpret_cast<Type*>(dstRaw + offset));
		try
		{
			pvCopy<void, Types...>(memManager, srcRaw, dstRaw,
				offset + ColumnTraits::template GetSize<Type>());
		}
		catch (...)
		{
			ColumnTraits::Destroy(&memManager, reinterpret_cast<Type*>(dstRaw + offset));
			throw;
		}
	}

	template<typename Void>
	static void pvCopy(MemManager& /*memManager*/, const Raw* /*srcRaw*/, Raw* /*dstRaw*/,
		size_t /*offset*/) MOMO_NOEXCEPT
	{
	}

	template<typename Type>
	static void pvCorrectOffset(size_t& offset) MOMO_NOEXCEPT
	{
		static const size_t alignment = ColumnTraits::template GetAlignment<Type>();
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

	template<typename Type>
	using Column = DataColumn<Type, Struct>;

	typedef Struct Raw;

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

	DataColumnListStatic(DataColumnListStatic&& columnList) MOMO_NOEXCEPT
		: mMemManager(std::move(columnList.mMemManager)),
		mMutOffsets(columnList.mMutOffsets)
	{
	}

	DataColumnListStatic(const DataColumnListStatic& columnList)
		: mMemManager(columnList.mMemManager),
		mMutOffsets(columnList.mMutOffsets)
	{
	}

	~DataColumnListStatic() MOMO_NOEXCEPT
	{
	}

	DataColumnListStatic& operator=(const DataColumnListStatic&) = delete;

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return mMemManager;
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return mMemManager;
	}

	template<typename... Types>
	void SetMutable(const Column<Types>&... columns)
	{
		pvSetMutable(columns...);
	}

	bool IsMutable(size_t offset) const MOMO_NOEXCEPT
	{
		return mMutOffsets.test(offset);
	}

	size_t GetTotalSize() const MOMO_NOEXCEPT
	{
		return sizeof(StructNumber);
	}

	size_t GetAlignment() const MOMO_NOEXCEPT
	{
		return MOMO_ALIGNMENT_OF(StructNumber);
	}

	void CreateRaw(Raw* raw)
	{
		(typename RawManager::template Creator<>(mMemManager))(raw);
	}

	void DestroyRaw(Raw* raw) const MOMO_NOEXCEPT
	{
		RawManager::Destroyer::Destroy(nullptr, *raw);
	}

	void DestroyRaw(Raw* raw) MOMO_NOEXCEPT
	{
		RawManager::Destroy(mMemManager, *raw);
	}

	void CopyRaw(const Raw* srcRaw, Raw* dstRaw)
	{
		RawManager::Copy(mMemManager, *srcRaw, dstRaw);
	}

	template<typename Type>
	size_t GetOffset(const Column<Type>& column) const MOMO_NOEXCEPT
	{
		return column.GetOffset();
	}

	template<typename Type>
	Type& GetByOffset(Raw* raw, size_t offset) const MOMO_NOEXCEPT
	{
		MOMO_ASSERT(offset < sizeof(Struct));
		MOMO_ASSERT(offset % MOMO_ALIGNMENT_OF(Type) == 0);
		return *reinterpret_cast<Type*>(reinterpret_cast<char*>(raw) + offset);
	}

	template<typename Type, typename TypeArg>
	void Assign(Raw* raw, size_t offset, TypeArg&& itemArg) const
	{
		GetByOffset<Type>(raw, offset) = std::forward<TypeArg>(itemArg);
	}

	size_t GetNumber(const Raw* raw) const MOMO_NOEXCEPT
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		return static_cast<const StructNumber*>(raw)->rowNumber;
	}

	void SetNumber(Raw* raw, size_t number) const MOMO_NOEXCEPT
	{
		MOMO_STATIC_ASSERT(Settings::keepRowNumber);
		static_cast<StructNumber*>(raw)->rowNumber = number;
	}

private:
	template<typename Type, typename... Types>
	void pvSetMutable(const Column<Type>& column, const Column<Types>&... columns)
	{
		mMutOffsets.set(GetOffset(column));
		pvSetMutable(columns...);
	}

	void pvSetMutable() MOMO_NOEXCEPT
	{
	}

private:
	MemManager mMemManager;
	MutOffsets mMutOffsets;
};

} // namespace experimental

} // namespace momo
