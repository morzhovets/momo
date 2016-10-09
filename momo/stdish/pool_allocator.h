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

  Deviation from the `Allocator` concept: `pool_allocator(a) != a`.

\**********************************************************/

#pragma once

#include "../MemPool.h"

namespace momo
{

namespace stdish
{

template<typename TValue,
	typename TBaseAllocator = std::allocator<char>,
	typename TMemPoolParams = MemPoolParams<>>
class pool_allocator
{
public:
	typedef TValue value_type;

	typedef TBaseAllocator base_allocator_type;
	typedef TMemPoolParams mem_pool_params;

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
		typedef pool_allocator<Value, base_allocator_type, mem_pool_params> other;
	};

private:
	typedef momo::MemPoolParamsStatic<sizeof(value_type), MOMO_ALIGNMENT_OF(value_type),
		mem_pool_params::blockCount, mem_pool_params::cachedFreeBlockCount> MemPoolParams;
	typedef MemManagerStd<base_allocator_type> MemManager;
	typedef momo::MemPool<MemPoolParams, MemManager> MemPool;

public:
	pool_allocator()
	{
	}

	explicit pool_allocator(const base_allocator_type& alloc)
		: mMemPool(MemManager(alloc))
	{
	}

	pool_allocator(pool_allocator&& alloc) MOMO_NOEXCEPT
		: mMemPool(std::move(alloc.mMemPool))
	{
	}

	pool_allocator(const pool_allocator& alloc) MOMO_NOEXCEPT	//?
		: pool_allocator(alloc.get_base_allocator())
	{
	}

	template<class Value>
	pool_allocator(const pool_allocator<Value,
		base_allocator_type, mem_pool_params>& alloc) MOMO_NOEXCEPT	//?
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

	pool_allocator& operator=(const pool_allocator& alloc) = delete;

	base_allocator_type get_base_allocator() const MOMO_NOEXCEPT
	{
		return mMemPool.GetMemManager().GetAllocator();
	}

	pool_allocator select_on_container_copy_construction() const MOMO_NOEXCEPT
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
			return mMemPool.template Allocate<value_type>();
		else
			return mMemPool.GetMemManager().template Allocate<value_type>(count * sizeof(value_type));
	}

	void deallocate(pointer ptr, size_type count) MOMO_NOEXCEPT
	{
		if (count == 1)
			mMemPool.Deallocate(ptr);
		else
			mMemPool.GetMemManager().Deallocate(ptr, count * sizeof(value_type));
	}

	template<typename Value, typename... ValueArgs>
	void construct(Value* ptr, ValueArgs&&... valueArgs)
	{
		typedef typename internal::ObjectManager<Value, MemManager>
			::template Creator<ValueArgs...> ValueCreator;
		ValueCreator(std::forward<ValueArgs>(valueArgs)...)(ptr);
	}

	template<class Value>
	void destroy(Value* ptr) MOMO_NOEXCEPT
	{
		internal::ObjectManager<Value, MemManager>::Destroy(*ptr);
	}

	size_type max_size() const MOMO_NOEXCEPT
	{
		return SIZE_MAX / sizeof(value_type);
	}

	bool operator==(const pool_allocator& alloc) const MOMO_NOEXCEPT
	{
		return this == &alloc;
	}

	bool operator!=(const pool_allocator& alloc) const MOMO_NOEXCEPT
	{
		return this != &alloc;
	}

private:
	MemPool mMemPool;
};

template<typename TBaseAllocator, typename TMemPoolParams>
class pool_allocator<void, TBaseAllocator, TMemPoolParams>
{
public:
	typedef void value_type;

	typedef TBaseAllocator base_allocator_type;
	typedef TMemPoolParams mem_pool_params;

	typedef void* pointer;
	typedef const void* const_pointer;

	template<typename Value>
	struct rebind
	{
		typedef pool_allocator<Value, base_allocator_type, mem_pool_params> other;
	};
};

} // namespace stdish

} // namespace momo
