/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/stdish/pool_allocator.h

  namespace momo::stdish:
    class pool_allocator

  Allocator with a pool of memory for containers like `std::list`,
  `std::forward_list`, `std::map`, `std::unordered_map`.
  It makes no sense to use this allocator for classes `momo::stdish`.

  Each copy of the container keeps its own memory pool.
  Memory is released not only after destruction of the object,
  but also in case of removal sufficient number of items.

\**********************************************************/

#pragma once

#include "../MemPool.h"

namespace momo
{

namespace stdish
{

template<typename TValue,
	typename TBaseAllocator = std::allocator<char>>
class pool_allocator
{
public:
	typedef TValue value_type;
	typedef TBaseAllocator base_allocator;

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
		typedef pool_allocator<Value, base_allocator> other;
	};

private:
	typedef momo::MemPoolParams<sizeof(value_type), MOMO_ALIGNMENT_OF(value_type)> MemPoolParams;
	typedef MemManagerStd<base_allocator> MemManager;
	typedef momo::MemPool<MemPoolParams, MemManager> MemPool;

public:
	pool_allocator()
	{
	}

	explicit pool_allocator(const base_allocator& alloc)
		: mMemPool(MemPoolParams(), MemManager(alloc))
	{
	}

	pool_allocator(pool_allocator&& alloc) MOMO_NOEXCEPT
		: mMemPool(std::move(alloc.mMemPool))
	{
	}

	pool_allocator(const pool_allocator& alloc)
		: pool_allocator(alloc.get_base_allocator())
	{
	}

	template<class Value>
	pool_allocator(const pool_allocator<Value>& alloc)
		: pool_allocator(alloc.get_base_allocator())
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
		//?
		return *this;
	}

	template<class Value>
	pool_allocator& operator=(const pool_allocator<Value>& /*alloc*/) MOMO_NOEXCEPT
	{
		//?
		return *this;
	}

	base_allocator get_base_allocator() const
	{
		return mMemPool.GetMemManager().GetAllocator();
	}

	pool_allocator select_on_container_copy_construction() const
	{
		return pool_allocator(get_base_allocator());
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
		typedef typename internal::ObjectManager<Value>::template Creator<Args...> ValueCreator;
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

template<typename TBaseAllocator>
class pool_allocator<void, TBaseAllocator>
{
public:
	typedef void value_type;
	typedef TBaseAllocator base_allocator;

	typedef void* pointer;
	typedef const void* const_pointer;

	template<typename Value>
	struct rebind
	{
		typedef pool_allocator<Value, base_allocator> other;
	};
};

} // namespace stdish

} // namespace momo
