//* @file */

#pragma once

#include "internal.hpp"

#include <memory>
#include <tuple>
#include <type_traits>
#include <utility>

__CANEY_UTILV1_BEGIN

namespace impl {
	/* - calling will pass arguments by rvalue-ref to callable, and the arguments might get destroyed
	 *   so it really should only be called once
	 * - copying the wrapper doesn't create a copy of the arguments, they are shared, so copying
	 *   cannot be used to circumvent the first rule of only calling it once
	 * - this is useful if you want to pass non-copyable but moveable data types (std::unique_ptr, ...)
	 *   to an asynchronous completion handler
	 */
	template <typename Callable, typename... Args>
	class wrapped_call_once {
	public:
		template <typename... ConstrArgs>
		explicit wrapped_call_once(Callable const& callable, ConstrArgs&&... args)
		: m_context(std::make_shared<context>(callable, std::forward<ConstrArgs>(args)...)) {}

		void operator()() const {
			context& ctx{*m_context};
			ctx.m_callable(std::move(std::get<Args>(ctx.m_args))...);
		}

	private:
		struct context {
			Callable m_callable;
			std::tuple<Args...> m_args;

			template <typename... ConstrArgs>
			explicit context(Callable const& callable, ConstrArgs&&... args) : m_callable(callable), m_args(std::forward<ConstrArgs>(args)...) {}
		};
		std::shared_ptr<context> m_context;
	};

	/* when called with some arguments, pass a `void() const` callable to the
	 * dispatchers dispatch() function; the passed callable must only be called once
	 */
	template <typename Dispatcher, typename Callable>
	class wrapped_dispatcher {
	public:
		explicit wrapped_dispatcher(Dispatcher&& dispatcher, Callable&& callable)
		: m_dispatcher(std::forward<Dispatcher>(dispatcher)), m_callable(std::forward<Callable>(callable)) {}

		template <typename... Args>
		void operator()(Args&&... args) const {
			typedef wrapped_call_once<CallableT, std::decay_t<Args>...> CompletionT;
			m_dispatcher.dispatch(CompletionT{m_callable, std::forward<Args>(args)...});
		}

	private:
		typedef std::decay_t<Dispatcher> DispatcherT;
		typedef std::decay_t<Callable> CallableT;

		mutable DispatcherT m_dispatcher;
		CallableT m_callable;
	};

	/* wrap a dispatcher with a post() method and provide it through a dispatch() method */
	template <typename Dispatcher>
	class post_dispatcher {
	public:
		explicit post_dispatcher(Dispatcher&& dispatcher) : m_dispatcher(std::forward<Dispatcher>(dispatcher)) {}

		template <typename Callable>
		void dispatch(Callable&& callable) {
			m_dispatcher.post(std::forward<Callable>(callable));
		}

	private:
		typedef std::decay_t<Dispatcher> DispatcherT;
		DispatcherT m_dispatcher;
	};
} // namespace impl

/**
 * @brief return a new callable which passes all calls to `callable` through `dispatcher` using dispatch() on the dispatcher
 * @param dispatcher dispatcher to use
 * @param callable   callable to wrap
 * @return new callable wrapping calls through `dispatcher.dispatch()`
 */
template <typename Dispatcher, typename Callable>
impl::wrapped_dispatcher<Dispatcher, Callable> wrap_dispatch(Dispatcher&& dispatcher, Callable&& callable) {
	return impl::wrapped_dispatcher<Dispatcher, Callable>(std::forward<Dispatcher>(dispatcher), std::forward<Callable>(callable));
}

/**
 * @brief return a new callable which passes all calls to `callable` through `dispatcher` using post() on the dispatcher
 * @param dispatcher dispatcher to use
 * @param callable   callable to wrap
 * @return new callable wrapping calls through `dispatcher.post()`
 */
template <typename Dispatcher, typename Callable>
impl::wrapped_dispatcher<impl::post_dispatcher<Dispatcher>, Callable> wrap_post(Dispatcher&& dispatcher, Callable&& callable) {
	return impl::wrapped_dispatcher<impl::post_dispatcher<Dispatcher>, Callable>(
		impl::post_dispatcher<Dispatcher>{std::forward<Dispatcher>(dispatcher)}, std::forward<Callable>(callable));
}

__CANEY_UTILV1_END
