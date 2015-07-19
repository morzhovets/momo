/**********************************************************\

  momo/ObjectManager.h

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

namespace internal
{
	template<typename TObject,
		size_t tObjectCount = 1>
	class ObjectBuffer
	{
	public:
		typedef TObject Object;

		static const size_t objectCount = tObjectCount;

	public:
		const Object* operator&() const MOMO_NOEXCEPT
		{
			return (const Object*)mBuffer;
		}

		Object* operator&() MOMO_NOEXCEPT
		{
			return (Object*)mBuffer;
		}

	//private:
	//	MOMO_DISABLE_COPY_CONSTRUCTOR(ObjectBuffer);
	//	MOMO_DISABLE_COPY_OPERATOR(ObjectBuffer);

	private:
		char mBuffer[objectCount * sizeof(Object)];
	};

	template<typename TObject>
	class ObjectBuffer<TObject, 0>
	{
	public:
		typedef TObject Object;

		static const size_t objectCount = 0;

	public:
		const void* operator&() const MOMO_NOEXCEPT
		{
			return this;
		}

		void* operator&() MOMO_NOEXCEPT
		{
			return this;
		}
	};

	template<typename TObject>
	struct ObjectManager
	{
		typedef TObject Object;

		static const bool isNothrowMoveConstructible = MOMO_IS_NOTHROW_MOVE_CONSTRUCTIBLE(Object);
		static const bool isNothrowAnywayCopyAssignable =
			std::is_nothrow_copy_assignable<Object>::value
			|| std::is_nothrow_copy_constructible<Object>::value;
		static const bool isNothrowAnywayMoveAssignable = isNothrowAnywayCopyAssignable
			|| std::is_nothrow_move_assignable<Object>::value
			|| std::is_nothrow_move_constructible<Object>::value;
		static const bool isTriviallyRelocatable = IsTriviallyRelocatable<Object>::value;
		static const bool isNothrowRelocatable = isNothrowMoveConstructible
			|| isTriviallyRelocatable;

		class Creator
		{
		public:
			void operator()(void* pobject) const
			{
				Create(pobject);
			}
		};

		class MoveCreator
		{
		public:
			explicit MoveCreator(Object&& object) MOMO_NOEXCEPT
				: mObject(std::move(object))
			{
			}

			void operator()(void* pobject) const MOMO_NOEXCEPT_IF(isNothrowMoveConstructible)
			{
				Create(std::move(mObject), pobject);
			}

		private:
			Object&& mObject;
		};

		class CopyCreator
		{
		public:
			explicit CopyCreator(const Object& object) MOMO_NOEXCEPT
				: mObject(object)
			{
			}

			void operator()(void* pobject) const
			{
				Create(mObject, pobject);
			}

		private:
			const Object& mObject;
		};

		static void Create(void* pobject)
		{
			new(pobject) Object();
		}

		template<typename Argument>
		static void Create(Argument&& arg, void* pobject)
		{
			new(pobject) Object(std::forward<Argument>(arg));
		}

		template<typename Object2Creator>
		static void CreatePair(Object&& object1, const Object2Creator& object2Creator,
			void* pobject1, void* pobject2)
		{
			return _CreatePair(std::move_if_noexcept(object1), object2Creator,
				pobject1, pobject2);
		}

		template<typename Object2Creator>
		static void CreatePair(const Object& object1, const Object2Creator& object2Creator,
			void* pobject1, void* pobject2)
		{
			return _CreatePair(object1, object2Creator, pobject1, pobject2);
		}

		static void Destroy(Object& object) MOMO_NOEXCEPT
		{
			(void)object;	// vs bug
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
			static std::is_nothrow_move_assignable<Object> isNothrowMoveAssignable;
			static std::is_nothrow_move_constructible<Object> isNothrowMoveConstructible;
			_AssignNothrowAnyway(std::move(srcObject), dstObject,
				isNothrowMoveAssignable, &isNothrowMoveConstructible);
		}

		static void AssignNothrowAnyway(const Object& srcObject, Object& dstObject) MOMO_NOEXCEPT
		{
			static std::is_nothrow_copy_assignable<Object> isNothrowCopyAssignable;
			static std::is_nothrow_copy_constructible<Object> isNothrowCopyConstructible;
			_AssignNothrowAnyway(srcObject, dstObject,
				isNothrowCopyAssignable, &isNothrowCopyConstructible);
		}

		static void Relocate(Object* srcObjects, Object* dstObjects, size_t count)
			MOMO_NOEXCEPT_IF(isNothrowRelocatable)
		{
			static std::integral_constant<bool, isNothrowMoveConstructible> isNothrowMoveConstructible;
			_Relocate(srcObjects, dstObjects, count,
				std::integral_constant<bool, isTriviallyRelocatable>(),
				&isNothrowMoveConstructible);
		}

		template<typename ObjectCreator>
		static void RelocateAddBack(Object* srcObjects, Object* dstObjects, size_t srcCount,
			const ObjectCreator& objectCreator)
		{
			_RelocateAddBack(srcObjects, dstObjects, srcCount, objectCreator,
				std::integral_constant<bool, isNothrowRelocatable>());
		}

	private:
		template<typename Object2Creator>
		static void _CreatePair(Object&& object1, const Object2Creator& object2Creator,
			void* pobject1, void* pobject2)
		{
			MOMO_STATIC_ASSERT(isNothrowMoveConstructible);
			object2Creator(pobject2);
			Create(std::move(object1), pobject1);
		}

		template<typename Object2Creator>
		static void _CreatePair(const Object& object1, const Object2Creator& object2Creator,
			void* pobject1, void* pobject2)
		{
			Create(object1, pobject1);
			try
			{
				object2Creator(pobject2);
			}
			catch (...)
			{
				Destroy(*(Object*)pobject1);
				throw;
			}
		}

		static void _AssignNothrowAnyway(Object&& srcObject, Object& dstObject,
			std::true_type /*isNothrowMoveAssignable*/,
			void* /*isNothrowMoveConstructible*/) MOMO_NOEXCEPT
		{
			dstObject = std::move(srcObject);
		}

		static void _AssignNothrowAnyway(Object&& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/,
			std::true_type* /*isNothrowMoveConstructible*/) MOMO_NOEXCEPT
		{
			if (std::addressof(srcObject) != std::addressof(dstObject))
			{
				Destroy(dstObject);
				Create(std::move(srcObject), std::addressof(dstObject));
			}
		}

		static void _AssignNothrowAnyway(Object&& srcObject, Object& dstObject,
			std::false_type /*isNothrowMoveAssignable*/,
			std::false_type* /*isNothrowMoveConstructible*/) MOMO_NOEXCEPT
		{
			AssignNothrowAnyway((const Object&)srcObject, dstObject);
		}

		static void _AssignNothrowAnyway(const Object& srcObject, Object& dstObject,
			std::true_type /*isNothrowCopyAssignable*/,
			void* /*isNothrowCopyConstructible*/) MOMO_NOEXCEPT
		{
			dstObject = srcObject;
		}

		static void _AssignNothrowAnyway(const Object& srcObject, Object& dstObject,
			std::false_type /*isNothrowCopyAssignable*/,
			std::true_type* /*isNothrowCopyConstructible*/) MOMO_NOEXCEPT
		{
			if (std::addressof(srcObject) != std::addressof(dstObject))
			{
				Destroy(dstObject);
				Create(srcObject, std::addressof(dstObject));
			}
		}

		static void _Relocate(Object* srcObjects, Object* dstObjects, size_t count,
			std::true_type /*isTriviallyRelocatable*/,
			void* /*isNothrowMoveConstructible*/) MOMO_NOEXCEPT
		{
			memcpy(dstObjects, srcObjects, count * sizeof(Object));
		}

		static void _Relocate(Object* srcObjects, Object* dstObjects, size_t count,
			std::false_type /*isTriviallyRelocatable*/,
			std::true_type* /*isNothrowMoveConstructible*/) MOMO_NOEXCEPT
		{
			if (count > 0)	// vs bug
				std::uninitialized_copy_n(std::make_move_iterator(srcObjects), count, dstObjects);
			Destroy(srcObjects, count);
		}

		static void _Relocate(Object* srcObjects, Object* dstObjects, size_t count,
			std::false_type /*isTriviallyRelocatable*/,
			std::false_type* /*isNothrowMoveConstructible*/)
		{
			if (count > 0)
			{
				_RelocateAddBack(srcObjects, dstObjects, count - 1,
					MoveCreator(std::move(srcObjects[count - 1])), std::false_type());
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
			if (srcCount > 0)	// vs bug
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
