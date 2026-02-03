/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/IteratorUtility.h

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_ITERATOR_UTILITY
#define MOMO_INCLUDE_GUARD_ITERATOR_UTILITY

#include "Utility.h"

#define MOMO_MORE_ARRAY_ITERATOR_OPERATORS(Iterator) \
	MOMO_MORE_COMPARISON_OPERATORS(Iterator) \
	Iterator& operator++() \
	{ \
		return *this += 1; \
	} \
	Iterator operator++(int) \
	{ \
		Iterator resIter = *this; \
		*this += 1; \
		return resIter; \
	} \
	Iterator& operator--() \
	{ \
		return *this += -1; \
	} \
	Iterator operator--(int) \
	{ \
		Iterator resIter = *this; \
		*this += -1; \
		return resIter; \
	} \
	Iterator operator+(ptrdiff_t diff) const \
	{ \
		return Iterator(*this) += diff; \
	} \
	friend Iterator operator+(ptrdiff_t diff, Iterator iter) \
	{ \
		return iter += diff; \
	} \
	Iterator& operator-=(ptrdiff_t diff) \
	{ \
		return *this += (-diff); \
	} \
	Iterator operator-(ptrdiff_t diff) const \
	{ \
		return Iterator(*this) += (-diff); \
	} \
	Reference operator*() const \
	{ \
		return *operator->(); \
	} \
	Reference operator[](ptrdiff_t diff) const \
	{ \
		return *(Iterator(*this) += diff); \
	}

