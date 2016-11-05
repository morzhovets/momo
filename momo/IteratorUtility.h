/**********************************************************\

  This file is distributed under the MIT License.
  See accompanying file LICENSE for details.

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
	} \
	explicit operator bool() const MOMO_NOEXCEPT \
	{ \
		return !!*this; \
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

		explicit IteratorVersion(const size_t* version) MOMO_NOEXCEPT
			: mContainerVersion(version),
			mVersion(*version)
		{
		}

		bool Check() const MOMO_NOEXCEPT
		{
			return *mContainerVersion == mVersion;
		}

		bool Check(const size_t* version) const MOMO_NOEXCEPT
		{
			MOMO_ASSERT(version != nullptr);
			return mContainerVersion == version && mVersion == *version;
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

		explicit IteratorVersion(const size_t* /*version*/) MOMO_NOEXCEPT
		{
		}

		bool Check() const MOMO_NOEXCEPT
		{
			return true;
		}

		bool Check(const size_t* /*version*/) const MOMO_NOEXCEPT
		{
			return true;
		}
	};

	template<typename TReference>
	class IteratorPointer
	{
	public:
		typedef TReference Reference;

		typedef IteratorPointer<typename Reference::ConstReference> ConstPointer;

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

		const Reference* operator->() const MOMO_NOEXCEPT
		{
			return &mReference;
		}

		Reference operator*() const MOMO_NOEXCEPT
		{
			return mReference;
		}

	private:
		Reference mReference;
	};

	template<typename TBaseIterator, typename TReference>
	class HashDerivedIterator
	{
	public:
		typedef TBaseIterator BaseIterator;
		typedef TReference Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef HashDerivedIterator<typename BaseIterator::ConstIterator,
			typename Reference::ConstReference> ConstIterator;

	public:
		HashDerivedIterator() MOMO_NOEXCEPT
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIterator(mBaseIterator);
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
			return mBaseIterator == iter.frGetBaseIterator();
		}

		MOMO_MORE_HASH_ITERATOR_OPERATORS(HashDerivedIterator)

	public:
		explicit HashDerivedIterator(BaseIterator iter) MOMO_NOEXCEPT
			: mBaseIterator(iter)
		{
		}

		BaseIterator frGetBaseIterator() const MOMO_NOEXCEPT
		{
			return mBaseIterator;
		}

	private:
		BaseIterator mBaseIterator;
	};

	template<typename TBaseIterator, typename TReference>
	class TreeDerivedIterator
	{
	public:
		typedef TBaseIterator BaseIterator;
		typedef TReference Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef TreeDerivedIterator<typename BaseIterator::ConstIterator,
			typename Reference::ConstReference> ConstIterator;

	public:
		TreeDerivedIterator() MOMO_NOEXCEPT
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIterator(mBaseIterator);
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
			return mBaseIterator == iter.frGetBaseIterator();
		}

		MOMO_MORE_TREE_ITERATOR_OPERATORS(TreeDerivedIterator)

	public:
		explicit TreeDerivedIterator(BaseIterator iter) MOMO_NOEXCEPT
			: mBaseIterator(iter)
		{
		}

		BaseIterator frGetBaseIterator() const MOMO_NOEXCEPT
		{
			return mBaseIterator;
		}

	private:
		BaseIterator mBaseIterator;
	};

	template<typename TBucketIterator, typename TBaseBucketBounds>
	class HashDerivedBucketBounds
	{
	public:
		typedef TBucketIterator BucketIterator;
		typedef TBaseBucketBounds BaseBucketBounds;

		typedef BucketIterator Iterator;

		typedef HashDerivedBucketBounds<typename BucketIterator::ConstIterator,
			typename BaseBucketBounds::ConstBounds> ConstBounds;

	public:
		HashDerivedBucketBounds() MOMO_NOEXCEPT
		{
		}

		explicit HashDerivedBucketBounds(BaseBucketBounds bounds) MOMO_NOEXCEPT
			: mBaseBucketBounds(bounds)
		{
		}

		operator ConstBounds() const MOMO_NOEXCEPT
		{
			return ConstBounds(mBaseBucketBounds);
		}

		BucketIterator GetBegin() const MOMO_NOEXCEPT
		{
			return BucketIterator(mBaseBucketBounds.GetBegin());
		}

		BucketIterator GetEnd() const MOMO_NOEXCEPT
		{
			return BucketIterator(mBaseBucketBounds.GetEnd());
		}

		MOMO_FRIENDS_BEGIN_END(const HashDerivedBucketBounds&, BucketIterator)

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mBaseBucketBounds.GetCount();
		}

	private:
		BaseBucketBounds mBaseBucketBounds;
	};
}

} // namespace momo

namespace std
{
	template<typename BI, typename R>
	struct iterator_traits<momo::internal::HashDerivedIterator<BI, R>>
	{
		typedef forward_iterator_tag iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename momo::internal::HashDerivedIterator<BI, R>::Pointer pointer;
		typedef typename momo::internal::HashDerivedIterator<BI, R>::Reference reference;
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
