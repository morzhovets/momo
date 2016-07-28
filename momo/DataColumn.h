/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/DataColumn.h

  namespace momo::experimental:
    class DataColumnList
    struct DataColumnTraits
    class DataColumnListVar

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

namespace experimental
{

template<typename TStruct>
class DataColumnList
{
public:
	typedef TStruct Struct;

	template<typename Type>
	using Column = Type Struct::*;

	typedef Struct Raw;

public:
	DataColumnList() noexcept
	{
	}

	constexpr static size_t GetTotalSize() noexcept
	{
		return sizeof(Struct);
	}

	void CreateRaw(Raw* raw) const
	{
		new(raw) Raw();
	}

	void DestroyRaw(Raw* raw) const noexcept
	{
		raw->~Raw();
	}

	constexpr static bool IsMutable(size_t /*offset*/) noexcept
	{
		return false;
	}

	template<typename Type>
	size_t GetOffset(const Column<Type>& column) const noexcept
	{
		return reinterpret_cast<size_t>(std::addressof(reinterpret_cast<Struct*>(0)->*column));
		//return offsetof(Struct, *column);
	}

	template<typename Type>
	Type& GetByOffset(Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < GetTotalSize());
		MOMO_ASSERT(offset % alignof(Type) == 0);
		return *reinterpret_cast<Type*>(reinterpret_cast<char*>(raw) + offset);
	}

	template<typename Type>
	const Type& GetByOffset(const Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < GetTotalSize());
		MOMO_ASSERT(offset % alignof(Type) == 0);
		return *reinterpret_cast<const Type*>(reinterpret_cast<const char*>(raw) + offset);
	}

	template<typename Type>
	Type& GetByColumn(Raw* raw, const Column<Type>& column) const noexcept
	{
		return raw->*column;
	}

	template<typename Type>
	const Type& GetByColumn(const Raw* raw, const Column<Type>& column) const noexcept
	{
		return raw->*column;
	}

	template<typename Type, typename RType>
	void Assign(Raw* raw, const Column<Type>& column, RType&& item) const
	{
		GetByColumn(raw, column) = std::forward<RType>(item);
	}
};

template<typename TStruct>
struct DataColumnTraits
{
	typedef TStruct Struct;

	template<typename Type>
	using Column = Type Struct::*;

	template<typename Type>
	static size_t GetCode(const Column<Type>& column) noexcept
	{
		//? typeid(Type).hash_code()
		return reinterpret_cast<size_t>(std::addressof(reinterpret_cast<Struct*>(0)->*column));
		//return offsetof(Struct, *column);
	}

	template<typename Type>
	constexpr static size_t GetSize(/*const Column<Type>& column*/) noexcept
	{
		return sizeof(Type);
	}

	template<typename Type>
	constexpr static size_t GetAlignment(/*const Column<Type>& column*/) noexcept
	{
		return alignof(Type);
	}

	template<typename Type>
	static void Create(/*const Column<Type>& column,*/ Type* pitem)
	{
		new(pitem) Type();
	}

	template<typename Type>
	static void Destroy(/*const Column<Type>& column,*/ Type* pitem) noexcept
	{
		pitem->~Type();
	}

	template<typename RType, typename Type>
	static void Assign(RType&& srcItem, Type& dstItem)
	{
		dstItem = std::forward<RType>(srcItem);
	}
};

template<typename TColumnTraits,
	size_t tLogMaxColumnCount = 7>
class DataColumnListVar
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
		Graph() noexcept
		{
			mEdgeNumber = 0;
			std::fill_n(mEdges, vertexCount, nullptr);
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
		Edge mEdgeStorage[edgeCount];
		Edge* mEdges[vertexCount];
	};

	typedef std::function<void(Raw*)> CreateFunc;
	typedef std::function<void(Raw*)> DestroyFunc;

