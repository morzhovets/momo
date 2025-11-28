/**********************************************************\

  This file is part of the
  https://github.com/morzhovets/momo
  project, distributed under the MIT License. See
  https://github.com/morzhovets/momo/blob/master/LICENSE
  for details.

  momo/FunctionUtility.h

  namespace momo:
    class FastMovableFunctor
    class FastCopyableFunctor

\**********************************************************/

#pragma once

#include "Utility.h"

namespace momo
{

namespace internal
{
	template<typename Result, typename BaseResult>
	concept conceptFunctorResult =
		std::is_void_v<BaseResult> || std::convertible_to<Result, BaseResult>;

	template<typename Functor, typename Result, typename... Args>
	concept conceptMoveFunctor =
		std::is_nothrow_destructible_v<Functor> &&
		requires (Functor func, Args&&... args)
			{ { std::forward<Functor>(func)(std::forward<Args>(args)...) }
				-> conceptFunctorResult<Result>; };

	template<typename Functor, typename Result, typename... Args>
	concept conceptConstFunctor =
		std::is_nothrow_destructible_v<Functor> &&
		requires (Functor func, Args&&... args)
			{ { std::as_const(func)(std::forward<Args>(args)...) }
				-> conceptFunctorResult<Result>; };

	template<typename Executor>
	concept conceptExecutor = conceptMoveFunctor<Executor, void>;

	template<typename Predicate, typename... Args>
	concept conceptPredicate = conceptConstFunctor<Predicate, bool, Args...>;

	template<typename... Args>
	requires ((std::is_nothrow_destructible_v<Args> && std::is_nothrow_copy_constructible_v<Args>) && ...)
	class FinalizerArgs
	{
	public:
		FinalizerArgs(Args... args) noexcept
			: mArgs(args...)
		{
		}

	protected:
		void ptExecute(void (*func)(Args...) noexcept) noexcept
		{
			std::apply(func, mArgs);
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
		void ptExecute(void (Class::*func)() noexcept, Class& object) noexcept
		{
			(object.*func)();
		}
	};

	template<typename Arg0>
	class FinalizerArgs<Arg0>
	{
	protected:
		void ptExecute(void (*func)(Arg0) noexcept) noexcept
		{
			func(arg0);
		}

		template<typename Class>
		void ptExecute(void (Class::*func)(Arg0) noexcept, Class& object) noexcept
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
		void ptExecute(void (*func)(Arg0, Arg1) noexcept) noexcept
		{
			func(arg0, arg1);
		}

		template<typename Class>
		void ptExecute(void (Class::*func)(Arg0, Arg1) noexcept, Class& object) noexcept
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
		void ptExecute(void (*func)(Arg0, Arg1, Arg2) noexcept) noexcept
		{
			func(arg0, arg1, arg2);
		}

		template<typename Class>
		void ptExecute(void (Class::*func)(Arg0, Arg1, Arg2) noexcept, Class& object) noexcept
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
	class Finalizer<void (*)(Args...) noexcept>
		: private FinalizerArgs<Args...>
	{
	public:
		typedef void (*Function)(Args...) noexcept;

	private:
		typedef internal::FinalizerArgs<Args...> FinalizerArgs;

	public:
		[[nodiscard]] explicit Finalizer(Function func, Args... args) noexcept
			: FinalizerArgs(args...),
			mFunction(func)
		{
		}

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

	template<typename... Args>
	Finalizer(void (*)(Args...) noexcept, std::type_identity_t<Args>...)
		-> Finalizer<void (*)(Args...) noexcept>;

	template<typename Class, typename... Args>
	class Finalizer<void (Class::*)(Args...) noexcept>
		: private FinalizerArgs<Args...>
	{
	public:
		typedef void (Class::*Function)(Args...) noexcept;

	private:
		typedef internal::FinalizerArgs<Args...> FinalizerArgs;

	public:
		[[nodiscard]] explicit Finalizer(Function func, Class& object, Args... args) noexcept
			: FinalizerArgs(args...),
			mObject(object),
			mFunction(func)
		{
		}

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
		Function mFunction;
		Class& mObject;
	};

	template<typename Class, typename... Args>
	Finalizer(void (Class::*)(Args...) noexcept,
		std::type_identity_t<Class>&, std::type_identity_t<Args>...)
		-> Finalizer<void (Class::*)(Args...) noexcept>;

	template<typename Class>
	Finalizer(std::type_identity_t<void (Class::*)() noexcept>, Class&)
		-> Finalizer<void (Class::*)() noexcept>;

	class Catcher
	{
	public:
		template<typename Settings>
#if defined(MOMO_DISABLE_EXCEPTIONS)
		static const bool allowExceptionSuppression = true;
#elif defined(MOMO_CATCH_ALL)
		static const bool allowExceptionSuppression = Settings::allowExceptionSuppression;
#else
		static const bool allowExceptionSuppression = false;
#endif

	public:
		template<typename... Args, std::invocable<Args&&...> Functor>
		static bool CatchAll(Functor&& func, Args&&... args) noexcept
		{
#if !defined(MOMO_DISABLE_EXCEPTIONS) && !defined(MOMO_CATCH_ALL)
			static_assert(false);
#endif
			bool res = false;
#if !defined(MOMO_DISABLE_EXCEPTIONS) && defined(MOMO_CATCH_ALL)
			try
#endif
			{
				if constexpr (std::is_member_pointer_v<std::decay_t<Functor>>)
					pvInvoke(std::forward<Functor>(func), std::forward<Args>(args)...);
				else
					std::forward<Functor>(func)(std::forward<Args>(args)...);
				res = true;
			}
#if !defined(MOMO_DISABLE_EXCEPTIONS) && defined(MOMO_CATCH_ALL)
			MOMO_CATCH_ALL
#endif
			return res;
		}

	private:
		template<typename Class, typename... Args>
		static void pvInvoke(void (Class::*func)(Args...), Class& object, Args... args)
		{
			(object.*func)(args...);
		}
	};
}

template<typename TBaseFunctor>
class FastMovableFunctor
{
public:
	typedef TBaseFunctor BaseFunctor;

private:
	typedef std::conditional_t<(std::is_trivially_destructible_v<BaseFunctor>
		&& std::is_trivially_move_constructible_v<BaseFunctor>
		&& sizeof(BaseFunctor) <= internal::UIntConst::maxFastFunctorSize),
		BaseFunctor, BaseFunctor&&> BaseFunctorReference;

public:
	explicit FastMovableFunctor(BaseFunctor&& baseFunctor) noexcept
		: mBaseFunctor(std::forward<BaseFunctor>(baseFunctor))
	{
	}

