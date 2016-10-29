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

template<typename TStruct>
struct DataColumnTraits
{
	typedef TStruct Struct;

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
	static void Create(/*const Column<Type>& column,*/ Type* pitem)
	{
		new(pitem) Type();
	}

	template<typename Type>
	static void Destroy(/*const Column<Type>& column,*/ Type* pitem) MOMO_NOEXCEPT
	{
		pitem->~Type();
	}

	template<typename TypeArg, typename Type>
	static void Assign(TypeArg&& itemArg, Type& item)
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

	typedef std::function<void(Raw*)> CreateFunc;
	typedef std::function<void(Raw*)> DestroyFunc;

public:
	template<typename... Types>
	explicit DataColumnList(const Column<Types>&... columns)
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
		mCreateFunc = [] (Raw* raw) { pvCreate<void, Types...>(raw, 0); };
		mDestroyFunc = [] (Raw* raw) { pvDestroy<void, Types...>(raw, 0); };
	}

	static constexpr bool IsMutable(size_t /*offset*/) MOMO_NOEXCEPT
	{
		return false;
	}

	size_t GetTotalSize() const MOMO_NOEXCEPT
	{
		return mTotalSize;
	}

	void CreateRaw(Raw* raw) const
	{
		mCreateFunc(raw);
	}

	void DestroyRaw(Raw* raw) const MOMO_NOEXCEPT
	{
		mDestroyFunc(raw);
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
		ColumnTraits::Assign(std::forward<TypeArg>(itemArg), GetByOffset<Type>(raw, offset));
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
	static void pvCreate(Raw* raw, size_t offset)
	{
		pvCorrectOffset<Type>(offset);
		ColumnTraits::Create(reinterpret_cast<Type*>(raw + offset));
		try
		{
			pvCreate<void, Types...>(raw, offset + ColumnTraits::template GetSize<Type>());
		}
		catch (...)
		{
			ColumnTraits::Destroy(reinterpret_cast<Type*>(raw + offset));
			throw;
		}
	}

	template<typename Void>
	static void pvCreate(Raw* /*raw*/, size_t /*offset*/) MOMO_NOEXCEPT
	{
	}

	template<typename Void, typename Type, typename... Types>
	static void pvDestroy(Raw* raw, size_t offset) MOMO_NOEXCEPT
	{
		pvCorrectOffset<Type>(offset);
		ColumnTraits::Destroy(reinterpret_cast<Type*>(raw + offset));
		pvDestroy<void, Types...>(raw, offset + ColumnTraits::template GetSize<Type>());
	}

	template<typename Void>
	static void pvDestroy(Raw* /*raw*/, size_t /*offset*/) MOMO_NOEXCEPT
	{
	}

	template<typename Type>
	static void pvCorrectOffset(size_t& offset) MOMO_NOEXCEPT
	{
		static const size_t alignment = ColumnTraits::template GetAlignment<Type>();
		offset = ((offset + alignment - 1) / alignment) * alignment;
	}

private:
	size_t mTotalSize;
	Addends mAddends;
	CreateFunc mCreateFunc;
	DestroyFunc mDestroyFunc;
};

template<typename TStruct>
class DataColumnListStatic
{
public:
	typedef TStruct Struct;

	template<typename Type>
	using Column = Type Struct::*;

	typedef Struct Raw;

private:
	static const size_t structSize = sizeof(Struct);

	typedef std::bitset<structSize> MutOffsets;

public:
	DataColumnListStatic() MOMO_NOEXCEPT
	{
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

	static constexpr size_t GetTotalSize() MOMO_NOEXCEPT
	{
		return structSize;
	}

	void CreateRaw(Raw* raw) const
	{
		new(raw) Raw();
	}

	void DestroyRaw(Raw* raw) const MOMO_NOEXCEPT
	{
		(void)raw;
		raw->~Raw();
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
	MutOffsets mMutOffsets;
};

} // namespace experimental

} // namespace momo
