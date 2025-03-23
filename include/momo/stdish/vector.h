/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/stdish/vector.h

  namespace momo::stdish:
    class vector
    class vector_intcap

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_STDISH_VECTOR
#define MOMO_INCLUDE_GUARD_STDISH_VECTOR

#include "../Array.h"

#ifdef MOMO_HAS_CONTAINERS_RANGES
# include <ranges>
#endif

namespace momo
{

namespace stdish
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

#ifdef MOMO_HAS_CONTAINERS_RANGES
	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	vector(std::from_range_t, Range&& values, const allocator_type& alloc = allocator_type())
		: mArray(std::ranges::begin(values), std::ranges::end(values), MemManager(alloc))
	{
	}
#endif // MOMO_HAS_CONTAINERS_RANGES

	vector(vector&& right) noexcept
		: vector(std::move(right), right.get_allocator())
	{
	}

	vector(vector&& right, const momo::internal::Identity<allocator_type>& alloc)
		noexcept(std::is_empty<allocator_type>::value)
		: vector(alloc)
	{
		if (right.get_allocator() == alloc)
		{
			mArray.Swap(right.mArray);
		}
		else
		{
			pvAssign(std::make_move_iterator(right.begin()), std::make_move_iterator(right.end()));
			right.mArray.Clear(true);
		}
	}

	vector(const vector& right)
		: mArray(right.mArray)
	{
	}

	vector(const vector& right, const momo::internal::Identity<allocator_type>& alloc)
		: mArray(right.mArray, MemManager(alloc))
	{
	}

	~vector() = default;

	vector& operator=(vector&& right)
		noexcept(momo::internal::ContainerAssignerStd::isNothrowMoveAssignable<vector>)
	{
		return momo::internal::ContainerAssignerStd::Move(std::move(right), *this);
	}

	vector& operator=(const vector& right)
	{
		return momo::internal::ContainerAssignerStd::Copy(right, *this);
	}

	vector& operator=(std::initializer_list<value_type> values)
	{
		assign(values);
		return *this;
	}

	void swap(vector& right) noexcept
	{
		momo::internal::ContainerAssignerStd::Swap(*this, right);
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
	{
		return mArray.GetItems();
	}

	const value_type* data() const noexcept
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

	MOMO_NODISCARD bool empty() const noexcept
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

	template<typename Iterator,
		typename = typename std::iterator_traits<Iterator>::iterator_category>
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

#ifdef MOMO_HAS_CONTAINERS_RANGES
	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	void append_range(Range&& values)
	{
		insert_range(cend(), std::forward<Range>(values));
	}

	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	iterator insert_range(const_iterator where, Range&& values)
	{
		size_t index = SMath::Dist(cbegin(), where);
		mArray.Insert(index, std::ranges::begin(values), std::ranges::end(values));
		return SMath::Next(begin(), index);
	}
#endif // MOMO_HAS_CONTAINERS_RANGES

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
		auto valueFilter = [&valueArg] (const value_type& value)
			{ return value == valueArg; };
		return cont.mArray.Remove(valueFilter);
	}

	template<typename ValueFilter>
	friend size_type erase_if(vector& cont, const ValueFilter& valueFilter)
	{
		return cont.mArray.Remove(valueFilter);
	}

	void assign(size_type count, const value_type& value)
	{
		mArray = Array(count, value, MemManager(get_allocator()));
	}

	template<typename Iterator,
		typename = typename std::iterator_traits<Iterator>::iterator_category>
	void assign(Iterator first, Iterator last)
	{
		pvAssign(first, last);
	}

	void assign(std::initializer_list<value_type> values)
	{
		pvAssign(values.begin(), values.end());
	}

#ifdef MOMO_HAS_CONTAINERS_RANGES
	template<std::ranges::input_range Range>
	requires std::convertible_to<std::ranges::range_reference_t<Range>, value_type>
	void assign_range(Range&& values)
	{
		pvAssign(std::ranges::begin(values), std::ranges::end(values));
	}
#endif // MOMO_HAS_CONTAINERS_RANGES

	friend bool operator==(const vector& left, const vector& right)
	{
		return left.mArray.IsEqual(right.mArray);
	}

#ifdef MOMO_HAS_THREE_WAY_COMPARISON
	friend auto operator<=>(const vector& left, const vector& right)
		requires requires (const_reference ref) { std::tie(ref) <=> std::tie(ref); }
	{
		auto valueThreeComp = [] (const value_type& value1, const value_type& value2)
			{ return std::tie(value1) <=> std::tie(value2); };
		return std::lexicographical_compare_three_way(left.begin(), left.end(),
			right.begin(), right.end(), valueThreeComp);
	}
#else
	friend bool operator<(const vector& left, const vector& right)
	{
		return std::lexicographical_compare(left.begin(), left.end(), right.begin(), right.end());
	}
#endif

	MOMO_MORE_COMPARISON_OPERATORS(const vector&)

private:
	template<typename Iterator, typename Sentinel>
	void pvAssign(Iterator begin, Sentinel end)
	{
		mArray = Array(std::move(begin), std::move(end), MemManager(get_allocator()));
	}

private:
	Array mArray;
};

#ifdef MOMO_HAS_DEDUCTION_GUIDES

namespace internal
{
	template<typename Allocator,
		typename = decltype(std::declval<Allocator&>().allocate(size_t{}))>
	class vector_checker
	{
	};
}

template<typename Iterator,
	typename Value = typename std::iterator_traits<Iterator>::value_type,
	typename Allocator = std::allocator<Value>,
	typename = internal::vector_checker<Allocator>>
vector(Iterator, Iterator, Allocator = Allocator())
	-> vector<Value, Allocator>;

#ifdef MOMO_HAS_CONTAINERS_RANGES
template<std::ranges::input_range Range,
	typename Value = std::ranges::range_value_t<Range>,
	typename Allocator = std::allocator<Value>,
	typename = internal::vector_checker<Allocator>>
vector(std::from_range_t, Range&&, Allocator = Allocator())
	-> vector<Value, Allocator>;
#endif // MOMO_HAS_CONTAINERS_RANGES

#endif // MOMO_HAS_DEDUCTION_GUIDES

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

} // namespace stdish

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_STDISH_VECTOR
