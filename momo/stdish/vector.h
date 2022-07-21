/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/stdish/vector.h

  namespace momo::stdish:
    class vector
    class vector_intcap

\**********************************************************/

#pragma once

#include "../Array.h"

namespace momo::stdish
{

/*!
	\brief
	`momo::stdish::vector` is similar to `std::vector`.

	\details
	It is allowed to pass to functions `push_back`, `insert`, `emplace_back`
	and `emplace` references to items within the container.
	But in case of the function `insert`, receiving pair of iterators, it's
	not allowed to pass iterators pointing to the items within the container.
*/

template<typename TValue,
	typename TAllocator = std::allocator<TValue>,
	typename TArray = Array<TValue, MemManagerStd<TAllocator>>>
class vector
{
private:
	typedef TArray Array;
	typedef typename Array::MemManager MemManager;

	typedef momo::internal::UIntMath<> SMath;

public:
	typedef TValue value_type;
	typedef TAllocator allocator_type;

	typedef Array nested_container_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef typename Array::Iterator iterator;
	typedef typename Array::ConstIterator const_iterator;

	typedef value_type& reference;
	typedef const value_type& const_reference;

	typedef value_type* pointer;
	typedef const value_type* const_pointer;
	//typedef typename std::allocator_traits<allocator_type>::pointer pointer;
	//typedef typename std::allocator_traits<allocator_type>::const_pointer const_pointer;

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
	vector() noexcept(noexcept(Array()))
	{
	}

	explicit vector(const allocator_type& alloc) noexcept
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

	template<momo::internal::conceptInputIterator Iterator>
	vector(Iterator first, Iterator last, const allocator_type& alloc = allocator_type())
		: mArray(first, last, MemManager(alloc))
	{
	}

	vector(std::initializer_list<value_type> values, const allocator_type& alloc = allocator_type())
		: mArray(values, MemManager(alloc))
	{
	}

	vector(vector&& right) noexcept
		: mArray(std::move(right.mArray))
	{
	}

	vector(vector&& right, const std::type_identity_t<allocator_type>& alloc)
		noexcept(std::allocator_traits<allocator_type>::is_always_equal::value)
		: mArray(pvCreateArray(std::move(right), alloc))
	{
	}

	vector(const vector& right)
		: mArray(right.mArray)
	{
	}

	vector(const vector& right, const std::type_identity_t<allocator_type>& alloc)
		: mArray(right.mArray, MemManager(alloc))
	{
	}

	~vector() noexcept = default;

	vector& operator=(vector&& right)
		noexcept(std::allocator_traits<allocator_type>::is_always_equal::value ||
			std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>::is_always_equal::value ||
				std::allocator_traits<allocator_type>::propagate_on_container_move_assignment::value;
			allocator_type alloc = (propagate ? &right : this)->get_allocator();
			mArray = pvCreateArray(std::move(right), alloc);
		}
		return *this;
	}

