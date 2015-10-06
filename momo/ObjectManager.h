/**********************************************************\

  momo/ObjectManager.h

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

namespace internal
{
	template<typename TObject, size_t tAlignment>
	class ObjectBuffer
	{
	public:
		typedef TObject Object;

		static const size_t alignment = tAlignment;
		//MOMO_STATIC_ASSERT(alignment > 0 && ((alignment - 1) & alignment) == 0);

	public:
		const Object* operator&() const MOMO_NOEXCEPT
		{
			return (const Object*)&mBuffer;
		}

		Object* operator&() MOMO_NOEXCEPT
		{
			return (Object*)&mBuffer;
		}

	private:
		typename std::aligned_storage<sizeof(Object), alignment>::type mBuffer;
	};

	template<typename TObject>
	struct ObjectManager
	{
		typedef TObject Object;

		static const bool isNothrowMoveConstructible = MOMO_IS_NOTHROW_MOVE_CONSTRUCTIBLE(Object);
		static const bool isTriviallyRelocatable = IsTriviallyRelocatable<Object>::value;
		static const bool isNothrowRelocatable = isNothrowMoveConstructible
			|| isTriviallyRelocatable;
		static const bool isNothrowAnywayCopyAssignable =
			std::is_nothrow_copy_assignable<Object>::value
			|| std::is_nothrow_copy_constructible<Object>::value;
		static const bool isNothrowAnywayMoveAssignable =
			std::is_nothrow_move_assignable<Object>::value || isTriviallyRelocatable
			|| isNothrowMoveConstructible || isNothrowAnywayCopyAssignable;

		static const size_t alignment = MOMO_ALIGNMENT_OF(Object);

		template<typename... Args>
		class Creator
		{
		public:
			explicit Creator(Args&&... args)
				: mArgs(std::forward<Args>(args)...)
			{
			}

			explicit Creator(std::tuple<Args...>&& args)
				: mArgs(std::move(args))
			{
			}

			Creator(const Creator&) = delete;

			~Creator() MOMO_NOEXCEPT
			{
			}

			Creator& operator=(const Creator&) = delete;

			void operator()(void* pobject) const
			{
				_Create(pobject, typename MakeSequence<sizeof...(Args)>::Sequence());
			}

		private:
			template<size_t... sequence>
			void _Create(void* pobject, Sequence<sequence...>) const
			{
				new(pobject) Object(std::forward<Args>(std::get<sequence>(mArgs))...);
			}

		private:
			std::tuple<Args&&...> mArgs;
		};

		static void Create(Object&& object, void* pobject)
			MOMO_NOEXCEPT_IF(isNothrowMoveConstructible)
		{
			new(pobject) Object(std::move(object));
		}

		static void Create(const Object& object, void* pobject)
		{
			new(pobject) Object(object);
		}

		static void Destroy(Object& object) MOMO_NOEXCEPT
		{
			(void)object;	// vs warning
			object.~Object();
		}

		static void Destroy(Object* objects, size_t count) MOMO_NOEXCEPT
		{
			if (!std::is_trivially_destructible<Object>::value)
			{
				for (size_t i = 0; i < count; ++i)
					Destroy(objects[i]);
			}
		}

		static void AssignNothrowAnyway(Object&& srcObject, Object& dstObject) MOMO_NOEXCEPT
		{
			_AssignNothrowAnyway(std::move(srcObject), dstObject,
				std::is_nothrow_move_assignable<Object>(), BoolConstant<isTriviallyRelocatable>(),
				BoolConstant<isNothrowMoveConstructible>());
		}

		static void AssignNothrowAnyway(const Object& srcObject, Object& dstObject) MOMO_NOEXCEPT
		{
			_AssignNothrowAnyway(srcObject, dstObject,
				std::is_nothrow_copy_assignable<Object>(),
				std::is_nothrow_copy_constructible<Object>());
		}

		static void Relocate(Object* srcObjects, Object* dstObjects, size_t count)
			MOMO_NOEXCEPT_IF(isNothrowRelocatable)
		{
			_Relocate(srcObjects, dstObjects, count, BoolConstant<isTriviallyRelocatable>(),
				BoolConstant<isNothrowMoveConstructible>());
		}

		template<typename ObjectCreator>
		static void RelocateAddBack(Object* srcObjects, Object* dstObjects, size_t srcCount,
			const ObjectCreator& objectCreator)
		{
			_RelocateAddBack(srcObjects, dstObjects, srcCount, objectCreator,
				BoolConstant<isNothrowRelocatable>());
		}

	private:
		template<bool isTriviallyRelocatable, bool isNothrowMoveConstructible>
		static void _AssignNothrowAnyway(Object&& srcObject, Object& dstObject,
			std::true_type /*isNothrowMoveAssignable*/, BoolConstant<isTriviallyRelocatable>,
			BoolConstant<isNothrowMoveConstructible>) MOMO_NOEXCEPT
		{
			dstObject = std::move(srcObject);
		}

		template<bool isNothrowMoveConstructible>
		static void _AssignNothrowAnyway(Object&& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::true_type /*isTriviallyRelocatable*/,
			BoolConstant<isNothrowMoveConstructible>) MOMO_NOEXCEPT
		{
			static const size_t size = sizeof(Object);
			char buf[size];
			memcpy(buf, std::addressof(dstObject), size);
			memcpy(std::addressof(dstObject), std::addressof(srcObject), size);
			memcpy(std::addressof(srcObject), buf, size);
		}

		static void _AssignNothrowAnyway(Object&& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::false_type /*isTriviallyRelocatable*/,
			std::true_type /*isNothrowMoveConstructible*/) MOMO_NOEXCEPT
		{
			if (std::addressof(srcObject) != std::addressof(dstObject))
			{
				Destroy(dstObject);
				Create(std::move(srcObject), std::addressof(dstObject));
			}
		}

		static void _AssignNothrowAnyway(Object&& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/, std::false_type /*isTriviallyRelocatable*/,
			std::false_type /*isNothrowMoveConstructible*/) MOMO_NOEXCEPT
		{
			AssignNothrowAnyway((const Object&)srcObject, dstObject);
		}

		template<bool isNothrowCopyConstructible>
		static void _AssignNothrowAnyway(const Object& srcObject, Object& dstObject,
			std::true_type /*isNothrowCopyAssignable*/,
			std::integral_constant<bool, isNothrowCopyConstructible>) MOMO_NOEXCEPT
		{
			dstObject = srcObject;
		}

		static void _AssignNothrowAnyway(const Object& srcObject, Object& dstObject,
			std::false_type /*isNothrowCopyAssignable*/,
			std::true_type /*isNothrowCopyConstructible*/) MOMO_NOEXCEPT
		{
			if (std::addressof(srcObject) != std::addressof(dstObject))
			{
				Destroy(dstObject);
				Create(srcObject, std::addressof(dstObject));
			}
		}

		template<bool isNothrowMoveConstructible>
		static void _Relocate(Object* srcObjects, Object* dstObjects, size_t count,
			std::true_type /*isTriviallyRelocatable*/,
			BoolConstant<isNothrowMoveConstructible>) MOMO_NOEXCEPT
		{
			memcpy(dstObjects, srcObjects, count * sizeof(Object));
		}

		static void _Relocate(Object* srcObjects, Object* dstObjects, size_t count,
			std::false_type /*isTriviallyRelocatable*/,
			std::true_type /*isNothrowMoveConstructible*/) MOMO_NOEXCEPT
		{
			if (count > 0)	// vs
				std::uninitialized_copy_n(std::make_move_iterator(srcObjects), count, dstObjects);
			Destroy(srcObjects, count);
		}

		static void _Relocate(Object* srcObjects, Object* dstObjects, size_t count,
			std::false_type /*isTriviallyRelocatable*/,
			std::false_type /*isNothrowMoveConstructible*/)
		{
			if (count > 0)
			{
				_RelocateAddBack(srcObjects, dstObjects, count - 1,
					Creator<Object>(std::move(srcObjects[count - 1])), std::false_type());
			}
		}

		template<typename ObjectCreator>
		static void _RelocateAddBack(Object* srcObjects, Object* dstObjects, size_t srcCount,
			const ObjectCreator& objectCreator, std::true_type /*isNothrowRelocatable*/)
		{
			objectCreator(dstObjects + srcCount);
			Relocate(srcObjects, dstObjects, srcCount);
		}

		template<typename ObjectCreator>
		static void _RelocateAddBack(Object* srcObjects, Object* dstObjects, size_t srcCount,
			const ObjectCreator& objectCreator, std::false_type /*isNothrowRelocatable*/)
		{
			if (srcCount > 0)	// vs
				std::uninitialized_copy_n(srcObjects, srcCount, dstObjects);
			try
			{
				objectCreator(dstObjects + srcCount);
			}
			catch (...)
			{
				Destroy(dstObjects, srcCount);
				throw;
			}
			Destroy(srcObjects, srcCount);
		}
	};
}

} // namespace momo
