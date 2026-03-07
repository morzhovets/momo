/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/branch_cpp11/LICENSE
  for details.

  momo/FunctionUtility.h

\**********************************************************/

#ifndef MOMO_INCLUDE_GUARD_FUNCTION_UTILITY
#define MOMO_INCLUDE_GUARD_FUNCTION_UTILITY

#include "Utility.h"

namespace momo
{

namespace internal
{
	template<typename... Args>
	class FinalizerArgs
	{
	public:
		FinalizerArgs(Args... args) noexcept
			: mArgs(args...)
		{
		}

	protected:
		void ptExecute(void (*func)(Args...)) noexcept
		{
			pvExecute(func, typename SequenceMaker<sizeof...(Args)>::Sequence());
		}

	private:
		template<size_t... sequence>
		void pvExecute(void (*func)(Args...), internal::Sequence<sequence...>) noexcept
		{
			func(std::get<sequence>(mArgs)...);
		}

	private:
		std::tuple<Args...> mArgs;
	};

	template<>
	class FinalizerArgs<>
	{
	public:
		FinalizerArgs() = default;

	protected:
		template<typename Class>
		void ptExecute(void (Class::*func)(), Class& object) noexcept
		{
			(object.*func)();
		}
	};

	template<typename Arg0>
	class FinalizerArgs<Arg0>
	{
	protected:
		void ptExecute(void (*func)(Arg0)) noexcept
		{
			func(arg0);
		}

		template<typename Class>
		void ptExecute(void (Class::*func)(Arg0), Class& object) noexcept
		{
			(object.*func)(arg0);
		}

	public:
		Arg0 arg0;
	};

	template<typename Arg0, typename Arg1>
	class FinalizerArgs<Arg0, Arg1>
	{
	protected:
		void ptExecute(void (*func)(Arg0, Arg1)) noexcept
		{
			func(arg0, arg1);
		}

		template<typename Class>
		void ptExecute(void (Class::*func)(Arg0, Arg1), Class& object) noexcept
		{
			(object.*func)(arg0, arg1);
		}

	public:
		Arg0 arg0;
		Arg1 arg1;
	};

	template<typename Arg0, typename Arg1, typename Arg2>
	class FinalizerArgs<Arg0, Arg1, Arg2>
	{
	protected:
		void ptExecute(void (*func)(Arg0, Arg1, Arg2)) noexcept
		{
			func(arg0, arg1, arg2);
		}

		template<typename Class>
		void ptExecute(void (Class::*func)(Arg0, Arg1, Arg2), Class& object) noexcept
		{
			(object.*func)(arg0, arg1, arg2);
		}

	public:
		Arg0 arg0;
		Arg1 arg1;
		Arg2 arg2;
	};

	template<typename TFunction>
	class Finalizer;

	template<typename... Args>
	class Finalizer<void (*)(Args...)>
		: private FinalizerArgs<Args...>
	{
	public:
		typedef void (*Function)(Args...);

	private:
		typedef internal::FinalizerArgs<Args...> FinalizerArgs;

	public:
		explicit Finalizer(Function func, Args... args) noexcept
			: FinalizerArgs{args...},
			mFunction(func)
		{
		}

#ifndef MOMO_HAS_GUARANTEED_COPY_ELISION
		Finalizer(Finalizer&& fin) noexcept
			: FinalizerArgs(fin),
			mFunction(fin.mFunction)
		{
			fin.Detach();
		}
#endif

		Finalizer(const Finalizer&) = delete;

		~Finalizer() noexcept
		{
			if (mFunction != nullptr)
				FinalizerArgs::ptExecute(mFunction);
		}

		Finalizer& operator=(const Finalizer&) = delete;

		explicit operator bool() const noexcept
		{
			return mFunction != nullptr;
		}

		void Detach() noexcept
		{
			mFunction = nullptr;
		}

	private:
		Function mFunction;
	};

	template<typename Class, typename... Args>
	class Finalizer<void (Class::*)(Args...)>
		: private FinalizerArgs<Args...>
	{
	public:
		typedef void (Class::*Function)(Args...);

	private:
		typedef internal::FinalizerArgs<Args...> FinalizerArgs;

	public:
		explicit Finalizer(Function func, Class& object, Args... args) noexcept
			: FinalizerArgs{args...},
			mObject(object),
			mFunction(func)
		{
		}

#ifndef MOMO_HAS_GUARANTEED_COPY_ELISION
		Finalizer(Finalizer&& fin) noexcept
			: FinalizerArgs(fin),
			mObject(fin.mObject),
			mFunction(fin.mFunction)
		{
			fin.Detach();
		}
#endif

		Finalizer(const Finalizer&) = delete;

		~Finalizer() noexcept
		{
			if (mFunction != nullptr)
				FinalizerArgs::ptExecute(mFunction, mObject);
		}

		Finalizer& operator=(const Finalizer&) = delete;

		explicit operator bool() const noexcept
		{
			return mFunction != nullptr;
		}

		void Detach() noexcept
		{
			mFunction = nullptr;
		}

	private:
		Class& mObject;
		Function mFunction;
	};

	class Catcher
	{
	public:
		template<typename Settings,
			typename = bool>
		class AllowExceptionSuppression
#if defined(MOMO_DISABLE_EXCEPTIONS) || defined(MOMO_CATCH_ALL)
			: public std::true_type
#else
			: public std::false_type
#endif
		{
		};

#if !defined(MOMO_DISABLE_EXCEPTIONS) && defined(MOMO_CATCH_ALL)
		template<typename Settings>
		class AllowExceptionSuppression<Settings,
			typename std::decay<decltype(Settings::allowExceptionSuppression)>::type>
			: public BoolConstant<Settings::allowExceptionSuppression>
		{
		};
#endif

	public:
		template<typename... Args>
		static Finalizer<void (*)(Args...)> Finalize(
			void (*func)(Args...), Identity<Args>... args) noexcept
		{
			return Finalizer<void (*)(Args...)>(func, args...);
		}

		template<typename Class, typename... Args>
		static Finalizer<void (Class::*)(Args...)> Finalize(
			void (Class::*func)(Args...), Identity<Class>& object, Identity<Args>... args) noexcept
		{
			return Finalizer<void (Class::*)(Args...)>(func, object, args...);
		}

		template<typename Functor, typename... Args>
		static bool CatchAll(Functor&& func, Args&&... args) noexcept
		{
#if !defined(MOMO_DISABLE_EXCEPTIONS) && !defined(MOMO_CATCH_ALL)
			MOMO_ASSERT(false);
#endif
			bool res = false;
#if !defined(MOMO_DISABLE_EXCEPTIONS) && defined(MOMO_CATCH_ALL)
			try
#endif
			{
				pvInvoke(std::forward<Functor>(func), std::forward<Args>(args)...);
				res = true;
			}
#if !defined(MOMO_DISABLE_EXCEPTIONS) && defined(MOMO_CATCH_ALL)
			MOMO_CATCH_ALL
#endif
			return res;
		}

	private:
		template<typename Functor, typename... Args>
		static EnableIf<!std::is_member_pointer<typename std::decay<Functor>::type>::value>
		pvInvoke(Functor&& func, Args&&... args)
		{
			std::forward<Functor>(func)(std::forward<Args>(args)...);
		}

		template<typename Class, typename... Args>
		static void pvInvoke(void (Class::*func)(Args...),
			Identity<Class>& object, Identity<Args>... args)
		{
			(object.*func)(args...);
		}
	};
}

} // namespace momo

#endif // MOMO_INCLUDE_GUARD_FUNCTION_UTILITY
