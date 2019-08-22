/**********************************************************\

  This file is distributed under the MIT License.
  See https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/IteratorUtility.h

\**********************************************************/

#pragma once

#include "Utility.h"

#define MOMO_MORE_ARRAY_ITERATOR_OPERATORS(Iterator) \
	Iterator& operator++() \
	{ \
		return *this += 1; \
	} \
	Iterator operator++(int) \
	{ \
		Iterator tempIter = *this; \
		++*this; \
		return tempIter; \
	} \
	Iterator& operator--() \
	{ \
		return *this -= 1; \
	} \
	Iterator operator--(int) \
	{ \
		Iterator tempIter = *this; \
		--*this; \
		return tempIter; \
	} \
	Iterator operator+(ptrdiff_t diff) const \
	{ \
		return Iterator(*this) += diff; \
	} \
	friend Iterator operator+(ptrdiff_t diff, Iterator iter) \
	{ \
		return iter + diff; \
	} \
	Iterator& operator-=(ptrdiff_t diff) \
	{ \
		return *this += (-diff); \
	} \
	Iterator operator-(ptrdiff_t diff) const \
	{ \
		return *this + (-diff); \
	} \
	Reference operator*() const \
	{ \
		return *operator->(); \
	} \
	Reference operator[](ptrdiff_t diff) const \
	{ \
		return *(*this + diff); \
	} \
	bool operator!=(ConstIterator iter) const noexcept \
	{ \
		return !(*this == iter); \
	} \
	bool operator>(ConstIterator iter) const \
	{ \
		return iter < *this; \
	} \
	bool operator<=(ConstIterator iter) const \
	{ \
		return !(iter < *this); \
	} \
	bool operator>=(ConstIterator iter) const \
	{ \
		return iter <= *this; \
	}

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
	bool operator!=(ConstIterator iter) const noexcept \
	{ \
		return !(*this == iter); \
	} \
	bool operator!() const noexcept \
	{ \
		return *this == ConstIterator(); \
	} \
	explicit operator bool() const noexcept \
	{ \
		return !!*this; \
	}

#define MOMO_MORE_HASH_POSITION_OPERATORS(Position) \
	Reference operator*() const \
	{ \
		return *operator->(); \
	} \
	bool operator!=(ConstPosition pos) const noexcept \
	{ \
		return !(*this == pos); \
	} \
	bool operator!() const noexcept \
	{ \
		return *this == ConstPosition(); \
	} \
	explicit operator bool() const noexcept \
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
	bool operator!=(ConstIterator iter) const noexcept \
	{ \
		return !(*this == iter); \
	}

namespace momo
{

namespace internal
{
	template<typename Iterator>
	struct InsertResult
	{
		Iterator iterator;
		bool inserted;
	};

	template<typename TSettings,
		bool tCheckVersion = TSettings::checkVersion>
	class VersionKeeper;

	template<typename TSettings>
	class VersionKeeper<TSettings, true>
	{
	public:
		typedef TSettings Settings;

		static const bool checkVersion = true;

	public:
		explicit VersionKeeper() noexcept
			: mContainerVersion(nullptr),
			mVersion(0)
		{
		}

		explicit VersionKeeper(const size_t* version) noexcept
			: mContainerVersion(version),
			mVersion(*version)
		{
		}

		void Check() const
		{
			MOMO_CHECK(mContainerVersion != nullptr && *mContainerVersion == mVersion);
		}

		void Check(const size_t* version, bool allowEmpty = false) const
		{
			(void)version;
			MOMO_ASSERT(version != nullptr);
			if (allowEmpty && mContainerVersion == nullptr)
				return;
			MOMO_CHECK(mContainerVersion == version && mVersion == *version);
		}

	private:
		const size_t* mContainerVersion;
		size_t mVersion;
	};

	template<typename TSettings>
	class VersionKeeper<TSettings, false>
	{
	public:
		typedef TSettings Settings;

		static const bool checkVersion = false;

	public:
		explicit VersionKeeper() noexcept
		{
		}

		explicit VersionKeeper(const size_t* /*version*/) noexcept
		{
		}

		void Check() const
		{
		}

		void Check(const size_t* /*version*/, bool /*allowEmpty*/ = false) const
		{
		}
	};

	template<typename Reference>
	struct ConstReferenceSelector
	{
		typedef typename Reference::ConstReference ConstReference;
	};

	template<typename Object>
	struct ConstReferenceSelector<Object&>
	{
		typedef const Object& ConstReference;
	};

