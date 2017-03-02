/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/DataColumn.h

  namespace momo::experimental:
    struct DataColumnTraits
    class DataColumnList
    class DataColumnListStatic

\**********************************************************/

#pragma once

#include "ObjectManager.h"

#include <bitset>

namespace momo
{

namespace experimental
{

template<typename TStruct,
	typename TMemManager = MemManagerDefault>
struct DataColumnTraits
{
	typedef TStruct Struct;
	typedef TMemManager MemManager;

	template<typename Type>
	using Column = Type Struct::*;

	template<typename Type>
	static size_t GetCode(const Column<Type>& column) MOMO_NOEXCEPT
	{
		//? typeid(Type).hash_code()
		return reinterpret_cast<size_t>(std::addressof(reinterpret_cast<Struct*>(0)->*column));
		//return offsetof(Struct, *column);
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
	static void Create(MemManager& memManager, Type* pitem /*, const Column<Type>& column*/)
	{
		(typename momo::internal::ObjectManager<Type, MemManager>::template Creator<>(memManager))(pitem);
	}

	template<typename Type>
	static void Destroy(MemManager& memManager, Type* pitem /*, const Column<Type>& column*/) MOMO_NOEXCEPT
	{
		momo::internal::ObjectManager<Type, MemManager>::Destroy(memManager, *pitem);
	}

	template<typename TypeArg, typename Type>
	static void Assign(/*MemManager& memManager,*/ TypeArg&& itemArg, Type& item)
	{
		item = std::forward<TypeArg>(itemArg);
	}
};

template<typename TColumnTraits,
	size_t tLogMaxColumnCount = 7>
class DataColumnList
{
public:
	typedef TColumnTraits ColumnTraits;
	typedef typename ColumnTraits::MemManager MemManager;

	static const size_t logMaxColumnCount = tLogMaxColumnCount;
	MOMO_STATIC_ASSERT(logMaxColumnCount <= 15);

	template<typename Type>
	using Column = typename ColumnTraits::template Column<Type>;

	typedef char Raw;

private:
	static const size_t logVertexCount = logMaxColumnCount + 1;	//?
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

	typedef std::function<void(MemManager&, Raw*)> CreateFunc;
	typedef std::function<void(MemManager&, Raw*)> DestroyFunc;

public:
	template<typename... Types>
	explicit DataColumnList(const Column<Types>&... columns)
		: DataColumnList(MemManager(), columns...)
	{
	}

	template<typename... Types>
	explicit DataColumnList(MemManager&& memManager, const Column<Types>&... columns)
		: mMemManager(std::move(memManager))
	{
		static const size_t columnCount = sizeof...(columns);
		MOMO_STATIC_ASSERT(0 < columnCount && columnCount < (1 << logMaxColumnCount));
		Graph<2 * columnCount> graph;
		pvMakeGraph(graph, 0, 1, columns...);
		std::fill(mAddends.begin(), mAddends.end(), 0);
		for (size_t v = 0; v < vertexCount; ++v)
		{
			if (!graph.HasEdge(v) || mAddends[v] != 0)
				continue;
			mAddends[v] = (size_t)1 << (8 * sizeof(size_t) - 1);
			if (!graph.FillAddends(mAddends.data(), v))
				throw std::runtime_error("Cannot create DataColumnListVar");	//?
		}
		mCreateFunc = [] (MemManager& memManager, Raw* raw)
			{ pvCreate<void, Types...>(memManager, raw, 0); };
		mDestroyFunc = [] (MemManager& memManager, Raw* raw)
			{ pvDestroy<void, Types...>(memManager, raw, 0); };
	}

	DataColumnList(DataColumnList&& columnList) MOMO_NOEXCEPT
		: mMemManager(std::move(columnList.mMemManager)),
		mTotalSize(columnList.mTotalSize),
		mAddends(columnList.mAddends),
		mCreateFunc(std::move(columnList.mCreateFunc)),
		mDestroyFunc(std::move(columnList.mDestroyFunc))
	{
	}

	DataColumnList(const DataColumnList& columnList)
		: mMemManager(columnList.mMemManager),
		mTotalSize(columnList.mTotalSize),
		mAddends(columnList.mAddends),
		mCreateFunc(columnList.mCreateFunc),
		mDestroyFunc(columnList.mDestroyFunc)
	{
	}

	~DataColumnList() MOMO_NOEXCEPT
	{
	}

	DataColumnList& operator=(const DataColumnList&) = delete;

	const MemManager& GetMemManager() const MOMO_NOEXCEPT
	{
		return mMemManager;
	}

	MemManager& GetMemManager() MOMO_NOEXCEPT
	{
		return mMemManager;
	}

	bool IsMutable(size_t /*offset*/) const MOMO_NOEXCEPT
	{
		return false;	//?
	}

	size_t GetTotalSize() const MOMO_NOEXCEPT
	{
		return mTotalSize;
	}

	void CreateRaw(Raw* raw)
	{
		mCreateFunc(mMemManager, raw);
	}

	void DestroyRaw(Raw* raw) MOMO_NOEXCEPT
	{
		mDestroyFunc(mMemManager, raw);
	}