#define MOMO_MORE_HASH_ITERATOR_OPERATORS(Iterator) \
	Iterator operator++(int) \
	{ \
		Iterator resIter = *this; \
		++*this; \
		return resIter; \
	} \
	Reference operator*() const \
	{ \
		return *operator->(); \
	} \
	friend bool operator!=(Iterator iter1, Iterator iter2) noexcept \
	{ \
		return !(iter1 == iter2); \
	} \
	bool operator!() const noexcept \
	{ \
		return *this == Iterator(); \
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
	friend bool operator!=(Position pos1, Position pos2) noexcept \
	{ \
		return !(pos1 == pos2); \
	} \
	bool operator!() const noexcept \
	{ \
		return *this == Position(); \
	} \
	explicit operator bool() const noexcept \
	{ \
		return !!*this; \
	}

#define MOMO_MORE_TREE_ITERATOR_OPERATORS(Iterator) \
	Iterator operator++(int) \
	{ \
		Iterator resIter = *this; \
		++*this; \
		return resIter; \
	} \
	Iterator operator--(int) \
	{ \
		Iterator resIter = *this; \
		--*this; \
		return resIter; \
	} \
	Reference operator*() const \
	{ \
		return *operator->(); \
	} \
	friend bool operator!=(Iterator iter1, Iterator iter2) noexcept \
	{ \
		return !(iter1 == iter2); \
	}

namespace momo
{

namespace internal
{
	template<typename Position>
	struct InsertResult
	{
	public:
		InsertResult() noexcept
			: position(),
			inserted(false)
		{
		}

		InsertResult(Position pos, bool inserted) noexcept
			: position(pos),
			inserted(inserted)
		{
		}

		InsertResult(const InsertResult& insRes) noexcept	// gcc 6, clang 4
			: position(insRes.position),
			inserted(insRes.inserted)
		{
		}

		~InsertResult() = default;

		InsertResult& operator=(const InsertResult& insRes) noexcept
		{
			position = insRes.position;
			inserted = insRes.inserted;
			return *this;
		}

	public:
		union
		{
			Position position;
			Position iterator MOMO_DEPRECATED;
		};
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

	template<typename Iterator>
	struct ConstIteratorSelector
	{
		typedef typename Iterator::ConstIterator ConstIterator;
	};

	template<typename QObject>
	struct ConstIteratorSelector<QObject*>
	{
		typedef const QObject* ConstIterator;
	};

	template<typename QObject>
	struct ConstIteratorSelector<std::reverse_iterator<QObject*>>
	{
		typedef std::reverse_iterator<const QObject*> ConstIterator;
	};

	template<typename TReference>
	class IteratorPointer
	{
	public:
		typedef TReference Reference;

		typedef IteratorPointer<typename Reference::ConstReference> ConstPointer;

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
	class ArrayBoundsBase
	{
	public:
		typedef TIterator Iterator;

	public:
		explicit ArrayBoundsBase() noexcept
			: mBegin(),
			mCount(0)
		{
		}

		explicit ArrayBoundsBase(Iterator begin, size_t count) noexcept
			: mBegin(begin),
			mCount(count)
		{
		}

		Iterator GetBegin() const noexcept
		{
			return mBegin;
		}

		Iterator GetEnd() const noexcept
		{
			return UIntMath<>::Next(mBegin, mCount);
		}

		MOMO_FRIENDS_SIZE_BEGIN_END_CONST(ArrayBoundsBase, Iterator)

		size_t GetCount() const noexcept
		{
			return mCount;
		}

		typename std::iterator_traits<Iterator>::reference operator[](size_t index) const noexcept
		{
			MOMO_ASSERT(index < mCount);
			return *UIntMath<>::Next(mBegin, index);
		}

	private:
		Iterator mBegin;
		size_t mCount;
	};

	template<typename TIterator>
	class ArrayBounds : public ArrayBoundsBase<TIterator>
	{
	private:
		typedef ArrayBoundsBase<TIterator> BoundsBase;

	public:
		using typename BoundsBase::Iterator;

		typedef ArrayBounds<typename ConstIteratorSelector<Iterator>::ConstIterator> ConstBounds;

	public:
		using BoundsBase::BoundsBase;

		operator ConstBounds() const noexcept
		{
			return ConstBounds(BoundsBase::GetBegin(), BoundsBase::GetCount());
		}
	};

	template<typename TBaseIterator, template<typename BaseReference> class TReference>
	class HashDerivedIterator
	{
	protected:
		typedef TBaseIterator BaseIterator;

	public:
		typedef TReference<typename BaseIterator::Reference> Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef HashDerivedIterator<typename ConstIteratorSelector<BaseIterator>::ConstIterator,
			TReference> ConstIterator;

	public:
		explicit HashDerivedIterator() noexcept
			: mBaseIterator()
		{
		}

		operator ConstIterator() const noexcept
		{
			return ProxyConstructor<ConstIterator>(mBaseIterator);
		}

		HashDerivedIterator& operator++()
		{
			++mBaseIterator;
			return *this;
		}

		Pointer operator->() const
		{
			return Pointer(ProxyConstructor<Reference>(*mBaseIterator));
		}

		friend bool operator==(HashDerivedIterator iter1, HashDerivedIterator iter2) noexcept
		{
			return iter1.mBaseIterator == iter2.mBaseIterator;
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

	template<typename TBaseIterator, template<typename BaseReference> class TReference>
	class TreeDerivedIterator
	{
	protected:
		typedef TBaseIterator BaseIterator;

	public:
		typedef TReference<typename BaseIterator::Reference> Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef TreeDerivedIterator<typename ConstIteratorSelector<BaseIterator>::ConstIterator,
			TReference> ConstIterator;

	public:
		explicit TreeDerivedIterator() noexcept
			: mBaseIterator()
		{
		}

		operator ConstIterator() const noexcept
		{
			return ProxyConstructor<ConstIterator>(mBaseIterator);
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
			return Pointer(ProxyConstructor<Reference>(*mBaseIterator));
		}

		friend bool operator==(TreeDerivedIterator iter1, TreeDerivedIterator iter2) noexcept
		{
			return iter1.mBaseIterator == iter2.mBaseIterator;
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

	template<typename Iterator, typename IteratorCategory>
	struct IteratorTraitsStd
	{
		typedef IteratorCategory iterator_category;
		typedef ptrdiff_t difference_type;
		typedef typename Iterator::Pointer pointer;
		typedef typename Iterator::Reference reference;
		typedef typename std::decay<reference>::type value_type;
	};
}

} // namespace momo

namespace std
{
	template<typename BI, template<typename> class R>
	struct iterator_traits<momo::internal::HashDerivedIterator<BI, R>>
		: public momo::internal::IteratorTraitsStd<momo::internal::HashDerivedIterator<BI, R>,
			forward_iterator_tag>
	{
	};

	template<typename BI, template<typename> class R>
	struct iterator_traits<momo::internal::TreeDerivedIterator<BI, R>>
		: public momo::internal::IteratorTraitsStd<momo::internal::TreeDerivedIterator<BI, R>,
			bidirectional_iterator_tag>
	{
	};
} // namespace std

#endif // MOMO_INCLUDE_GUARD_ITERATOR_UTILITY
