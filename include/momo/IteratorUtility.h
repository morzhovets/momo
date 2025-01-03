/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
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
	}

#define MOMO_MORE_BIDIRECTIONAL_ITERATOR_OPERATORS(Iterator) \
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
	}

#define MOMO_MORE_FORWARD_ITERATOR_OPERATORS(Iterator) \
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
	bool operator!() const noexcept \
	{ \
		return *this == Iterator(); \
	} \
	explicit operator bool() const noexcept \
	{ \
		return !!*this; \
	}

#define MOMO_MORE_POSITION_OPERATORS(Position) \
	Reference operator*() const \
	{ \
		return *operator->(); \
	} \
	bool operator!() const noexcept \
	{ \
		return *this == Position(); \
	} \
	explicit operator bool() const noexcept \
	{ \
		return !!*this; \
	}

namespace momo
{

namespace internal
{
	template<typename Position>
	struct InsertResult
	{
		Position position;
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

		void Check([[maybe_unused]] const size_t* version, bool allowEmpty = false) const
		{
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
		explicit VersionKeeper() noexcept = default;

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

		decltype(auto) operator->() const noexcept
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

	template<conceptRandomIterator17 TIterator>
	class ArrayBoundsBase : public Rangeable
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

		size_t GetCount() const noexcept
		{
			return mCount;
		}

		decltype(auto) operator[](size_t index) const noexcept
		{
			MOMO_ASSERT(index < mCount);
			return *UIntMath<>::Next(mBegin, index);
		}

	private:
		Iterator mBegin;
		size_t mCount;
	};

	template<conceptRandomIterator17 TIterator>
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

	template<std::bidirectional_iterator TBaseIterator,
		template<typename BaseReference> class TReference>
	class DerivedBidirectionalIterator
	{
	protected:
		typedef TBaseIterator BaseIterator;

	public:
		typedef TReference<typename BaseIterator::Reference> Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef DerivedBidirectionalIterator<
			typename ConstIteratorSelector<BaseIterator>::ConstIterator, TReference> ConstIterator;

	private:
		struct ReferenceProxy : public Reference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Reference)
		};

		struct ConstIteratorProxy : public ConstIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		};

	public:
		explicit DerivedBidirectionalIterator() noexcept
			: mBaseIterator()
		{
		}

		operator ConstIterator() const noexcept
		{
			return ConstIteratorProxy(mBaseIterator);
		}

		DerivedBidirectionalIterator& operator++()
		{
			++mBaseIterator;
			return *this;
		}

		DerivedBidirectionalIterator& operator--()
		{
			--mBaseIterator;
			return *this;
		}

		Pointer operator->() const
		{
			return Pointer(ReferenceProxy(*mBaseIterator));
		}

		friend bool operator==(DerivedBidirectionalIterator iter1,
			DerivedBidirectionalIterator iter2) noexcept = default;

		MOMO_MORE_BIDIRECTIONAL_ITERATOR_OPERATORS(DerivedBidirectionalIterator)

	protected:
		explicit DerivedBidirectionalIterator(BaseIterator iter) noexcept
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

	template<std::forward_iterator TBaseIterator,
		template<typename BaseReference> class TReference>
	class DerivedForwardIterator
	{
	protected:
		typedef TBaseIterator BaseIterator;

	public:
		typedef TReference<typename BaseIterator::Reference> Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef DerivedForwardIterator<typename ConstIteratorSelector<BaseIterator>::ConstIterator,
			TReference> ConstIterator;

	private:
		struct ReferenceProxy : public Reference
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(Reference)
		};

		struct ConstIteratorProxy : public ConstIterator
		{
			MOMO_DECLARE_PROXY_CONSTRUCTOR(ConstIterator)
		};

	public:
		explicit DerivedForwardIterator() noexcept
			: mBaseIterator()
		{
		}

		operator ConstIterator() const noexcept
		{
			return ConstIteratorProxy(mBaseIterator);
		}

		DerivedForwardIterator& operator++()
		{
			++mBaseIterator;
			return *this;
		}

		Pointer operator->() const
		{
			return Pointer(ReferenceProxy(*mBaseIterator));
		}

		friend bool operator==(DerivedForwardIterator iter1,
			DerivedForwardIterator iter2) noexcept = default;

		MOMO_MORE_FORWARD_ITERATOR_OPERATORS(DerivedForwardIterator)

	protected:
		explicit DerivedForwardIterator(BaseIterator iter) noexcept
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

	template<typename TPointerGenerator>
	requires (std::copy_constructible<TPointerGenerator>) &&
		(noexcept(std::declval<TPointerGenerator>()())) &&
		(std::is_pointer_v<decltype(std::declval<TPointerGenerator>()())>)
	class IncIterator
	{
	public:
		typedef TPointerGenerator PointerGenerator;

		typedef decltype(std::declval<PointerGenerator>()()) Pointer;

	public:
		IncIterator(const PointerGenerator& gen) noexcept	//?
			: mPointerGenerator(gen)
		{
		}

		[[nodiscard]] Pointer operator++(int) noexcept
		{
			return mPointerGenerator();
		}

	private:
		PointerGenerator mPointerGenerator;
	};

	template<typename Iterator, typename IteratorCategory,
		typename IteratorConcept = IteratorCategory>
	struct IteratorTraitsStd
	{
		typedef IteratorCategory iterator_category;
		typedef IteratorConcept iterator_concept;
		typedef ptrdiff_t difference_type;
		typedef typename Iterator::Pointer pointer;
		typedef typename Iterator::Reference reference;
		typedef std::decay_t<reference> value_type;
	};
}

} // namespace momo

namespace std
{
	template<typename BI, template<typename> class R>
	struct iterator_traits<momo::internal::DerivedBidirectionalIterator<BI, R>>
		: public momo::internal::IteratorTraitsStd<momo::internal::DerivedBidirectionalIterator<BI, R>,
			bidirectional_iterator_tag>
	{
	};

	template<typename BI, template<typename> class R>
	struct iterator_traits<momo::internal::DerivedForwardIterator<BI, R>>
		: public momo::internal::IteratorTraitsStd<momo::internal::DerivedForwardIterator<BI, R>,
			forward_iterator_tag>
	{
	};

	template<typename G>
	struct iterator_traits<momo::internal::IncIterator<G>>
		: public momo::internal::IteratorTraitsStd<momo::internal::IncIterator<G>,
			forward_iterator_tag>	//?
	{
	};
} // namespace std
