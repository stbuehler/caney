/** @file */

#pragma once

#include "internal.hpp"
#include "smart_ptr_traits.hpp"

#include <memory>

__CANEY_STDV1_BEGIN

/**
 * @defgroup weak_fn weak functions
 *
 * `weak_fn(function [, smartpointer] [, policy])`
 *   - returns a (const-)callable object, which only calls functions if the smartpointer still
 *     locks at the time of the call (it stores a weak reference internally)
 *   - function can be a pointer to a member function of a class or a free
 *     function/lambda, which needs to take a strong reference as first argument
 *   - the smartpointer can either be a strong or a weak reference, and must match
 *     either the class the member function belongs to or the first argument of the
 *     free function/lambda
 *   - the policy is a function objects, and must either take no arguments or
 *     the same as the wrapped function (minus the first reference if the function
 *     is not a member function pointer)
 *   - the policy is called to determine the result if the internally stored or passed
 *     weak reference doesn't lock anymore when the @ref weak_fn() result is called
 *   - if the function doesn't return void, the policy is mandatory
 *   - if no `smartpointer` is passed, the returned callable expects a weak reference
 *     (not a shared reference!) as additional first argument. <br>
 *     Also in this case @ref weak_fn() will default to `std::shared_ptr` as strong reference type;
 *     if you want to work with a different smart pointer type, pass the strong reference
 *     template type as first template parameter: `weak_fn<boost::shared_ptr>(&some_function)`
 *   - all other arguments are simply passed through
 *   - supports `std::shared_ptr`/`std::weak_ptr` by default; for `boost::shared_ptr` include
 *     @ref caney/std/smart_ptr_traits_boost.hpp
 *
 * Using this function an object can register callbacks to itself without keeping itself alive forever
 *
 * @addtogroup weak_fn
 * @{
 */

namespace impl {
	template<typename Unused>
	struct static_assert_fail {
		static constexpr bool value = false;
	};

	template<typename... Args>
	struct args_container { };

	template<typename Ptr>
	using shared_ptr_traits = smart_ptr_traits::shared_ptr_traits<typename std::decay<Ptr>::type>;

	template<typename Ptr>
	using weak_ptr_traits = smart_ptr_traits::weak_ptr_traits<typename std::decay<Ptr>::type>;

	template<typename SharedPtrTraits, typename Result, typename Function, typename... Args>
	class shared_free_fn {
	private:
		using pointer_t = typename SharedPtrTraits::pointer_t;

	public:
		using result_t = Result;
		using shared_ptr_traits = SharedPtrTraits;
		using arguments_t = args_container<Args...>;
		static constexpr arguments_t* arguments = nullptr;

		explicit shared_free_fn(Function function)
		: m_function(function) {
		}

		Result operator()(pointer_t const& obj, Args... args) const {
			return m_function(obj, std::forward<Args>(args)...);
		}

	private:
		Function m_function;
	};

	template<template<typename T> class /* SharedPtr */, typename Result, typename Pointer, typename... Args>
	auto make_shared_fn(Result (*func)(Pointer, Args... args))
			-> shared_free_fn<shared_ptr_traits<Pointer>, Result, decltype(func), Args...> {
		return shared_free_fn<shared_ptr_traits<Pointer>, Result, decltype(func), Args...>(func);
	}

	template<typename SharedPtrTraits, typename Result, typename Functor, typename... Args>
	class shared_functor_fn : private Functor {
	private:
		using pointer_t = typename SharedPtrTraits::pointer_t;

	public:
		using result_t = Result;
		using shared_ptr_traits = SharedPtrTraits;
		using arguments_t = args_container<Args...>;
		static constexpr arguments_t* arguments = nullptr;

		explicit shared_functor_fn(Functor const& functor)
		: Functor(functor) {
		}

		Result operator()(pointer_t const& obj, Args... args) const {
			Functor const& const_self = *this;
			Functor& self = const_cast<Functor&>(const_self);
			return self(obj, std::forward<Args>(args)...);
		}
	};

	template<typename Functor, typename Result, typename Pointer, typename... Args>
	auto make_functor_fn(Functor const& func, Result (Functor::*)(Pointer, Args... args) const)
			-> shared_functor_fn<shared_ptr_traits<Pointer>, Result, Functor, Args...> {
		return shared_functor_fn<shared_ptr_traits<Pointer>, Result, Functor, Args...>(func);
	}

