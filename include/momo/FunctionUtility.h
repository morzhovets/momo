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

	template<typename TFunctor>
	class Finalizer;

	template<conceptExecutor TFunctor>
	requires (std::is_nothrow_destructible_v<TFunctor> &&
		std::is_nothrow_move_constructible_v<TFunctor> &&
		!std::is_reference_v<TFunctor> /*&& std::is_nothrow_invocable_v<TFunctor>*/)
	class Finalizer<TFunctor>
	{
	public:
		typedef TFunctor Functor;

	public:
		[[nodiscard]] Finalizer(Functor func) noexcept
			: mFunctor(std::move(func)),
			mIsEmpty(false)
		{
		}

		Finalizer(const Finalizer&) = delete;

		~Finalizer() noexcept
		{
			if (!mIsEmpty)
				std::move(mFunctor)();
		}

		Finalizer& operator=(const Finalizer&) = delete;

		explicit operator bool() const noexcept
		{
			return !mIsEmpty;
		}

		void Detach() noexcept
		{
			mIsEmpty = true;
		}

	private:
		Functor mFunctor;
		bool mIsEmpty;
	};

	template<conceptExecutor Functor>
	Finalizer(Functor)
		-> Finalizer<Functor>;

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
		template<typename Function>
		void ptExecute(Function func) noexcept
		{
			std::apply(func, mArgs);
		}

	private:
		std::tuple<Args...> mArgs;
	};

	template<typename Arg>
	class FinalizerArgs<Arg>
	{
	protected:
		void ptExecute(void (*func)(Arg) noexcept) noexcept
		{
			func(arg);
		}

		template<typename Class = std::decay_t<Arg>>
		requires std::is_class_v<Class>
		void ptExecute(void (Class::*func)() noexcept) noexcept
		{
			(arg.*func)();
		}

	public:
		Arg arg;
	};

	template<typename Arg0, typename Arg1>
	class FinalizerArgs<Arg0, Arg1>
	{
	protected:
		void ptExecute(void (*func)(Arg0, Arg1) noexcept) noexcept
		{
			func(arg0, arg1);
		}

		template<typename Class = std::decay_t<Arg0>>
		requires std::is_class_v<Class>
		void ptExecute(void (Class::*func)(Arg1) noexcept) noexcept
		{
			(arg0.*func)(arg1);
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

		template<typename Class = std::decay_t<Arg0>>
		requires std::is_class_v<Class>
		void ptExecute(void (Class::*func)(Arg1, Arg2) noexcept) noexcept
		{
			(arg0.*func)(arg1, arg2);
		}

	public:
		Arg0 arg0;
		Arg1 arg1;
		Arg2 arg2;
	};

	template<typename... Args>
	class Finalizer<void (*)(Args...) noexcept>
		: private FinalizerArgs<Args...>
	{
	public:
		typedef void (*Functor)(Args...) noexcept;

	private:
		typedef internal::FinalizerArgs<Args...> FinalizerArgs;

	public:
		[[nodiscard]] explicit Finalizer(Functor func, Args... args) noexcept
			: FinalizerArgs(args...),
			mFunctor(func)
		{
		}

		Finalizer(const Finalizer&) = delete;

		~Finalizer() noexcept
		{
			if (mFunctor != nullptr)
				FinalizerArgs::ptExecute(mFunctor);
		}

		Finalizer& operator=(const Finalizer&) = delete;

		explicit operator bool() const noexcept
		{
			return mFunctor != nullptr;
		}

		void Detach() noexcept
		{
			mFunctor = nullptr;
		}

	private:
		Functor mFunctor;
	};

	template<typename... Args>
	Finalizer(void (*)(Args...) noexcept, std::type_identity_t<Args>...)
		-> Finalizer<void (*)(Args...) noexcept>;

	template<typename Class, typename... Args>
	class Finalizer<void (Class::*)(Args...) noexcept>
		: private FinalizerArgs<Class&, Args...>
	{
	public:
		typedef void (Class::*Functor)(Args...) noexcept;

	private:
		typedef internal::FinalizerArgs<Class&, Args...> FinalizerArgs;

	public:
		[[nodiscard]] explicit Finalizer(Functor func, Class& object, Args... args) noexcept
			: FinalizerArgs(object, args...),
			mFunctor(func)
		{
		}

		Finalizer(const Finalizer&) = delete;

		~Finalizer() noexcept
		{
			if (mFunctor != nullptr)
				FinalizerArgs::ptExecute(mFunctor);
		}

		Finalizer& operator=(const Finalizer&) = delete;

		explicit operator bool() const noexcept
		{
			return mFunctor != nullptr;
		}

		void Detach() noexcept
		{
			mFunctor = nullptr;
		}

	private:
		Functor mFunctor;
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
		template<typename... Args, std::invocable<Args&&...> Func>
		static bool CatchAll(Func&& func, Args&&... args) noexcept
		{
#if !defined(MOMO_DISABLE_EXCEPTIONS) && !defined(MOMO_CATCH_ALL)
			static_assert(false);
#endif
			bool res = false;
#if !defined(MOMO_DISABLE_EXCEPTIONS) && defined(MOMO_CATCH_ALL)
			try
#endif
			{
				if constexpr (std::is_member_pointer_v<std::decay_t<Func>>)
					pvInvoke(std::forward<Func>(func), std::forward<Args>(args)...);
				else
					std::forward<Func>(func)(std::forward<Args>(args)...);
				res = true;
			}
#if !defined(MOMO_DISABLE_EXCEPTIONS) && defined(MOMO_CATCH_ALL)
			MOMO_CATCH_ALL
#endif
			return res;
		}

	private:
		template<typename Class, typename... Args, std::invocable<Class&&, Args&&...> Func>
		static void pvInvoke(Func&& func, Class&& object, Args&&... args)
		{
			(std::forward<Class>(object).*func)(std::forward<Args>(args)...);
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
