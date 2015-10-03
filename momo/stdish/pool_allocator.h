/**********************************************************\

  momo/stdish/pool_allocator.h

  namespace momo::stdish:
    class pool_allocator

\**********************************************************/

#pragma once

#include "../MemPool.h"

namespace momo
{

namespace stdish
{

template<typename TValue>
class pool_allocator
{
public:
	typedef TValue value_type;

	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	typedef value_type& reference;
	typedef const value_type& const_reference;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef std::false_type propagate_on_container_copy_assignment;
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type propagate_on_container_swap;

	template<typename Value>
	struct rebind
	{
		typedef pool_allocator<Value> other;
	};

private:
	typedef MemPool<MemPoolParams<sizeof(value_type), std::alignment_of<value_type>::value>> MemPool;
	typedef typename MemPool::Params MemPoolParams;
	typedef typename MemPool::MemManager MemManager;

public:
	pool_allocator()
		: mMemPool(MemPoolParams(), MemManager())
	{
	}

	pool_allocator(pool_allocator&& alloc) MOMO_NOEXCEPT
		: mMemPool(std::move(alloc.mMemPool))
	{
	}

	pool_allocator(const pool_allocator& /*alloc*/)
		: pool_allocator()
	{
	}

	template<class Value>
	pool_allocator(const pool_allocator<Value>& /*alloc*/)
		: pool_allocator()
	{
	}

	~pool_allocator() MOMO_NOEXCEPT
	{
	}

	pool_allocator& operator=(pool_allocator&& alloc) MOMO_NOEXCEPT
	{
		mMemPool = std::move(alloc.mMemPool);
		return *this;
	}

	pool_allocator& operator=(const pool_allocator& /*alloc*/) MOMO_NOEXCEPT
	{
		return *this;
	}

	template<class Value>
	pool_allocator& operator=(const pool_allocator<Value>& /*alloc*/) MOMO_NOEXCEPT
	{
		return *this;
	}

	pool_allocator select_on_container_copy_construction() const
	{
		return pool_allocator();
	}

	pointer address(reference ref) const MOMO_NOEXCEPT
	{
		return std::addressof(ref);
	}

	const_pointer address(const_reference ref) const MOMO_NOEXCEPT
	{
		return std::addressof(ref);
	}

	pointer allocate(size_type count, const void* = nullptr)
	{
		if (count == 1)
			return (pointer)mMemPool.Allocate();
		else
			return (pointer)mMemPool.GetMemManager().Allocate(count * sizeof(value_type));
	}

	void deallocate(pointer ptr, size_type count) MOMO_NOEXCEPT
	{
		if (count == 1)
			mMemPool.Deallocate(ptr);
		else
			mMemPool.GetMemManager().Deallocate(ptr, count * sizeof(value_type));
	}

	template<typename Value, typename... Args>
	void construct(Value* ptr, Args&&... args)
	{
		typedef typename internal::ObjectManager<Value>::template VariadicCreator<Args...> ValueCreator;
		ValueCreator(std::forward<Args>(args)...)(ptr);
	}

	template<class Value>
	void destroy(Value* ptr) MOMO_NOEXCEPT
	{
		internal::ObjectManager<Value>::Destroy(*ptr);
	}

	size_type max_size() const MOMO_NOEXCEPT
	{
		return SIZE_MAX / sizeof(value_type);
	}

	bool operator==(const pool_allocator& /*alloc*/) const MOMO_NOEXCEPT
	{
		return false;
	}

	bool operator!=(const pool_allocator& /*alloc*/) const MOMO_NOEXCEPT
	{
		return true;
	}

private:
	MemPool mMemPool;
};

template<>
class pool_allocator<void>
{
public:
	typedef void value_type;

	typedef void* pointer;
	typedef const void* const_pointer;

	template<typename Value>
	struct rebind
	{
		typedef pool_allocator<Value> other;
	};
};

} // namespace stdish

} // namespace momo
