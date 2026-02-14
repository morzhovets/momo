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

	class PositionBase
	{
	public:
		template<typename Iterator>
		typename Iterator::Reference operator*(this const Iterator& iter)
		{
			return *iter.operator->();
		}

		template<typename Iterator>
		explicit operator bool(this const Iterator& iter) noexcept
			requires requires { { iter.operator!() } noexcept; }
		{
			return !!iter;
		}
	};

	class ForwardIteratorBase : public PositionBase
	{
	public:
		template<conceptMutableThis RIterator>
		std::remove_reference_t<RIterator> operator++(this RIterator&& iter, int)
		{
			std::remove_reference_t<RIterator> resIter = iter;
			++iter;
			return resIter;
		}
	};

	class BidirectionalIteratorBase : public ForwardIteratorBase
	{
	public:
		template<conceptMutableThis RIterator>
		std::remove_reference_t<RIterator> operator--(this RIterator&& iter, int)
		{
			std::remove_reference_t<RIterator> resIter = iter;
			--iter;
			return resIter;
		}
	};

	class ArrayIteratorBase : public BidirectionalIteratorBase
	{
	public:
		template<conceptMutableThis RIterator>
		std::remove_reference_t<RIterator>& operator++(this RIterator&& iter)
		{
			return iter += 1;
		}

		template<conceptMutableThis RIterator>
		std::remove_reference_t<RIterator>& operator--(this RIterator&& iter)
		{
			return iter += -1;
		}

		using BidirectionalIteratorBase::operator++;
		using BidirectionalIteratorBase::operator--;

		template<typename Iterator>
		Iterator operator+(this Iterator iter, ptrdiff_t diff)
		{
			return iter += diff;
		}

		template<std::derived_from<ArrayIteratorBase> Iterator>
		friend Iterator operator+(ptrdiff_t diff, Iterator iter)
		{
			return iter += diff;
		}

		template<conceptMutableThis RIterator>
		std::remove_reference_t<RIterator>& operator-=(this RIterator&& iter, ptrdiff_t diff)
		{
			return iter += (-diff);
		}

		template<typename Iterator>
		Iterator operator-(this Iterator iter, ptrdiff_t diff)
		{
			return iter += (-diff);
		}

		template<typename Iterator>
		typename Iterator::Reference operator[](this Iterator iter, ptrdiff_t diff)
		{
			return *(iter += diff);
		}
	};

	template<std::bidirectional_iterator TBaseIterator,
		template<typename BaseReference> class TReference>
	class DerivedBidirectionalIterator : public BidirectionalIteratorBase
	{
	protected:
		typedef TBaseIterator BaseIterator;

	public:
		typedef TReference<typename BaseIterator::Reference> Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef DerivedBidirectionalIterator<
			typename ConstIteratorSelector<BaseIterator>::ConstIterator, TReference> ConstIterator;

	public:
		explicit DerivedBidirectionalIterator() noexcept
			: mBaseIterator()
		{
		}

		operator ConstIterator() const noexcept
		{
			return ProxyConstructor<ConstIterator>(mBaseIterator);
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

		using BidirectionalIteratorBase::operator++;
		using BidirectionalIteratorBase::operator--;

		Pointer operator->() const
		{
			return Pointer(ProxyConstructor<Reference>(*mBaseIterator));
		}

		friend bool operator==(DerivedBidirectionalIterator iter1,
			DerivedBidirectionalIterator iter2) noexcept
		{
			return iter1.mBaseIterator == iter2.mBaseIterator;
		}

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
	class DerivedForwardIterator : public ForwardIteratorBase
	{
	protected:
		typedef TBaseIterator BaseIterator;

	public:
		typedef TReference<typename BaseIterator::Reference> Reference;

		typedef IteratorPointer<Reference> Pointer;

		typedef DerivedForwardIterator<typename ConstIteratorSelector<BaseIterator>::ConstIterator,
			TReference> ConstIterator;

	public:
		explicit DerivedForwardIterator() noexcept
			: mBaseIterator()
		{
		}

		operator ConstIterator() const noexcept
		{
			return ProxyConstructor<ConstIterator>(mBaseIterator);
		}

		DerivedForwardIterator& operator++()
		{
			++mBaseIterator;
			return *this;
		}

		using ForwardIteratorBase::operator++;

		Pointer operator->() const
		{
			return Pointer(ProxyConstructor<Reference>(*mBaseIterator));
		}

		bool operator!() const noexcept
			requires requires { { !BaseIterator() } noexcept; }
		{
			return !mBaseIterator;
		}

		friend bool operator==(DerivedForwardIterator iter1, DerivedForwardIterator iter2) noexcept
		{
			return iter1.mBaseIterator == iter2.mBaseIterator;
		}

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

	template<std::forward_iterator BaseIterator,
		template<typename BaseReference> class Reference>
	struct DerivedIteratorSelector
	{
		typedef DerivedForwardIterator<BaseIterator, Reference> DerivedIterator;
	};

	template<std::bidirectional_iterator BaseIterator,
		template<typename BaseReference> class Reference>
	struct DerivedIteratorSelector<BaseIterator, Reference>
	{
		typedef DerivedBidirectionalIterator<BaseIterator, Reference> DerivedIterator;
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