	template<typename Iterator>
	struct ConstIteratorSelector
	{
		typedef typename Iterator::ConstIterator ConstIterator;
	};

	template<typename Object>
	struct ConstIteratorSelector<Object*>
	{
		typedef const Object* ConstIterator;
	};

	template<typename Object>
	struct ConstIteratorSelector<std::reverse_iterator<Object*>>
	{
		typedef std::reverse_iterator<const Object*> ConstIterator;
	};

	template<typename Reference>
	struct Dereferencer
	{
		typedef typename std::decay<Reference>::type Object;
	};

	template<typename TReference>
	class IteratorPointer
	{
	public:
		typedef TReference Reference;

	private:
		typedef typename ConstReferenceSelector<Reference>::ConstReference ConstReference;

	public:
		typedef IteratorPointer<ConstReference> ConstPointer;

	public:
		IteratorPointer() = delete;

		explicit IteratorPointer(Reference ref) noexcept
			: mReference(ref)
		{
		}

		operator ConstPointer() const noexcept
		{
			return ConstPointer(mReference);
		}

		typename std::remove_reference<const Reference>::type* operator->() const noexcept
		{
			return std::addressof(mReference);
		}

		Reference operator*() const noexcept
		{
			return mReference;
		}

		bool operator!() const noexcept
		{
			return false;
		}

		explicit operator bool() const noexcept
		{
			return true;
		}

	private:
		Reference mReference;
	};

	template<typename TIterator>
	class ArrayBounds
	{
	public:
		typedef TIterator Iterator;

		typedef ArrayBounds<typename ConstIteratorSelector<Iterator>::ConstIterator> ConstBounds;

	public:
		explicit ArrayBounds() noexcept
			: mBegin(),
			mCount(0)
		{
		}

		explicit ArrayBounds(Iterator begin, size_t count) noexcept
			: mBegin(begin),
			mCount(count)
		{
		}

		operator ConstBounds() const noexcept
		{
			return ConstBounds(mBegin, mCount);
		}

		Iterator GetBegin() const noexcept
		{
			return mBegin;
		}

		Iterator GetEnd() const noexcept
		{
			return mBegin + mCount;
		}

		MOMO_FRIENDS_BEGIN_END(const ArrayBounds&, Iterator)

		size_t GetCount() const noexcept
		{
			return mCount;
		}

		typename std::iterator_traits<Iterator>::reference operator[](size_t index) const noexcept
		{
			MOMO_ASSERT(index < mCount);
			return mBegin[index];
		}

	private:
		Iterator mBegin;
		size_t mCount;
	};

	template<typename TBaseIterator, typename TReference>
	class HashDerivedIterator
	{
	protected:
		typedef TBaseIterator BaseIterator;

	public:
		typedef TReference Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef HashDerivedIterator<typename ConstIteratorSelector<BaseIterator>::ConstIterator,
			typename ConstReferenceSelector<Reference>::ConstReference> ConstIterator;

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
		explicit HashDerivedIterator() noexcept
			: mBaseIterator()
		{
		}

		operator ConstIterator() const noexcept
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

		bool operator==(ConstIterator iter) const noexcept
		{
			return mBaseIterator == ConstIteratorProxy::GetBaseIterator(iter);
		}

		MOMO_MORE_HASH_ITERATOR_OPERATORS(HashDerivedIterator)

	protected:
		explicit HashDerivedIterator(BaseIterator iter) noexcept
			: mBaseIterator(iter)
		{
		}

		BaseIterator ptGetBaseIterator() const noexcept
		{
			return mBaseIterator;
		}

	private:
		BaseIterator mBaseIterator;
	};

	template<typename TBasePosition, typename TIterator>
	class HashDerivedPosition
	{
	protected:
		typedef TBasePosition BasePosition;

	public:
		typedef TIterator Iterator;

		typedef typename Iterator::Reference Reference;
		typedef IteratorPointer<Reference> Pointer;

		typedef HashDerivedPosition<typename BasePosition::ConstPosition,
			typename ConstIteratorSelector<Iterator>::ConstIterator> ConstPosition;

	private:
		struct ReferenceProxy : public Reference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Reference)
		};

