/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

  momo/stdish/vector.h

  namespace momo::stdish:
    class vector
    class vector_intcap

  This classes are similar to `std::vector`.
  `stdish::vector_intcap` is vector with internal capacity.
  This vector doesn't need dynamic memory while its size is not greater
  than user-defined constant.

  It is allowed to pass to functions `insert` and `emplace` references
  to items within the container.
  But in case of the function `insert`, receiving pair of iterators, it's
  not allowed to pass iterators pointing to the items within the container. 

\**********************************************************/

#pragma once

#include "../Array.h"

namespace momo
{

namespace stdish
{

template<typename TValue,
	typename TAllocator = std::allocator<TValue>,
	typename TArray = Array<TValue, MemManagerStd<TAllocator>>>
class vector
{
private:
	typedef TArray Array;
	typedef typename Array::MemManager MemManager;

public:
	typedef TValue value_type;
	typedef TAllocator allocator_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef typename Array::Iterator iterator;
	typedef typename Array::ConstIterator const_iterator;

	typedef typename iterator::Reference reference;
	typedef typename const_iterator::Reference const_reference;

	typedef typename iterator::Pointer pointer;
	typedef typename const_iterator::Pointer const_pointer;
	//typedef typename std::allocator_traits<allocator_type>::pointer pointer;
	//typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
	vector() MOMO_NOEXCEPT_IF(noexcept(Array()))
	{
	}

	explicit vector(const allocator_type& alloc) MOMO_NOEXCEPT
		: mArray(MemManager(alloc))
	{
	}

	explicit vector(size_type count, const allocator_type& alloc = allocator_type())
		: mArray(count, MemManager(alloc))
	{
	}

	vector(size_type count, const value_type& value, const allocator_type& alloc = allocator_type())
		: mArray(count, value, MemManager(alloc))
	{
	}

	template<typename Iterator,
		typename = typename std::iterator_traits<Iterator>::iterator_category>
	vector(Iterator first, Iterator last, const allocator_type& alloc = allocator_type())
		: mArray(first, last, MemManager(alloc))
	{
	}

	vector(std::initializer_list<value_type> values, const allocator_type& alloc = allocator_type())
		: mArray(values, MemManager(alloc))
	{
	}

	vector(vector&& right) MOMO_NOEXCEPT
		: mArray(std::move(right.mArray))
	{
	}

	vector(vector&& right, const allocator_type& alloc)
		MOMO_NOEXCEPT_IF((std::is_same<allocator_type, std::allocator<value_type>>::value))
		: mArray(_create_array(std::move(right), alloc))
	{
	}

	vector(const vector& right)
		: mArray(right.mArray)
	{
	}

	vector(const vector& right, const allocator_type& alloc)
		: mArray(right.mArray, MemManager(alloc))
	{
	}

	~vector() MOMO_NOEXCEPT
	{
	}

	vector& operator=(vector&& right)
		MOMO_NOEXCEPT_IF((std::is_same<allocator_type, std::allocator<value_type>>::value) ||
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_move_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			mArray = _create_array(std::move(right), alloc);
		}
		return *this;
	}

	vector& operator=(const vector& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_copy_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			mArray = Array(right.mArray, MemManager(alloc));
		}
		return *this;
	}

	vector& operator=(std::initializer_list<value_type> values)
	{
		assign(values);
		return *this;
	}