public:
	template<typename... Types>
	explicit DataColumnListVar(const Column<Types>&... columns)
	{
		static const size_t columnCount = sizeof...(columns);
		MOMO_STATIC_ASSERT(0 < columnCount && columnCount < (1 << logMaxColumnCount));
		Graph<2 * columnCount> graph;
		_MakeGraph(graph, 0, 1, columns...);
		std::fill_n(mAddends, vertexCount, 0);
		for (size_t v = 0; v < vertexCount; ++v)
		{
			if (!graph.HasEdge(v) || mAddends[v] != 0)
				continue;
			mAddends[v] = (size_t)1 << (8 * sizeof(size_t) - 1);
			if (!graph.FillAddends(mAddends, v))
				throw std::runtime_error("Cannot create DataColumnListVar");	//?
		}
		mCreateFunc = [] (Raw* raw) { _Create<void, Types...>(raw, 0); };
		mDestroyFunc = [] (Raw* raw) { _Destroy<void, Types...>(raw, 0); };
	}

	size_t GetTotalSize() const noexcept
	{
		return mTotalSize;
	}

	void CreateRaw(Raw* raw) const
	{
		mCreateFunc(raw);
	}

	void DestroyRaw(Raw* raw) const noexcept
	{
		mDestroyFunc(raw);
	}

	constexpr static bool IsMutable(size_t /*offset*/) noexcept
	{
		return false;
	}

	template<typename Type>
	size_t GetOffset(const Column<Type>& column) const noexcept
	{
		std::pair<size_t, size_t> vertices = _GetVertices(column);
		size_t addend1 = mAddends[vertices.first];
		size_t addend2 = mAddends[vertices.second];
		MOMO_ASSERT(addend1 != 0 && addend2 != 0);
		size_t offset = addend1 + addend2;
		MOMO_ASSERT(offset < mTotalSize);
		MOMO_ASSERT(offset % ColumnTraits::template GetAlignment<Type>() == 0);
		return offset;
	}

	template<typename Type>
	Type& GetByOffset(Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < mTotalSize);
		MOMO_ASSERT(offset % ColumnTraits::template GetAlignment<Type>() == 0);
		return *reinterpret_cast<Type*>(raw + offset);
	}

	template<typename Type>
	const Type& GetByOffset(const Raw* raw, size_t offset) const noexcept
	{
		MOMO_ASSERT(offset < mTotalSize);
		MOMO_ASSERT(offset % ColumnTraits::template GetAlignment<Type>() == 0);
		return *reinterpret_cast<const Type*>(raw + offset);
	}

	template<typename Type>
	Type& GetByColumn(Raw* raw, const Column<Type>& column) const noexcept
	{
		return *reinterpret_cast<Type*>(raw + GetOffset(column));
	}

	template<typename Type>
	const Type& GetByColumn(const Raw* raw, const Column<Type>& column) const noexcept
	{
		return *reinterpret_cast<const Type*>(raw + GetOffset(column));
	}

	template<typename Type, typename RType>
	void Assign(Raw* raw, const Column<Type>& column, RType&& item) const
	{
		ColumnTraits::Assign(std::forward<RType>(item), GetByColumn(raw, column));
	}

private:
	template<size_t edgeCount, typename Type, typename... Types>
	void _MakeGraph(Graph<edgeCount>& graph, size_t offset, size_t maxAlignment, const Column<Type>& column,
		const Column<Types>&... columns) noexcept
	{
		_CorrectOffset<Type>(offset);
		std::pair<size_t, size_t> vertices = _GetVertices(column);
		graph.AddEdge(vertices.first, vertices.second, offset);
		graph.AddEdge(vertices.second, vertices.first, offset);
		offset += ColumnTraits::template GetSize<Type>();
		maxAlignment = std::minmax(maxAlignment,
			ColumnTraits::template GetAlignment<Type>()).second;
		_MakeGraph(graph, offset, maxAlignment, columns...);
	}

	template<size_t edgeCount>
	void _MakeGraph(Graph<edgeCount>& /*graph*/, size_t offset, size_t maxAlignment) noexcept
	{
		mTotalSize = internal::UIntMath<size_t>::Ceil(offset, maxAlignment);
	}

	template<typename Type>
	static std::pair<size_t, size_t> _GetVertices(const Column<Type>& column) noexcept
	{
		size_t code = ColumnTraits::GetCode(column);
		size_t vertex1 = code & (vertexCount - 1);
		size_t vertex2 = (code >> logVertexCount) & (vertexCount - 1);
		if (vertex1 == vertex2)	//?
			++vertex2;
		return std::make_pair(vertex1, vertex2);
	}

	template<typename Void, typename Type, typename... Types>
	static void _Create(Raw* raw, size_t offset)
	{
		_CorrectOffset<Type>(offset);
		ColumnTraits::Create(reinterpret_cast<Type*>(raw + offset));
		try
		{
			_Create<void, Types...>(raw, offset + ColumnTraits::template GetSize<Type>());
		}
		catch (...)
		{
			ColumnTraits::Destroy(reinterpret_cast<Type*>(raw + offset));
			throw;
		}
	}

	template<typename Void>
	static void _Create(Raw* /*raw*/, size_t /*offset*/) noexcept
	{
	}

	template<typename Void, typename Type, typename... Types>
	static void _Destroy(Raw* raw, size_t offset) noexcept
	{
		_CorrectOffset<Type>(offset);
		ColumnTraits::Destroy(reinterpret_cast<Type*>(raw + offset));
		_Destroy<void, Types...>(raw, offset + ColumnTraits::template GetSize<Type>());
	}

	template<typename Void>
	static void _Destroy(Raw* /*raw*/, size_t /*offset*/) noexcept
	{
	}

	template<typename Type>
	static void _CorrectOffset(size_t& offset) noexcept
	{
		static const size_t alignment = ColumnTraits::template GetAlignment<Type>();
		offset = ((offset + alignment - 1) / alignment) * alignment;
	}

private:
	size_t mTotalSize;
	size_t mAddends[vertexCount];
	CreateFunc mCreateFunc;
	DestroyFunc mDestroyFunc;
};

} // namespace experimental

} // namespace momo