		struct ConstPositionProxy : public ConstPosition
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstPosition)
			MOMO_DECLARE_PROXY_FUNCTION(ConstPosition, GetBasePosition,
				typename ConstPosition::BasePosition)
		};

		struct IteratorProxy : public Iterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Iterator)
			MOMO_DECLARE_PROXY_FUNCTION(Iterator, GetBaseIterator, typename Iterator::BaseIterator)
		};

	public:
		explicit HashDerivedPosition() noexcept
			: mBasePosition()
		{
		}

		template<typename ArgIterator,
			typename = EnableIf<std::is_convertible<ArgIterator, Iterator>::value>>
		HashDerivedPosition(ArgIterator iter) noexcept
			: mBasePosition(IteratorProxy::GetBaseIterator(static_cast<Iterator>(iter)))
		{
		}

		operator ConstPosition() const noexcept
		{
			return ConstPositionProxy(mBasePosition);
		}

		template<typename ResIterator,
			typename = EnableIf<std::is_convertible<Iterator, ResIterator>::value>>
		operator ResIterator() const noexcept
		{
			Iterator iter = IteratorProxy(mBasePosition);
			return static_cast<ResIterator>(iter);
		}

		Pointer operator->() const
		{
			return Pointer(ReferenceProxy(*mBasePosition));
		}

		bool operator==(ConstPosition iter) const noexcept
		{
			return mBasePosition == ConstPositionProxy::GetBasePosition(iter);
		}

		MOMO_MORE_HASH_POSITION_OPERATORS(HashDerivedPosition)

	protected:
		explicit HashDerivedPosition(BasePosition pos) noexcept
			: mBasePosition(pos)
		{
		}

		BasePosition ptGetBasePosition() const noexcept
		{
			return mBasePosition;
		}

	private:
		BasePosition mBasePosition;
	};

	template<typename TBaseIterator, typename TReference>
	class TreeDerivedIterator
	{
	protected:
		typedef TBaseIterator BaseIterator;

	public:
		typedef TReference Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef TreeDerivedIterator<typename ConstIteratorSelector<BaseIterator>::ConstIterator,
			typename ConstReferenceSelector<Reference>::ConstReference> ConstIterator;

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
		explicit TreeDerivedIterator() noexcept
			: mBaseIterator()
		{
		}

		operator ConstIterator() const noexcept
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

		bool operator==(ConstIterator iter) const noexcept
		{
			return mBaseIterator == ConstIteratorProxy::GetBaseIterator(iter);
		}

		MOMO_MORE_TREE_ITERATOR_OPERATORS(TreeDerivedIterator)

	protected:
		explicit TreeDerivedIterator(BaseIterator iter) noexcept
			: mBaseIterator(iter)
		{
		}

		BaseIterator ptGetBaseIterator() const noexcept
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

		typedef HashDerivedBucketBounds<typename ConstIteratorSelector<Iterator>::ConstIterator,
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
		explicit HashDerivedBucketBounds() noexcept
		{
		}

		operator ConstBounds() const noexcept
		{
			return ConstBoundsProxy(mBaseBucketBounds);
		}

		Iterator GetBegin() const noexcept
		{
			return BucketIteratorProxy(mBaseBucketBounds.GetBegin());
		}

		Iterator GetEnd() const noexcept
		{
			return BucketIteratorProxy(mBaseBucketBounds.GetEnd());
		}

		MOMO_FRIENDS_BEGIN_END(const HashDerivedBucketBounds&, BucketIterator)

		size_t GetCount() const noexcept
		{
			return mBaseBucketBounds.GetCount();
		}

	protected:
		explicit HashDerivedBucketBounds(BaseBucketBounds bounds) noexcept
			: mBaseBucketBounds(bounds)
		{
		}

	private:
		BaseBucketBounds mBaseBucketBounds;
	};

	template<typename Iterator, typename IteratorCategory>
	struct IteratorTraitsStd
	{
		typedef IteratorCategory iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename Iterator::Pointer pointer;
		typedef typename Iterator::Reference reference;
		typedef typename Dereferencer<reference>::Object value_type;
	};
}

} // namespace momo

namespace std
{
	template<typename BI, typename R>
	struct iterator_traits<momo::internal::HashDerivedIterator<BI, R>>
		: public momo::internal::IteratorTraitsStd<momo::internal::HashDerivedIterator<BI, R>,
			forward_iterator_tag>
	{
	};

	template<typename BI, typename R>
	struct iterator_traits<momo::internal::TreeDerivedIterator<BI, R>>
		: public momo::internal::IteratorTraitsStd<momo::internal::TreeDerivedIterator<BI, R>,
			bidirectional_iterator_tag>
	{
	};
} // namespace std
