/**********************************************************\

  momo/stdish/vector.h

  namespace momo::stdish:
    class vector
    class vector_intcap

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
	typedef TAllocator allocator_type;
	typedef TValue value_type;

	typedef size_t size_type;
	typedef ptrdiff_t difference_type;

	typedef typename Array::Iterator iterator;
	typedef typename Array::ConstIterator const_iterator;

	typedef typename iterator::Pointer pointer;
	typedef typename const_iterator::Pointer const_pointer;

	typedef typename iterator::Reference reference;
	typedef typename const_iterator::Reference const_reference;

	typedef std::reverse_iterator<iterator> reverse_iterator;
	typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

public:
	vector()
	{
	}

	explicit vector(const allocator_type& alloc)
		: mArray(MemManager(alloc))
	{
	}

	explicit vector(size_type count)
		: mArray(count)
	{
	}

	vector(size_type count, const value_type& value)
		: mArray(count, value)
	{
	}

	vector(size_type count, const value_type& value, const allocator_type& alloc)
		: mArray(count, value, MemManager(alloc))
	{
	}

	template<typename Iterator>
	vector(Iterator first, Iterator last)
		: mArray(first, last, MemManager())
	{
	}

	template<typename Iterator>
	vector(Iterator first, Iterator last, const allocator_type& alloc)
		: mArray(first, last, MemManager(alloc))
	{
	}

#ifdef MOMO_USE_INIT_LISTS
	vector(std::initializer_list<value_type> values)
		: mArray(values.begin(), values.end(), MemManager())
	{
	}

	vector(std::initializer_list<value_type> values, const allocator_type& alloc)
		: mArray(values.begin(), values.end(), MemManager(alloc))
	{
	}
#endif

	vector(vector&& right) MOMO_NOEXCEPT
		: mArray(std::move(right.mArray))
	{
	}

	vector(vector&& right, const allocator_type& alloc)
		: mArray(_create_array(std::move(right), alloc))
	{
		right.clear();
	}

	vector(const vector& right)
		: mArray(right.mArray)
	{
	}

	vector(const vector& right, const allocator_type& alloc)
		: mArray(right.begin(), right.end(), MemManager(alloc))
	{
	}

	~vector() MOMO_NOEXCEPT
	{
	}

	vector& operator=(vector&& right) MOMO_NOEXCEPT
	{
		mArray = std::move(right.mArray);
		return *this;
	}

	vector& operator=(const vector& right)
	{
		if (this != &right)
		{
			bool propagate = std::allocator_traits<allocator_type>
				::propagate_on_container_copy_assignment::value;
			allocator_type alloc = propagate ? right.get_allocator() : get_allocator();
			vector(right, alloc).swap(*this);
		}
		return *this;
	}

#ifdef MOMO_USE_INIT_LISTS
	vector& operator=(std::initializer_list<value_type> values)
	{
		assign(values);
		return *this;
	}
#endif

	void swap(vector& right) MOMO_NOEXCEPT
	{
		mArray.Swap(right.mArray);
	}

	friend void swap(vector& left, vector& right) MOMO_NOEXCEPT
	{
		left.swap(right);
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
		return reverse_iterator(end());
	}

	reverse_iterator rend() MOMO_NOEXCEPT
	{
		return reverse_iterator(begin());
	}

	const_reverse_iterator rend() const MOMO_NOEXCEPT
	{
		return reverse_iterator(begin());
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

	pointer data() MOMO_NOEXCEPT
	{
		return mArray.GetItems();
	}

	const_pointer data() const MOMO_NOEXCEPT
	{
		return mArray.GetItems();
	}

	allocator_type get_allocator() const //MOMO_NOEXCEPT
	{
		return mArray.GetMemManager().GetAllocator();
	}

	size_type max_size() const //MOMO_NOEXCEPT
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
		mArray.Add(index, std::move(value));
		return begin() + index;
	}

	iterator insert(const_iterator where, const value_type& value)
	{
		size_t index = where - begin();
		mArray.Add(index, value);
		return begin() + index;
	}

	iterator insert(const_iterator where, size_type count, const value_type& value)
	{
		size_t index = where - begin();
		mArray.Add(index, count, value);
		return begin() + index;
	}

	template<typename Iterator>
	iterator insert(const_iterator where, Iterator first, Iterator last)
	{
		size_t index = where - begin();
		mArray.Add(index, first, last);
		return begin() + index;
	}

#ifdef MOMO_USE_INIT_LISTS
	iterator insert(const_iterator where, std::initializer_list<value_type> values)
	{
		return insert(where, values.begin(), values.end());
	}
#endif

#ifdef MOMO_USE_VARIADIC_TEMPLATES
	template<typename... Args>
	void emplace_back(Args&&... args)
	{
		auto itemCreator = [this, &args...] (void* pitem)
			{ new(pitem) value_type(std::forward<Args>(args)...); };
		mArray.AddBackEmpl(itemCreator);
	}

	template<typename... Args>
	iterator emplace(const_iterator where, Args&&... args)
	{
		auto itemCreator = [this, &args...] (void* pitem)
			{ new(pitem) value_type(std::forward<Args>(args)...); };
		size_t index = where - begin();
		mArray.AddEmpl(index, itemCreator);
		return begin() + index;
	}
#endif

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
		*this = vector(count, value, get_allocator());
	}

	template<typename Iterator>
	void assign(Iterator first, Iterator last)
	{
		*this = vector(first, last, get_allocator());
	}

#ifdef MOMO_USE_INIT_LISTS
	void assign(std::initializer_list<value_type> values)
	{
		assign(values.begin(), values.end());
	}
#endif

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
		return _create_array(right, MemManager(alloc),
			std::integral_constant<bool, Array::ItemTraits::isNothrowMoveConstructible>());
	}

	static Array _create_array(vector& right, MemManager&& memManager,
		std::true_type /*isNothrowMoveConstructible*/)
	{
		return Array(std::make_move_iterator(right.begin()),
			std::make_move_iterator(right.end()), std::move(memManager));
	}

	static Array _create_array(vector& right, MemManager&& memManager,
		std::false_type /*isNothrowMoveConstructible*/)
	{
		return Array(right.begin(), right.end(), std::move(memManager));
	}

private:
	Array mArray;
};

#ifdef MOMO_USE_TYPE_ALIASES
template<size_t tInternalCapacity, typename TValue,
	typename TAllocator = std::allocator<TValue>>
using vector_intcap = vector<TValue, TAllocator,
	Array<TValue, MemManagerStd<TAllocator>, ArraySettings<tInternalCapacity>>>;
#endif

} // namespace stdish

} // namespace momo
