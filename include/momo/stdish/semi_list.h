/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/stdish/semi_list.h

  namespace momo::stdish:
    class semi_list_adaptor
    class semi_list

\**********************************************************/

#pragma once

#include "../SemiList.h"

namespace momo::stdish
{

template<typename TSemiList>
class semi_list_adaptor
	: public momo::internal::Swappable<semi_list_adaptor>
{
private:
	typedef TSemiList SemiList;
	typedef typename SemiList::MemManager MemManager;

public:
	typedef typename SemiList::Item value_type;
	typedef typename std::allocator_traits<typename MemManager::ByteAllocator>
		::template rebind_alloc<value_type> allocator_type;

	typedef SemiList nested_container_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef typename SemiList::Iterator iterator;
	typedef typename SemiList::ConstIterator const_iterator;

	typedef value_type& reference;
	typedef const value_type& const_reference;

	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	//typedef typename std::allocator_traits<allocator_type>::pointer pointer;
	//typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
	semi_list_adaptor()
	{
	}

	explicit semi_list_adaptor(const allocator_type& alloc)
		: mSemiList(MemManager(alloc))
	{
	}

	explicit semi_list_adaptor(size_type count, const allocator_type& alloc = allocator_type())
		: mSemiList(count, MemManager(alloc))
	{
	}

	semi_list_adaptor(size_type count, const value_type& value, const allocator_type& alloc = allocator_type())
		: mSemiList(count, value, MemManager(alloc))
	{
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	semi_list_adaptor(Iterator first, Iterator last, const allocator_type& alloc = allocator_type())
		: mSemiList(first, last, MemManager(alloc))
	{
	}

	semi_list_adaptor(std::initializer_list<value_type> values, const allocator_type& alloc = allocator_type())
		: mSemiList(values, MemManager(alloc))
	{
	}

#if defined(__cpp_lib_containers_ranges)
	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	semi_list_adaptor(std::from_range_t, Range&& values, const allocator_type& alloc = allocator_type())
		: mSemiList(std::ranges::begin(values), std::ranges::end(values), MemManager(alloc))
	{
	}
#endif

	semi_list_adaptor(semi_list_adaptor&& right)
		: semi_list_adaptor(std::move(right), right.get_allocator())
	{
	}

	semi_list_adaptor(semi_list_adaptor&& right, const allocator_type& alloc)
		: semi_list_adaptor(alloc)
	{
		if (right.get_allocator() == alloc)
		{
			mSemiList.Swap(right.mSemiList);
		}
		else
		{
			pvAssign(std::make_move_iterator(right.begin()), std::make_move_iterator(right.end()));
			right.mSemiList.Clear();
		}
	}

	semi_list_adaptor(const semi_list_adaptor& right)
		: mSemiList(right.mSemiList)
	{
	}

	semi_list_adaptor(const semi_list_adaptor& right, const allocator_type& alloc)
		: mSemiList(right.mSemiList, MemManager(alloc))
	{
	}

	~semi_list_adaptor() noexcept = default;

	semi_list_adaptor& operator=(semi_list_adaptor&& right)
		noexcept(momo::internal::ContainerAssignerStd::isNothrowMoveAssignable<semi_list_adaptor>)
	{
		return momo::internal::ContainerAssignerStd::Move(std::move(right), *this);
	}

	semi_list_adaptor& operator=(const semi_list_adaptor& right)
	{
		return momo::internal::ContainerAssignerStd::Copy(right, *this);
	}

	template<momo::internal::conceptMutableThis RSemiList>
	std::remove_reference_t<RSemiList>& operator=(this RSemiList&& left,
		std::initializer_list<value_type> values)
	{
		left.assign(values);
		return left;
	}

	void swap(semi_list_adaptor& right) noexcept
	{
		momo::internal::ContainerAssignerStd::Swap(*this, right);
	}

	const nested_container_type& get_nested_container() const noexcept
	{
		return mSemiList;
	}

	nested_container_type& get_nested_container() noexcept
	{
		return mSemiList;
	}

	const_iterator begin() const noexcept
	{
		return mSemiList.GetBegin();
	}

	iterator begin() noexcept
	{
		return mSemiList.GetBegin();
	}

	const_iterator end() const noexcept
	{
		return mSemiList.GetEnd();
	}

	iterator end() noexcept
	{
		return mSemiList.GetEnd();
	}

	const_reverse_iterator rbegin() const noexcept
	{
		return const_reverse_iterator(end());
	}

	reverse_iterator rbegin() noexcept
	{
		return reverse_iterator(end());
	}

	const_reverse_iterator rend() const noexcept
	{
		return const_reverse_iterator(begin());
	}

	reverse_iterator rend() noexcept
	{
		return reverse_iterator(begin());
	}

	const_iterator cbegin() const noexcept
	{
		return begin();
	}

	const_iterator cend() const noexcept
	{
		return end();
	}

	const_reverse_iterator crbegin() const noexcept
	{
		return rbegin();
	}

	const_reverse_iterator crend() const noexcept
	{
		return rend();
	}

	allocator_type get_allocator() const noexcept
	{
		return allocator_type(mSemiList.GetMemManager().GetByteAllocator());
	}

	size_type max_size() const noexcept
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const noexcept
	{
		return mSemiList.GetCount();
	}

	void resize(size_type size)
	{
		mSemiList.SetCount(size);
	}

	void resize(size_type size, const value_type& value)
	{
		mSemiList.SetCount(size, value);
	}

	[[nodiscard]] bool empty() const noexcept
	{
		return mSemiList.IsEmpty();
	}

	void clear() noexcept
	{
		mSemiList.Clear();
	}

	reference front()
	{
		return mSemiList.GetFrontItem();
	}

	const_reference front() const
	{
		return mSemiList.GetFrontItem();
	}

	reference back()
	{
		return mSemiList.GetBackItem();
	}

	const_reference back() const
	{
		return mSemiList.GetBackItem();
	}

	void push_front(value_type&& value)
	{
		mSemiList.AddFront(std::move(value));
	}

	void push_front(const value_type& value)
	{
		mSemiList.AddFront(value);
	}

	template<typename... ValueArgs>
	reference emplace_front(ValueArgs&&... valueArgs)
	{
		mSemiList.AddFrontVar(std::forward<ValueArgs>(valueArgs)...);
		return front();
	}

	//template<std::ranges::input_range Range>
	//requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	//void prepend_range(Range&& values)

	void push_back(value_type&& value)
	{
		mSemiList.AddBack(std::move(value));
	}

	void push_back(const value_type& value)
	{
		mSemiList.AddBack(value);
	}

	template<typename... ValueArgs>
	reference emplace_back(ValueArgs&&... valueArgs)
	{
		mSemiList.AddBackVar(std::forward<ValueArgs>(valueArgs)...);
		return back();
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	void append_range(Range&& values)
	{
		auto end = std::ranges::end(values);
		for (auto iter = std::ranges::begin(values); iter != end; ++iter)
			emplace_back(*iter);
	}

	void pop_front()
	{
		mSemiList.RemoveFront();
	}

	void pop_back()
	{
		mSemiList.RemoveBack();
	}

	iterator erase(const_iterator where)
	{
		return mSemiList.Remove(where);
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		return mSemiList.Remove(first, last);
	}

	template<typename ValueArg>
	size_type remove(const ValueArg& valueArg)
	{
		auto valueFilter = [&valueArg] (const value_type& value)
			{ return value == valueArg; };
		return mSemiList.Remove(valueFilter);
	}

	template<momo::internal::conceptPredicate<const_reference> ValueFilter>
	size_type remove_if(ValueFilter valueFilter)
	{
		return mSemiList.Remove(momo::FastCopyableFunctor(valueFilter));
	}

	template<typename ValueArg>
	friend size_type erase(semi_list_adaptor& cont, const ValueArg& valueArg)
	{
		return cont.remove(valueArg);
	}

	template<momo::internal::conceptPredicate<const_reference> ValueFilter>
	friend size_type erase_if(semi_list_adaptor& cont, ValueFilter valueFilter)
	{
		return cont.remove_if(momo::FastCopyableFunctor(valueFilter));
	}

	void assign(size_type count, const value_type& value)
	{
		mSemiList = SemiList(count, value, MemManager(get_allocator()));
	}

	template<momo::internal::conceptIterator17<std::input_iterator_tag> Iterator>
	void assign(Iterator first, Iterator last)
	{
		pvAssign(first, last);
	}

	void assign(std::initializer_list<value_type> values)
	{
		pvAssign(values.begin(), values.end());
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	void assign_range(Range&& values)
	{
		pvAssign(std::ranges::begin(values), std::ranges::end(values));
	}

	//size_type unique();

	//template<conceptEqualComparer<value_type> ValueEqualComparer>
	//size_type unique(ValueEqualComparer valueEqualComparer);

	bool operator==(const semi_list_adaptor& right) const
	{
		return mSemiList.IsEqual(right.mSemiList);
	}

	auto operator<=>(const semi_list_adaptor& right) const
		requires requires (const SemiList& list) { list.Compare(list); }
	{
		return mSemiList.Compare(right.mSemiList);
	}

private:
	template<std::input_iterator Iterator,
		momo::internal::conceptSentinel<Iterator> Sentinel>
	void pvAssign(Iterator begin, Sentinel end)
	{
		mSemiList = SemiList(std::move(begin), std::move(end), MemManager(get_allocator()));
	}

private:
	SemiList mSemiList;
};

template<typename TValue,
	typename TAllocator = std::allocator<TValue>>
class MOMO_EMPTY_BASES semi_list
	: public semi_list_adaptor<SemiList<TValue, MemManagerStd<TAllocator>>>,
	public momo::internal::Swappable<semi_list>
{
private:
	typedef semi_list_adaptor<SemiList<TValue, MemManagerStd<TAllocator>>> SemiListAdaptor;

public:
	using SemiListAdaptor::SemiListAdaptor;

	using SemiListAdaptor::operator=;
};

template<typename Value,
	momo::internal::conceptAllocator Allocator = std::allocator<Value>>
semi_list(size_t, Value, Allocator = Allocator())
	-> semi_list<Value, Allocator>;
template<typename Iterator,
	typename Value = std::iter_value_t<Iterator>,
	momo::internal::conceptAllocator Allocator = std::allocator<Value>>
semi_list(Iterator, Iterator, Allocator = Allocator())
	-> semi_list<Value, Allocator>;
template<typename Value,
	momo::internal::conceptAllocator Allocator = std::allocator<Value>>
semi_list(std::initializer_list<Value>, Allocator = Allocator())
	-> semi_list<Value, Allocator>;
template<typename Value, typename Allocator>
semi_list(semi_list<Value, Allocator>, std::type_identity_t<Allocator>)
	-> semi_list<Value, Allocator>;

#if defined(__cpp_lib_containers_ranges)
template<std::ranges::input_range Range,
	typename Value = std::ranges::range_value_t<Range>,
	momo::internal::conceptAllocator Allocator = std::allocator<Value>>
semi_list(std::from_range_t, Range&&, Allocator = Allocator())
	-> semi_list<Value, Allocator>;
#endif

} // namespace momo::stdish