	template<typename Functor, typename Result, typename Pointer, typename... Args>
	auto make_functor_fn(Functor const& func, Result (Functor::*)(Pointer, Args... args))
			-> shared_functor_fn<shared_ptr_traits<Pointer>, Result, Functor, Args...> {
		return shared_functor_fn<shared_ptr_traits<Pointer>, Result, Functor, Args...>(func);
	}

	template<template<typename T> class /* SharedPtr */, typename Functor>
	auto make_shared_fn(Functor const& func)
			-> decltype(make_functor_fn(func, &Functor::operator())) {
		return make_functor_fn(func, &Functor::operator());
	}

	template<typename SharedPtrTraits, typename Result, typename Function, typename... Args>
	class shared_member_fn {
	private:
		using pointer_t = typename SharedPtrTraits::pointer_t;

	public:
		using result_t = Result;
		using shared_ptr_traits = SharedPtrTraits;
		using arguments_t = args_container<Args...>;
		static constexpr arguments_t* arguments = nullptr;

		explicit shared_member_fn(Function function)
		: m_function(function) {
		}

		Result operator()(pointer_t const& obj, Args... args) const {
			return (*obj.*m_function)(std::forward<Args>(args)...);
		}

	private:
		Function m_function;
	};

	// normal member function, default to std::shared_ptr
	template<template<typename T> class SharedPtr, typename Result, typename Object, typename... Args>
	auto make_shared_fn(Result (Object::*func)(Args... args))
			-> shared_member_fn<shared_ptr_traits<SharedPtr<Object>>, Result, decltype(func), Args...> {
		return shared_member_fn<shared_ptr_traits<SharedPtr<Object>>, Result, decltype(func), Args...>(func);
	}

	// const member function, default to std::shared_ptr
	template<template<typename T> class SharedPtr, typename Result, typename Object, typename... Args>
	auto make_shared_fn(Result (Object::*func)(Args... args) const)
			-> shared_member_fn<shared_ptr_traits<SharedPtr<Object const>>, Result, decltype(func), Args...> {
		return shared_member_fn<shared_ptr_traits<SharedPtr<Object const>>, Result, decltype(func), Args...>(func);
	}

	template<typename Function, typename Pointer,
		typename shared_ptr_traits<Pointer>::object_t* = nullptr>
	auto make_shared_fn_from_ptr(Function&& function, Pointer&& pointer)
			-> decltype(make_shared_fn<shared_ptr_traits<Pointer>::template shared_ptr>(std::forward<Function>(function))) {
		return make_shared_fn<shared_ptr_traits<Pointer>::template shared_ptr>(std::forward<Function>(function));
	}

	template<typename Function, typename Pointer,
		typename weak_ptr_traits<Pointer>::object_t* = nullptr>
	auto make_shared_fn_from_ptr(Function&& function, Pointer&& pointer)
			-> decltype(make_shared_fn<weak_ptr_traits<Pointer>::template shared_ptr>(std::forward<Function>(function))) {
		return make_shared_fn<weak_ptr_traits<Pointer>::template shared_ptr>(std::forward<Function>(function));
	}

	template<typename Policy, typename Arg1, typename... Args>
	auto call_policy(Policy& p, Arg1 arg1, Args... args)
			-> decltype(p(std::forward<Arg1>(arg1), std::forward<Args>(args)...)) {
		return p(std::forward<Arg1>(arg1), std::forward<Args>(args)...);
	}

	template<typename Policy, typename... Args>
	auto call_policy(Policy& p, Args...)
			-> decltype(p()) {
		return p();
	}

	template<typename Policy, typename... Args>
	using call_policy_result = decltype(call_policy(std::declval<Policy&>(), std::declval<Args>()...));

	struct default_void_policy {
		void operator()() const {
		}
	};

	template<typename SharedFn, typename Policy, typename... Args>
	class weak_fn_needs_ptr : private SharedFn {
	private:
		using pointer_t = typename SharedFn::shared_ptr_traits::pointer_t;
		using weak_pointer_t = typename SharedFn::shared_ptr_traits::weak_pointer_t;
		using policy_t = typename std::decay<Policy>::type;

	public:
		using result_t = typename SharedFn::result_t;
		using arguments_t = args_container<Args...>;

		explicit weak_fn_needs_ptr(SharedFn&& shared_fn, Policy policy)
		: SharedFn(std::move(shared_fn)), m_policy(std::forward<Policy>(policy)) {
		}

