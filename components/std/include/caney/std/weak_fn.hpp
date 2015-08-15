#pragma once

#include <memory>

/* Usage:
 *   weak_fn(function [, smartpointer] [, policy])
 *   - returns a (const-)callable object, which only calls functions if the smartpointer still
 *     locks at the time of the call (it stores a weak_ptr internally)
 *   - function can be a pointer to a member function of a class or a free
 *     function/lambda, which needs to take a shared_ptr as first argument
 *   - the smartpointer can either be a shared_ptr or a weak_ptr, and must match
 *     either the class the member function belongs to or the first argument of the
 *     free function
 *   - the policy is a function objects, and must either take no arguments or
 *     the same as the function (minus the shared_pointer if the function is not
 *     a member function pointer)
 *   - the policy is called to determine the result if the internally stored weak_ptr
 *     doesn't lock anymore when the weak_fn is called
 *   - if the function doesn't return void, the policy is mandatory
 *   - if no smartpointer is passed, the returned callable expect a weak_ptr (not a
 *     shared_ptr!) as additional first argument
 *     Also in this case weak_fn will default to std::shared_ptr; if you want to work
 *     with a different smart pointer type, pass the shared_ptr type as first template
 *     parameter: weak_fn<boost::shared_ptr>(&some_function)
 *   - all other arguments are simply passed through
 *   - supports std::shared_ptr/std::weak_ptr by default; for boost smart_ptr include
 *     <caney/std/weak_fn_boost.hpp>
 *
 * Using this function an object can register callbacks to itself without keeping itself alive forever
 */

namespace caney {
	inline namespace stdv1 {
		namespace weakfn_traits {
			// support std::shared_ptr/weak_ptr by default. others could be added.
			template<typename Ptr>
			struct shared_ptr_traits;

			template<typename Ptr>
			struct weak_ptr_traits;

			template<typename Object>
			struct shared_ptr_traits<std::shared_ptr<Object>> {
				template<typename T>
				using shared_ptr = std::shared_ptr<T>;

				template<typename T>
				using weak_ptr = std::weak_ptr<T>;

				using object_t = Object;

				using pointer_t = shared_ptr<object_t>;
				using weak_pointer_t = weak_ptr<object_t>;
			};

			template<typename Object>
			struct weak_ptr_traits<std::weak_ptr<Object>> : shared_ptr_traits<std::shared_ptr<Object>> {
			};
		} // namespace weakfn_traits

		namespace impl {
			template<typename Unused>
			struct static_assert_fail {
				static constexpr bool value = false;
			};

			template<typename... Args>
			struct args_container { };

			template<typename Ptr>
			using shared_ptr_traits = weakfn_traits::shared_ptr_traits<typename std::decay<Ptr>::type>;

			template<typename Ptr>
			using weak_ptr_traits = weakfn_traits::weak_ptr_traits<typename std::decay<Ptr>::type>;

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

			template<template<typename T> class /* SharedPtr */, typename Functor, typename Call = decltype(&Functor::operator())>
			auto make_shared_fn(Functor const& func)
					-> decltype(make_functor_fn(func, Call{nullptr})) {
				return make_functor_fn(func, Call{nullptr});
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
	}
} // namespace caney
