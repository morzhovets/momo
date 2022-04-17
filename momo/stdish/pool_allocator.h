/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/stdish/pool_allocator.h

  namespace momo::stdish:
    class unsynchronized_pool_allocator

\**********************************************************/

#pragma once

#include "../MemPool.h"

namespace momo
{

namespace stdish
{

/*!
	\brief
	Allocator with a pool of memory for containers like `std::list`,
	`std::forward_list`, `std::map`, `std::unordered_map`.
	It makes no sense to use this allocator for classes `momo::stdish`.

	\details
	Each copy of the container keeps its own memory pool.
	Memory is released not only after destruction of the object,
	but also in case of removal sufficient number of items.
*/

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
	typedef MemManagerStd<base_allocator_type, false> MemManager;
	typedef internal::MemManagerProxy<MemManager> MemManagerProxy;

	typedef mem_pool_params MemPoolParams;
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

	~unsynchronized_pool_allocator() = default;

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
		return base_allocator_type(mMemPool->GetMemManager().GetByteAllocator());
	}

	unsynchronized_pool_allocator select_on_container_copy_construction() const noexcept
	{
		return unsynchronized_pool_allocator(get_base_allocator());
	}

	MOMO_NODISCARD pointer allocate(size_type count)
	{
		if (count == 1)
		{
			MemPoolParams memPoolParams = pvGetMemPoolParams();
			bool equal = pvIsEqual(memPoolParams, mMemPool->GetParams());
			if (!equal && mMemPool->GetAllocateCount() == 0)
			{
				*mMemPool = MemPool(memPoolParams, MemManager(get_base_allocator()));
				equal = true;
			}
			if (equal)
				return mMemPool->template Allocate<value_type>();
		}
		return MemManagerProxy::template Allocate<value_type>(mMemPool->GetMemManager(),
			count * sizeof(value_type));
	}

	void deallocate(pointer ptr, size_type count) noexcept
	{
		if (count == 1 && pvIsEqual(pvGetMemPoolParams(), mMemPool->GetParams()))
			return mMemPool->Deallocate(ptr);
		MemManagerProxy::Deallocate(mMemPool->GetMemManager(), ptr, count * sizeof(value_type));
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
		return MemPoolParams(sizeof(value_type),
			internal::ObjectAlignmenter<value_type>::alignment);
	}

	static bool pvIsEqual(const MemPoolParams& memPoolParams1,
		const MemPoolParams& memPoolParams2) noexcept
	{
		return memPoolParams1.GetBlockSize() == memPoolParams2.GetBlockSize()
			&& memPoolParams1.GetBlockAlignment() == memPoolParams2.GetBlockAlignment();
	}

private:
	std::shared_ptr<MemPool> mMemPool;
};

} // namespace stdish

} // namespace momo