		// Must not call with/bind to shared_ptr !
#if 0
		// show error as static_assert
		template<typename Object, typename... InnerArgs>
		void operator()(typename SharedFn::shared_ptr_traits::template shared_ptr<Object> const&, InnerArgs&&...) const {
			static_assert(static_assert_fail<Object>::value, "Must not call with/bind to shared_ptr");
		}
#else
		// delete the function
		template<typename Object, typename... InnerArgs>
		void operator()(typename SharedFn::shared_ptr_traits::template shared_ptr<Object> const&, InnerArgs&&...) const = delete;
#endif

		result_t operator()(weak_pointer_t const& weak_ptr, Args... args) const {
			pointer_t ptr = weak_ptr.lock();
			if (ptr) {
				return SharedFn::operator()(ptr, std::forward<Args>(args)...);
			} else {
				static_assert(std::is_convertible<call_policy_result<policy_t, Args...>, result_t>::value,
					"Policy result type doesn't mach. Perhaps you forgot to specify a policy?");
				return call_policy(m_policy, std::forward<Args>(args)...);
			}
		}

	private:
		policy_t m_policy;
	};

	template<typename SharedFn, typename Policy, typename... Args>
	class weak_fn_has_ptr : private SharedFn {
	private:
		using pointer_t = typename SharedFn::shared_ptr_traits::pointer_t;
		using weak_pointer_t = typename SharedFn::shared_ptr_traits::weak_pointer_t;
		using policy_t = typename std::decay<Policy>::type;

	public:
		using result_t = typename SharedFn::result_t;
		using arguments_t = args_container<Args...>;

		explicit weak_fn_has_ptr(SharedFn&& shared_fn, weak_pointer_t weak_ptr, Policy policy)
		: SharedFn(std::move(shared_fn)), m_weak_ptr(weak_ptr), m_policy(std::forward<Policy>(policy)) {
		}

		result_t operator()(Args... args) const {
			pointer_t ptr = m_weak_ptr.lock();
			if (ptr) {
				return SharedFn::operator()(ptr, std::forward<Args>(args)...);
			} else {
				static_assert(std::is_convertible<call_policy_result<policy_t, Args...>, result_t>::value,
					"Policy result type doesn't mach. Perhaps you forgot to specify a policy?");
				return call_policy(m_policy, std::forward<Args>(args)...);
			}
		}

	private:
		weak_pointer_t m_weak_ptr;
		policy_t m_policy;
	};

	/* unpack Args... from SharedFn for weak_fn_needs_ptr */
	template<typename SharedFn, typename Policy, typename... Args>
	weak_fn_needs_ptr<SharedFn, Policy, Args...> make_weak_fn_needs_ptr(SharedFn&& shared_fn, Policy&& policy, args_container<Args...>*) {
		return weak_fn_needs_ptr<SharedFn, Policy, Args...>(std::move(shared_fn), std::forward<Policy>(policy));
	}

	template<typename SharedFn, typename Policy>
	auto make_weak_fn(SharedFn&& shared_fn, Policy&& policy)
			-> decltype(make_weak_fn_needs_ptr(std::move(shared_fn), std::forward<Policy>(policy), SharedFn::arguments)) {
		return make_weak_fn_needs_ptr(std::move(shared_fn), std::forward<Policy>(policy), SharedFn::arguments);
	}

	/* unpack Args... from SharedFn for weak_fn_has_ptr */
	template<typename SharedFn, typename Pointer, typename Policy, typename... Args>
	weak_fn_has_ptr<SharedFn, Policy, Args...> make_weak_fn_has_ptr(SharedFn&& shared_fn, Pointer&& ptr, Policy&& policy, args_container<Args...>*) {
		return weak_fn_has_ptr<SharedFn, Policy, Args...>(std::move(shared_fn), std::forward<Pointer>(ptr), std::forward<Policy>(policy));
	}

	template<typename SharedFn, typename Pointer, typename Policy>
	auto make_weak_fn(SharedFn&& shared_fn, Pointer&& ptr, Policy&& policy)
			-> decltype(make_weak_fn_has_ptr(std::move(shared_fn), std::forward<Pointer>(ptr), std::forward<Policy>(policy), SharedFn::arguments)) {
		return make_weak_fn_has_ptr(std::move(shared_fn), std::forward<Pointer>(ptr), std::forward<Policy>(policy), SharedFn::arguments);
	}
} // namespace impl

/**
 * @brief bind some function to take a weak reference as first argument
 * @param function   function to wrap
 * @tparam SharedPtr strong reference type to use; defaults to std::shared_ptr. Does not automatically detect
 *                   the smart pointer type of the first argument in free functions.
 *
 * The functions needs to return void (i.e. nothing). When the returned callable is called
 * the weak reference gets locked. If the resulting strong reference is empty, nothing happens.
 * Otherwise:
 * - if the given function is a member function it will be called using the strong reference
 *   as object to call it on, and the remaining paramaters are passed through.
 * - otherwise the function will be given the strong reference as first argument,
 *   and the other arguments will be appended.
 */