	template<typename Type>
	size_t GetOffset(const Column<Type>& column) const MOMO_NOEXCEPT
	{
		std::pair<size_t, size_t> vertices = pvGetVertices(column);
		size_t addend1 = mAddends[vertices.first];
		size_t addend2 = mAddends[vertices.second];
		MOMO_ASSERT(addend1 != 0 && addend2 != 0);
		size_t offset = addend1 + addend2;
		MOMO_ASSERT(offset < mTotalSize);
		MOMO_ASSERT(offset % ColumnTraits::template GetAlignment<Type>() == 0);
		return offset;
	}

	template<typename Type>
	const Type& GetByOffset(const Raw* raw, size_t offset) const MOMO_NOEXCEPT
	{
		MOMO_ASSERT(offset < mTotalSize);
		MOMO_ASSERT(offset % ColumnTraits::template GetAlignment<Type>() == 0);
		return *reinterpret_cast<const Type*>(raw + offset);
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
		ColumnTraits::Assign(/*mMemManager,*/ std::forward<TypeArg>(itemArg),
			GetByOffset<Type>(raw, offset));
	}

private:
	template<size_t edgeCount, typename Type, typename... Types>
	void pvMakeGraph(Graph<edgeCount>& graph, size_t offset, size_t maxAlignment,
		const Column<Type>& column, const Column<Types>&... columns) MOMO_NOEXCEPT
	{
		pvCorrectOffset<Type>(offset);
		std::pair<size_t, size_t> vertices = pvGetVertices(column);
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
		mTotalSize = momo::internal::UIntMath<size_t>::Ceil(offset, maxAlignment);
	}

	template<typename Type>
	static std::pair<size_t, size_t> pvGetVertices(const Column<Type>& column) MOMO_NOEXCEPT
	{
		size_t code = ColumnTraits::GetCode(column);
		size_t vertex1 = code & (vertexCount - 1);
		size_t vertex2 = (code >> logVertexCount) & (vertexCount - 1);
		if (vertex1 == vertex2)	//?
			++vertex2;
		return std::make_pair(vertex1, vertex2);
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
			ColumnTraits::Destroy(memManager, reinterpret_cast<Type*>(raw + offset));
			throw;
		}
	}

	template<typename Void>
	static void pvCreate(MemManager& /*memManager*/, Raw* /*raw*/, size_t /*offset*/) MOMO_NOEXCEPT
	{
	}

	template<typename Void, typename Type, typename... Types>
	static void pvDestroy(MemManager& memManager, Raw* raw, size_t offset) MOMO_NOEXCEPT
	{
		pvCorrectOffset<Type>(offset);
		ColumnTraits::Destroy(memManager, reinterpret_cast<Type*>(raw + offset));
		pvDestroy<void, Types...>(memManager, raw, offset + ColumnTraits::template GetSize<Type>());
	}

	template<typename Void>
	static void pvDestroy(MemManager& /*memManager*/, Raw* /*raw*/, size_t /*offset*/) MOMO_NOEXCEPT
	{
	}

	template<typename Type>
	static void pvCorrectOffset(size_t& offset) MOMO_NOEXCEPT
	{
		static const size_t alignment = ColumnTraits::template GetAlignment<Type>();
		offset = ((offset + alignment - 1) / alignment) * alignment;
	}

private:
	MemManager mMemManager;
	size_t mTotalSize;
	Addends mAddends;
	CreateFunc mCreateFunc;
	DestroyFunc mDestroyFunc;
};

template<typename TStruct,
	typename TMemManager = MemManagerDefault>
class DataColumnListStatic
{
public:
	typedef TStruct Struct;
	typedef TMemManager MemManager;

	template<typename Type>
	using Column = Type Struct::*;

	typedef Struct Raw;

private:
	typedef momo::internal::ObjectManager<Raw, MemManager> RawManager;

	static const size_t structSize = sizeof(Struct);

	typedef std::bitset<structSize> MutOffsets;

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
		return structSize;
	}

	void CreateRaw(Raw* raw)
	{
		(typename RawManager::template Creator<>(mMemManager))(raw);
	}

	void DestroyRaw(Raw* raw) MOMO_NOEXCEPT
	{
		RawManager::Destroy(mMemManager, *raw);
	}

	template<typename Type>
	size_t GetOffset(const Column<Type>& column) const MOMO_NOEXCEPT
	{
		return reinterpret_cast<size_t>(std::addressof(reinterpret_cast<Struct*>(0)->*column));
		//return offsetof(Struct, *column);
	}

	template<typename Type>
	const Type& GetByOffset(const Raw* raw, size_t offset) const MOMO_NOEXCEPT
	{
		MOMO_ASSERT(offset < structSize);
		MOMO_ASSERT(offset % alignof(Type) == 0 || offset % alignof(Struct) == 0);
		return *reinterpret_cast<const Type*>(reinterpret_cast<const char*>(raw) + offset);
	}

	template<typename Type>
	Type& GetByOffset(Raw* raw, size_t offset) const MOMO_NOEXCEPT
	{
		MOMO_ASSERT(offset < structSize);
		MOMO_ASSERT(offset % alignof(Type) == 0 || offset % alignof(Struct) == 0);
		return *reinterpret_cast<Type*>(reinterpret_cast<char*>(raw) + offset);
	}

	template<typename Type, typename TypeArg>
	void Assign(Raw* raw, size_t offset, TypeArg&& itemArg) const
	{
		GetByOffset<Type>(raw, offset) = std::forward<TypeArg>(itemArg);
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