	FastMovableFunctor(FastMovableFunctor&&) noexcept = default;

	FastMovableFunctor(const FastMovableFunctor&) = delete;

	~FastMovableFunctor() noexcept = default;

	FastMovableFunctor& operator=(const FastMovableFunctor&) = delete;

	template<typename... Args>
	decltype(auto) operator()(Args&&... args) &&
		noexcept(noexcept(std::forward<BaseFunctor>(mBaseFunctor)(std::forward<Args>(args)...)))
	{
		return std::forward<BaseFunctor>(mBaseFunctor)(std::forward<Args>(args)...);
	}

private:
	MOMO_NO_UNIQUE_ADDRESS BaseFunctorReference mBaseFunctor;
};

template<typename TBaseFunctor>
class FastMovableFunctor<FastMovableFunctor<TBaseFunctor>>
{
	static_assert(false);	//?
};

template<typename BaseFunctor>
FastMovableFunctor(BaseFunctor&) -> FastMovableFunctor<BaseFunctor&>;

template<typename TBaseFunctor>
class FastCopyableFunctor
{
public:
	typedef TBaseFunctor BaseFunctor;

private:
	typedef std::conditional_t<
		internal::conceptSmallAndTriviallyCopyable<BaseFunctor, internal::UIntConst::maxFastFunctorSize>,	//?
		BaseFunctor, const BaseFunctor&> BaseFunctorReference;

public:
	explicit FastCopyableFunctor(const BaseFunctor& baseFunctor) noexcept
		: mBaseFunctor(baseFunctor)
	{
	}

	FastCopyableFunctor(FastCopyableFunctor&&) noexcept = default;

	FastCopyableFunctor(const FastCopyableFunctor&) noexcept = default;

	~FastCopyableFunctor() noexcept = default;

	FastCopyableFunctor& operator=(const FastCopyableFunctor&) = delete;

	template<typename... Args>
	decltype(auto) operator()(Args&&... args) const
		noexcept(noexcept(mBaseFunctor(std::forward<Args>(args)...)))
	{
		return mBaseFunctor(std::forward<Args>(args)...);
	}

private:
	MOMO_NO_UNIQUE_ADDRESS BaseFunctorReference mBaseFunctor;
};

template<typename TBaseFunctor>
class FastCopyableFunctor<FastCopyableFunctor<TBaseFunctor>>
{
	static_assert(false);	//?
};

template<typename TBaseFunctor>
class FastMovableFunctor<FastCopyableFunctor<TBaseFunctor>>
	: public FastCopyableFunctor<TBaseFunctor>
{
public:
	typedef FastCopyableFunctor<TBaseFunctor> BaseFunctor;

public:
	explicit FastMovableFunctor(BaseFunctor&& baseFunctor) noexcept
		: BaseFunctor(std::move(baseFunctor))
	{
	}

	FastMovableFunctor(FastMovableFunctor&&) noexcept = default;

	FastMovableFunctor(const FastMovableFunctor&) = delete;

	~FastMovableFunctor() noexcept = default;

	FastMovableFunctor& operator=(const FastMovableFunctor&) = delete;
};

} // namespace momo