	void swap(vector& right) MOMO_NOEXCEPT
	{
		MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
			|| get_allocator() == right.get_allocator());
		mArray.Swap(right.mArray);
	}

	iterator begin() MOMO_NOEXCEPT
	{
		return mArray.GetBegin();
	}

	const_iterator begin() const MOMO_NOEXCEPT
	{
		return mArray.GetBegin();
	}

	iterator end() MOMO_NOEXCEPT
	{
		return mArray.GetEnd();
	}

	const_iterator end() const MOMO_NOEXCEPT
	{
		return mArray.GetEnd();
	}

	reverse_iterator rbegin() MOMO_NOEXCEPT
	{
		return reverse_iterator(end());
	}

	const_reverse_iterator rbegin() const MOMO_NOEXCEPT
	{
		return const_reverse_iterator(end());
	}

	reverse_iterator rend() MOMO_NOEXCEPT
	{
		return reverse_iterator(begin());
	}

	const_reverse_iterator rend() const MOMO_NOEXCEPT
	{
		return const_reverse_iterator(begin());
	}

	const_iterator cbegin() const MOMO_NOEXCEPT
	{
		return begin();
	}

	const_iterator cend() const MOMO_NOEXCEPT
	{
		return end();
	}

	const_reverse_iterator crbegin() const MOMO_NOEXCEPT
	{
		return rbegin();
	}

	const_reverse_iterator crend() const MOMO_NOEXCEPT
	{
		return rend();
	}

	MOMO_FRIENDS_SWAP_BEGIN_END_STD(vector)

	value_type* data() MOMO_NOEXCEPT
	{
		return mArray.GetItems();
	}

	const value_type* data() const MOMO_NOEXCEPT
	{
		return mArray.GetItems();
	}

	allocator_type get_allocator() const MOMO_NOEXCEPT
	{
		return mArray.GetMemManager().GetAllocator();
	}

	size_type max_size() const MOMO_NOEXCEPT
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
		//return std::minmax(Array::maxCapacity, get_allocator().max_size()).second;
	}

	size_type size() const MOMO_NOEXCEPT
	{
		return mArray.GetCount();
	}

	void resize(size_type size)
	{
		mArray.SetCount(size);
	}

	void resize(size_type size, const value_type& value)
	{
		mArray.SetCount(size, value);
	}

	bool empty() const MOMO_NOEXCEPT
	{
		return mArray.IsEmpty();
	}

	void clear() MOMO_NOEXCEPT
	{
		mArray.Clear();
	}

	size_type capacity() const MOMO_NOEXCEPT
	{
		return mArray.GetCapacity();
	}

	void reserve(size_type count)
	{
		mArray.Reserve(count);
	}

	void shrink_to_fit()
	{
		mArray.Shrink();
	}

	const_reference operator[](size_type index) const
	{
		return mArray[index];
	}

	reference operator[](size_type index)
	{
		return mArray[index];
	}

	const_reference at(size_type index) const
	{
		if (index >= size())
			throw std::out_of_range("invalid vector subscript");
		return mArray[index];
	}

	reference at(size_type index)
	{
		if (index >= size())
			throw std::out_of_range("invalid vector subscript");
		return mArray[index];
	}

	reference front()
	{
		return mArray[0];
	}

	const_reference front() const
	{
		return mArray[0];
	}

	reference back()
	{
		return mArray.GetBackItem();
	}

	const_reference back() const
	{
		return mArray.GetBackItem();
	}

	void push_back(value_type&& value)
	{
		mArray.AddBack(std::move(value));
	}

	void push_back(const value_type& value)
	{
		mArray.AddBack(value);
	}

	iterator insert(const_iterator where, value_type&& value)
	{
		size_t index = where - begin();
		mArray.Insert(index, std::move(value));
		return begin() + index;
	}

	iterator insert(const_iterator where, const value_type& value)
	{
		size_t index = where - begin();
		mArray.Insert(index, value);
		return begin() + index;
	}

	iterator insert(const_iterator where, size_type count, const value_type& value)
	{
		size_t index = where - begin();
		mArray.Insert(index, count, value);
		return begin() + index;
	}

	template<typename Iterator,
		typename = typename std::iterator_traits<Iterator>::iterator_category>
	iterator insert(const_iterator where, Iterator first, Iterator last)
	{
		size_t index = where - begin();
		mArray.Insert(index, first, last);
		return begin() + index;
	}

	iterator insert(const_iterator where, std::initializer_list<value_type> values)
	{
		size_t index = where - begin();
		mArray.Insert(index, values);
		return begin() + index;
	}

	template<typename... ValueArgs>
	reference emplace_back(ValueArgs&&... valueArgs)
	{
		mArray.AddBackVar(std::forward<ValueArgs>(valueArgs)...);
		return back();	// C++17
	}

	template<typename... ValueArgs>
	iterator emplace(const_iterator where, ValueArgs&&... valueArgs)
	{
		size_t index = where - begin();
		mArray.InsertVar(index, std::forward<ValueArgs>(valueArgs)...);
		return begin() + index;
	}

	void pop_back()
	{
		mArray.RemoveBack();
	}

	iterator erase(const_iterator where)
	{
		return erase(where, where + 1);
	}

	iterator erase(const_iterator first, const_iterator last)
	{
		size_t index = first - begin();
		mArray.Remove(index, last - first);
		return begin() + index;
	}

	void assign(size_type count, const value_type& value)
	{
		clear();	//?
		insert(end(), count, value);
	}

	template<typename Iterator,
		typename = typename std::iterator_traits<Iterator>::iterator_category>
	void assign(Iterator first, Iterator last)
	{
		clear();	//?
		insert(end(), first, last);
	}

	void assign(std::initializer_list<value_type> values)
	{
		assign(values.begin(), values.end());
	}

	bool operator==(const vector& right) const
	{
		return size() == right.size() && std::equal(begin(), end(), right.begin());
	}

	bool operator!=(const vector& right) const
	{
		return !(*this == right);
	}

	bool operator<(const vector& right) const
	{
		return std::lexicographical_compare(begin(), end(), right.begin(), right.end());
	}

	bool operator>(const vector& right) const
	{
		return right < *this;
	}

	bool operator<=(const vector& right) const
	{
		return !(right < *this);
	}

	bool operator>=(const vector& right) const
	{
		return right <= *this;
	}

private:
	static Array _create_array(vector&& right, const allocator_type& alloc)
	{
		if (right.get_allocator() == alloc)
			return std::move(right.mArray);
		Array array(std::make_move_iterator(right.begin()),
			std::make_move_iterator(right.end()), MemManager(alloc));
		right.clear();
		return array;
	}

private:
	Array mArray;
};

template<size_t tInternalCapacity, typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector_intcap = vector<TValue, TAllocator,
	ArrayIntCap<tInternalCapacity, TValue, MemManagerStd<TAllocator>>>;

} // namespace stdish

} // namespace momo