template<template<typename T> class SharedPtr = std::shared_ptr, typename Function>
auto weak_fn(Function&& function)
		-> decltype(
			impl::make_weak_fn(
				impl::make_shared_fn<SharedPtr>(function),
				impl::default_void_policy())) {
	return impl::make_weak_fn(
		impl::make_shared_fn<SharedPtr>(function),
		impl::default_void_policy());
}

/**
 * @brief bind some function to take a weak reference as first argument
 * @param function   function to wrap
 * @param policy     policy to call when locking the weak reference fails.
 * @tparam SharedPtr strong reference type to use; defaults to std::shared_ptr. Does not automatically detect
 *                   the smart pointer type of the first argument in free functions.
 *
 * When the returned callable is called the weak reference gets locked. If the resulting
 * strong reference is empty the policy gets called and its return value returned.
 * Otherwise:
 * - if the given function is a member function it will be called using the strong reference
 *   as object to call it on, and the remaining paramaters are passed through.
 * - otherwise the function will be given the strong reference as first argument,
 *   and the other arguments will be appended.
 *
 * The policy gets either called with all remaining arguments (excluding the reference) or with none.
 */
template<template<typename T> class SharedPtr = std::shared_ptr, typename Function, typename Policy,
	impl::call_policy_result<Policy>* = nullptr>
auto weak_fn(Function&& function, Policy&& policy)
		-> decltype(
			impl::make_weak_fn(
				impl::make_shared_fn<SharedPtr>(function),
				std::forward<Policy>(policy))) {
	return impl::make_weak_fn(
		impl::make_shared_fn<SharedPtr>(function),
		std::forward<Policy>(policy));
}

/**
 * @brief bind some function to take a weak reference as first argument
 * @param function   function to wrap
 * @param ptr        strong or weak reference (converted to weak reference internally)
 *
 * The functions needs to return void (i.e. nothing). When the returned callable is called
 * the (internally kept as weak) reference gets locked. If the resulting strong reference
 * is empty the policy gets called and its return value returned.
 * Otherwise:
 * - if the given function is a member function it will be called using the strong reference
 *   as object to call it on, and the remaining paramaters are passed through.
 * - otherwise the function will be given the strong reference as first argument,
 *   and the other arguments will be appended.
 *
 * The policy gets either called with all remaining arguments (excluding the reference) or with none.
 */
template<typename Function, typename Pointer>
auto weak_fn(Function&& function, Pointer&& ptr)
		-> decltype(
			impl::make_weak_fn(
				impl::make_shared_fn_from_ptr(std::forward<Function>(function), ptr),
				std::forward<Pointer>(ptr),
				impl::default_void_policy())) {
	return impl::make_weak_fn(
		impl::make_shared_fn_from_ptr(std::forward<Function>(function), ptr),
		std::forward<Pointer>(ptr),
		impl::default_void_policy());
}

/**
 * @brief bind some function to take a weak reference as first argument
 * @param function   function to wrap
 * @param ptr        strong or weak reference (converted to weak reference internally)
 * @param policy     policy to call when locking the weak reference fails.
 *
 * When the returned callable is called the (internally kept as weak) reference gets locked.
 * If the resulting strong reference is empty the policy gets called and its return value returned.
 * Otherwise:
 * - if the given function is a member function it will be called using the strong reference
 *   as object to call it on, and the remaining paramaters are passed through.
 * - otherwise the function will be given the strong reference as first argument,
 *   and the other arguments will be appended.
 *
 * The policy gets either called with all remaining arguments (excluding the reference) or with none.
 */
template<typename Function, typename Pointer, typename Policy,
	impl::call_policy_result<Policy>* = nullptr>
auto weak_fn(Function&& function, Pointer&& ptr, Policy&& policy)
		-> decltype(
			impl::make_weak_fn(
				impl::make_shared_fn_from_ptr(std::forward<Function>(function), ptr),
				std::forward<Pointer>(ptr),
				std::forward<Policy>(policy))) {
	return impl::make_weak_fn(
		impl::make_shared_fn_from_ptr(std::forward<Function>(function), ptr),
		std::forward<Pointer>(ptr),
		std::forward<Policy>(policy));
}

/** @} */

__CANEY_STDV1_END
