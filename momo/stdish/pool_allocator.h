/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/stdish/pool_allocator.h

  namespace momo::stdish:
    class unsynchronized_pool_allocator

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
	typename TBaseAllocator = std::allocator<char>,
	typename TMemPoolParams = MemPoolParams<>>
class unsynchronized_pool_allocator
{
public:
	typedef TValue value_type;

	typedef TBaseAllocator base_allocator_type;
	typedef TMemPoolParams mem_pool_params;

	typedef value_type* pointer;
	typedef const value_type* const_pointer;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef std::false_type propagate_on_container_copy_assignment;
	typedef std::true_type propagate_on_container_move_assignment;
	typedef std::true_type propagate_on_container_swap;

private:
	typedef mem_pool_params MemPoolParams;
	typedef MemManagerStd<base_allocator_type> MemManager;
	typedef momo::MemPool<MemPoolParams, MemManager> MemPool;

	template<typename Value>
	struct PoolAllocatorProxy
		: public unsynchronized_pool_allocator<Value, base_allocator_type, mem_pool_params>
	{
		typedef unsynchronized_pool_allocator<Value, base_allocator_type, mem_pool_params> PoolAllocator;
		MOMO_DECLARE_PROXY_CONSTRUCTOR(PoolAllocator)
	};

public:
	explicit unsynchronized_pool_allocator(const base_allocator_type& alloc = base_allocator_type())
		: mMemPool(std::allocate_shared<MemPool>(alloc, pvGetMemPoolParams(), MemManager(alloc)))
	{
	}

	unsynchronized_pool_allocator(const unsynchronized_pool_allocator& alloc) noexcept
		: mMemPool(alloc.mMemPool)
	{
	}

	~unsynchronized_pool_allocator() noexcept
	{
	}

	unsynchronized_pool_allocator& operator=(const unsynchronized_pool_allocator& alloc) noexcept
	{
		mMemPool = alloc.mMemPool;
		return *this;
	}

	template<class Value>
	operator unsynchronized_pool_allocator<Value, base_allocator_type, mem_pool_params>() const
		noexcept
	{
		return PoolAllocatorProxy<Value>(mMemPool);
	}

	base_allocator_type get_base_allocator() const noexcept
	{
		return base_allocator_type(mMemPool->GetMemManager().GetCharAllocator());
	}

	unsynchronized_pool_allocator select_on_container_copy_construction() const noexcept
	{
		return unsynchronized_pool_allocator(get_base_allocator());
	}

	pointer allocate(size_type count)
	{
		if (count > 1)
			return mMemPool->GetMemManager().template Allocate<value_type>(count * sizeof(value_type));
		MemPoolParams memPoolParams = pvGetMemPoolParams();
		if (!mMemPool->GetParams().IsEqual(memPoolParams) && mMemPool->GetAllocateCount() == 0)
			*mMemPool = MemPool(memPoolParams, MemManager(get_base_allocator()));
		if (!mMemPool->GetParams().IsEqual(memPoolParams))
			return mMemPool->GetMemManager().template Allocate<value_type>(sizeof(value_type));
		return mMemPool->template Allocate<value_type>();
	}

	void deallocate(pointer ptr, size_type count) noexcept
	{
		if (count > 1 || !mMemPool->GetParams().IsEqual(pvGetMemPoolParams()))
			return mMemPool->GetMemManager().Deallocate(ptr, count * sizeof(value_type));
		mMemPool->Deallocate(ptr);
	}

	template<typename Value, typename... ValueArgs>
	void construct(Value* ptr, ValueArgs&&... valueArgs)
	{
		typedef typename momo::internal::ObjectManager<Value, MemManager>
			::template Creator<ValueArgs...> ValueCreator;
		ValueCreator(mMemPool->GetMemManager(), std::forward<ValueArgs>(valueArgs)...)(ptr);
	}

	template<class Value>
	void destroy(Value* ptr) noexcept
	{
		momo::internal::ObjectManager<Value, MemManager>::Destroy(mMemPool->GetMemManager(), *ptr);
	}

	bool operator==(const unsynchronized_pool_allocator& alloc) const noexcept
	{
		return mMemPool == alloc.mMemPool;
	}

	bool operator!=(const unsynchronized_pool_allocator& alloc) const noexcept
	{
		return !(*this == alloc);
	}

protected:
	explicit unsynchronized_pool_allocator(const std::shared_ptr<MemPool>& memPool) noexcept
		: mMemPool(memPool)
	{
	}

private:
	static MemPoolParams pvGetMemPoolParams() noexcept
	{
		return MemPoolParams(sizeof(value_type), MOMO_ALIGNMENT_OF(value_type));
	}

private:
	std::shared_ptr<MemPool> mMemPool;
};

} // namespace stdish

} // namespace momo
