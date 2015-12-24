/**********************************************************\

  momo/IteratorUtility.h

\**********************************************************/

#pragma once

#include "Utility.h"

#define MOMO_MORE_HASH_ITERATOR_OPERATORS(Iterator) \
	Iterator operator++(int) \
	{ \
		Iterator tempIter = *this; \
		++*this; \
		return tempIter; \
	} \
	Reference operator*() const \
	{ \
		return *operator->(); \
	} \
	bool operator!=(ConstIterator iter) const MOMO_NOEXCEPT \
	{ \
		return !(*this == iter); \
	} \
	bool operator!() const MOMO_NOEXCEPT \
	{ \
		return *this == ConstIterator(); \
	}

#define MOMO_MORE_TREE_ITERATOR_OPERATORS(Iterator) \
	Iterator operator++(int) \
	{ \
		Iterator tempIter = *this; \
		++*this; \
		return tempIter; \
	} \
	Iterator operator--(int) \
	{ \
		Iterator tempIter = *this; \
		--*this; \
		return tempIter; \
	} \
	Reference operator*() const \
	{ \
		return *operator->(); \
	} \
	bool operator!=(ConstIterator iter) const MOMO_NOEXCEPT \
	{ \
		return !(*this == iter); \
	}

namespace momo
{

namespace internal
{
	template<typename TIterator>
	class InsertResult
	{
	public:
		typedef TIterator Iterator;

	public:
		InsertResult(Iterator iter, bool inserted) MOMO_NOEXCEPT
			: iterator(iter),
			inserted(inserted)
		{
		}

	public:
		Iterator iterator;
		bool inserted;
	};

	template<bool tCheckVersion>
	class IteratorVersion;

	template<>
	class IteratorVersion<true>
	{
	public:
		static const bool checkVersion = true;

	public:
		IteratorVersion() MOMO_NOEXCEPT
			: mContainerVersion(nullptr),
			mVersion(0)
		{
		}

		explicit IteratorVersion(const size_t& version) MOMO_NOEXCEPT
			: mContainerVersion(&version),
			mVersion(version)
		{
		}

		bool Check() const MOMO_NOEXCEPT
		{
			return *mContainerVersion == mVersion;
		}

		bool Check(const size_t& version) const MOMO_NOEXCEPT
		{
			return mContainerVersion == &version && mVersion == version;
		}

	private:
		const size_t* mContainerVersion;
		size_t mVersion;
	};

	template<>
	class IteratorVersion<false>
	{
	public:
		static const bool checkVersion = false;

	public:
		IteratorVersion() MOMO_NOEXCEPT
		{
		}

		explicit IteratorVersion(const size_t& /*version*/) MOMO_NOEXCEPT
		{
		}

		bool Check() const MOMO_NOEXCEPT
		{
			return true;
		}

		bool Check(const size_t& /*version*/) const MOMO_NOEXCEPT
		{
			return true;
		}
	};

	template<typename TReference,
		typename TConstReference = typename TReference::ConstReference>
	class IteratorPointer
	{
	public:
		typedef TReference Reference;
		typedef TConstReference ConstReference;

		typedef IteratorPointer<ConstReference, ConstReference> ConstPointer;

		typedef const typename std::remove_reference<Reference>::type* RefAddress;

	public:
		//IteratorPointer() MOMO_NOEXCEPT
		//{
		//}

		explicit IteratorPointer(Reference ref) MOMO_NOEXCEPT
			: mReference(ref)
		{
		}

		operator ConstPointer() const MOMO_NOEXCEPT
		{
			return ConstPointer(mReference);
		}

		RefAddress operator->() const MOMO_NOEXCEPT
		{
			return std::addressof(mReference);
		}

		Reference operator*() const MOMO_NOEXCEPT
		{
			return mReference;
		}

	private:
		Reference mReference;
	};

	template<typename TBaseIterator, typename TReference,
		typename TConstBaseIterator = typename TBaseIterator::ConstIterator,
		typename TConstReference = typename TReference::ConstReference>
	class HashDerivedIterator
	{
	public:
		typedef TBaseIterator BaseIterator;
		typedef TReference Reference;
		typedef TConstBaseIterator ConstBaseIterator;
		typedef TConstReference ConstReference;

		typedef IteratorPointer<Reference, ConstReference> Pointer;

		typedef HashDerivedIterator<ConstBaseIterator, ConstReference,
			ConstBaseIterator, ConstReference> ConstIterator;

	public:
		HashDerivedIterator() MOMO_NOEXCEPT
		{
		}

		explicit HashDerivedIterator(BaseIterator iter) MOMO_NOEXCEPT
			: mBaseIterator(iter)
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIterator(mBaseIterator);
		}

		BaseIterator GetBaseIterator() const MOMO_NOEXCEPT
		{
			return mBaseIterator;
		}

		HashDerivedIterator& operator++()
		{
			++mBaseIterator;
			return *this;
		}

		Pointer operator->() const
		{
			return Pointer(Reference(*mBaseIterator));
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mBaseIterator == iter.GetBaseIterator();
		}

		MOMO_MORE_HASH_ITERATOR_OPERATORS(HashDerivedIterator)

	private:
		BaseIterator mBaseIterator;
	};

	template<typename TBaseIterator, typename TReference>
	class TreeDerivedIterator
	{
	public:
		typedef TBaseIterator BaseIterator;
		typedef TReference Reference;
		typedef typename BaseIterator::ConstIterator ConstBaseIterator;
		typedef typename Reference::ConstReference ConstReference;

		typedef IteratorPointer<Reference, ConstReference> Pointer;

		typedef TreeDerivedIterator<ConstBaseIterator, ConstReference> ConstIterator;

	public:
		TreeDerivedIterator() MOMO_NOEXCEPT
		{
		}

		explicit TreeDerivedIterator(BaseIterator iter) MOMO_NOEXCEPT
			: mBaseIterator(iter)
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIterator(mBaseIterator);
		}

		BaseIterator GetBaseIterator() const MOMO_NOEXCEPT
		{
			return mBaseIterator;
		}

		TreeDerivedIterator& operator++()
		{
			++mBaseIterator;
			return *this;
		}

		TreeDerivedIterator& operator--()
		{
			--mBaseIterator;
			return *this;
		}

		Pointer operator->() const
		{
			return Pointer(Reference(*mBaseIterator));
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mBaseIterator == iter.GetBaseIterator();
		}

		MOMO_MORE_TREE_ITERATOR_OPERATORS(TreeDerivedIterator)

	private:
		BaseIterator mBaseIterator;
	};
}

} // namespace momo

namespace std
{
	template<typename BI, typename R, typename CBI, typename CR>
	struct iterator_traits<momo::internal::HashDerivedIterator<BI, R, CBI, CR>>
	{
		typedef forward_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename momo::internal::HashDerivedIterator<BI, R, CBI, CR>::Pointer pointer;
		typedef typename momo::internal::HashDerivedIterator<BI, R, CBI, CR>::Reference reference;
		typedef reference value_type;	//?
	};

	template<typename BI, typename R>
	struct iterator_traits<momo::internal::TreeDerivedIterator<BI, R>>
	{
		typedef bidirectional_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename momo::internal::TreeDerivedIterator<BI, R>::Pointer pointer;
		typedef typename momo::internal::TreeDerivedIterator<BI, R>::Reference reference;
		typedef reference value_type;	//?
	};
} // namespace std