	vector& operator=(const vector& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>::is_always_equal::value ||
				std::allocator_traits<allocator_type>::propagate_on_container_copy_assignment::value;
			allocator_type alloc = (propagate ? &right : this)->get_allocator();
			mArray = Array(right.mArray, MemManager(alloc));
		}
		return *this;
	}

	vector& operator=(std::initializer_list<value_type> values)
	{
		assign(values);
		return *this;
	}

	void swap(vector& right) noexcept
	{
		MOMO_ASSERT(std::allocator_traits<allocator_type>::propagate_on_container_swap::value
			|| get_allocator() == right.get_allocator());
		mArray.Swap(right.mArray);
	}

	friend void swap(vector& left, vector& right) noexcept
	{
		left.swap(right);
	}

	const nested_container_type& get_nested_container() const noexcept
	{
		return mArray;
	}

	nested_container_type& get_nested_container() noexcept
	{
		return mArray;
	}

	const_iterator begin() const noexcept
	{
		return mArray.GetBegin();
	}

	iterator begin() noexcept
	{
		return mArray.GetBegin();
	}

	const_iterator end() const noexcept
	{
		return mArray.GetEnd();
	}

	iterator end() noexcept
	{
		return mArray.GetEnd();
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

	value_type* data() noexcept
		requires requires { { std::declval<Array&>().GetItems() } noexcept
			-> std::same_as<value_type*>; }
	{
		return mArray.GetItems();
	}

	const value_type* data() const noexcept
		requires requires { { std::declval<const Array&>().GetItems() } noexcept
			-> std::same_as<const value_type*>; }
	{
		return mArray.GetItems();
	}

	allocator_type get_allocator() const noexcept
	{
		return allocator_type(mArray.GetMemManager().GetByteAllocator());
	}

	size_type max_size() const noexcept
	{
		return std::allocator_traits<allocator_type>::max_size(get_allocator());
	}

	size_type size() const noexcept
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

	[[nodiscard]] bool empty() const noexcept
	{
		return mArray.IsEmpty();
	}

	void clear() noexcept
	{
		mArray.Clear();
	}

	size_type capacity() const noexcept
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
		return data()[index];
	}

	reference at(size_type index)
	{
		if (index >= size())
			throw std::out_of_range("invalid vector subscript");
		return data()[index];
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
		size_t index = SMath::Dist(cbegin(), where);
		mArray.Insert(index, std::move(value));
		return SMath::Next(begin(), index);
	}

	iterator insert(const_iterator where, const value_type& value)
	{
		size_t index = SMath::Dist(cbegin(), where);
		mArray.Insert(index, value);
		return SMath::Next(begin(), index);
	}

	iterator insert(const_iterator where, size_type count, const value_type& value)
	{
		size_t index = SMath::Dist(cbegin(), where);
		mArray.Insert(index, count, value);
		return SMath::Next(begin(), index);
	}

	template<momo::internal::conceptInputIterator Iterator>
	iterator insert(const_iterator where, Iterator first, Iterator last)
	{
		size_t index = SMath::Dist(cbegin(), where);
		mArray.Insert(index, first, last);
		return SMath::Next(begin(), index);
	}

	iterator insert(const_iterator where, std::initializer_list<value_type> values)
	{
		size_t index = SMath::Dist(cbegin(), where);
		mArray.Insert(index, values);
		return SMath::Next(begin(), index);
	}

	template<typename... ValueArgs>
	reference emplace_back(ValueArgs&&... valueArgs)
	{
		mArray.AddBackVar(std::forward<ValueArgs>(valueArgs)...);
		return back();
	}

	template<typename... ValueArgs>
	iterator emplace(const_iterator where, ValueArgs&&... valueArgs)
	{
		size_t index = SMath::Dist(cbegin(), where);
		mArray.InsertVar(index, std::forward<ValueArgs>(valueArgs)...);
		return SMath::Next(begin(), index);
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
		size_t index = SMath::Dist(cbegin(), first);
		mArray.Remove(index, SMath::Dist(first, last));
		return SMath::Next(begin(), index);
	}

	template<typename ValueArg>
	friend size_type erase(vector& cont, const ValueArg& valueArg)
	{
		auto pred = [&valueArg] (const value_type& value)
			{ return value == valueArg; };
		return cont.mArray.Remove(pred);
	}

	template<typename Predicate>
	requires std::predicate<const Predicate&, const_reference>
	friend size_type erase_if(vector& cont, const Predicate& pred)
	{
		return cont.mArray.Remove(pred);
	}

	void assign(size_type count, const value_type& value)
	{
		mArray = Array(count, value, MemManager(get_allocator()));
	}

	template<momo::internal::conceptInputIterator Iterator>
	void assign(Iterator first, Iterator last)
	{
		mArray = Array(first, last, MemManager(get_allocator()));
	}

	void assign(std::initializer_list<value_type> values)
	{
		assign(values.begin(), values.end());
	}

	bool operator==(const vector& right) const
	{
		return mArray.IsEqual(right.mArray);
	}

	auto operator<=>(const vector& right) const
	{
		auto comp = [] (const value_type& value1, const value_type& value2)
			{ return std::tie(value1) <=> std::tie(value2); };
		return std::lexicographical_compare_three_way(begin(), end(),
			right.begin(), right.end(), comp);
	}

private:
	static Array pvCreateArray(vector&& right, const allocator_type& alloc)
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

template<typename Iterator,
	typename Allocator = std::allocator<std::iter_value_t<Iterator>>>
requires momo::internal::conceptAllocator<Allocator>
vector(Iterator, Iterator, Allocator = Allocator())
	-> vector<std::iter_value_t<Iterator>, Allocator>;

/*!
	\brief
	`momo::stdish::vector_intcap` is vector with internal capacity. This vector
	doesn't need dynamic memory while its size is not greater than
	user-defined constant.

	\copydetails momo::stdish::vector
*/

template<size_t tInternalCapacity, typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector_intcap = vector<TValue, TAllocator,
	ArrayIntCap<tInternalCapacity, TValue, MemManagerStd<TAllocator>>>;

} // namespace momo::stdish
