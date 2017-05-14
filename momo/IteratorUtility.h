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
			MOMO_ASSERT(mContainerVersion != nullptr);
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
	protected:
		typedef TBaseIterator BaseIterator;

	public:
		typedef TReference Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef HashDerivedIterator<typename BaseIterator::ConstIterator,
			typename Reference::ConstReference> ConstIterator;

	private:
		struct ReferenceProxy : public Reference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Reference)
		};

		struct ConstIteratorProxy : public ConstIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
			MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBaseIterator,
				typename ConstIterator::BaseIterator)
		};

	public:
		HashDerivedIterator() MOMO_NOEXCEPT
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIteratorProxy(mBaseIterator);
		}

		HashDerivedIterator& operator++()
		{
			++mBaseIterator;
			return *this;
		}

		Pointer operator->() const
		{
			return Pointer(ReferenceProxy(*mBaseIterator));
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mBaseIterator == ConstIteratorProxy::GetBaseIterator(iter);
		}

		MOMO_MORE_HASH_ITERATOR_OPERATORS(HashDerivedIterator)

	protected:
		explicit HashDerivedIterator(BaseIterator iter) MOMO_NOEXCEPT
			: mBaseIterator(iter)
		{
		}

		BaseIterator ptGetBaseIterator() const MOMO_NOEXCEPT
		{
			return mBaseIterator;
		}

	private:
		BaseIterator mBaseIterator;
	};

	template<typename TBaseIterator, typename TReference>
	class TreeDerivedIterator
	{
	protected:
		typedef TBaseIterator BaseIterator;

	public:
		typedef TReference Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef TreeDerivedIterator<typename BaseIterator::ConstIterator,
			typename Reference::ConstReference> ConstIterator;

	private:
		struct ReferenceProxy : public Reference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Reference)
		};

		struct ConstIteratorProxy : public ConstIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
			MOMO_DECLARE_PROXY_FUNCTION(ConstIterator, GetBaseIterator,
				typename ConstIterator::BaseIterator)
		};

	public:
		TreeDerivedIterator() MOMO_NOEXCEPT
		{
		}

		operator ConstIterator() const MOMO_NOEXCEPT
		{
			return ConstIteratorProxy(mBaseIterator);
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
			return Pointer(ReferenceProxy(*mBaseIterator));
		}

		bool operator==(ConstIterator iter) const MOMO_NOEXCEPT
		{
			return mBaseIterator == ConstIteratorProxy::GetBaseIterator(iter);
		}

		MOMO_MORE_TREE_ITERATOR_OPERATORS(TreeDerivedIterator)

	protected:
		explicit TreeDerivedIterator(BaseIterator iter) MOMO_NOEXCEPT
			: mBaseIterator(iter)
		{
		}

		BaseIterator ptGetBaseIterator() const MOMO_NOEXCEPT
		{
			return mBaseIterator;
		}

	private:
		BaseIterator mBaseIterator;
	};

	template<typename TBucketIterator, typename TBaseBucketBounds>
	class HashDerivedBucketBounds
	{
	protected:
		typedef TBucketIterator BucketIterator;
		typedef TBaseBucketBounds BaseBucketBounds;

	public:
		typedef BucketIterator Iterator;

		typedef HashDerivedBucketBounds<typename BucketIterator::ConstIterator,
			typename BaseBucketBounds::ConstBounds> ConstBounds;

	private:
		struct BucketIteratorProxy : public BucketIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(BucketIterator)
		};

		struct ConstBoundsProxy : public ConstBounds
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstBounds)
		};

	public:
		HashDerivedBucketBounds() MOMO_NOEXCEPT
		{
		}

		operator ConstBounds() const MOMO_NOEXCEPT
		{
			return ConstBoundsProxy(mBaseBucketBounds);
		}

		Iterator GetBegin() const MOMO_NOEXCEPT
		{
			return BucketIteratorProxy(mBaseBucketBounds.GetBegin());
		}

		Iterator GetEnd() const MOMO_NOEXCEPT
		{
			return BucketIteratorProxy(mBaseBucketBounds.GetEnd());
		}

		MOMO_FRIENDS_BEGIN_END(const HashDerivedBucketBounds&, BucketIterator)

		size_t GetCount() const MOMO_NOEXCEPT
		{
			return mBaseBucketBounds.GetCount();
		}

	protected:
		explicit HashDerivedBucketBounds(BaseBucketBounds bounds) MOMO_NOEXCEPT
			: mBaseBucketBounds(bounds)
		{
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
