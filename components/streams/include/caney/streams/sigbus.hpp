#pragma once

#include "internal.hpp"

#include <cassert>
#include <csetjmp>
#include <memory>

#include <boost/noncopyable.hpp>

__CANEY_STREAMSV1_BEGIN

/**
 * @defgroup sigbus_handler SIGBUS handler
 *
 * USAGE:
 *
 *     {
 *         sigbus_handler_ptr sigbus_handle = make_sigbus_handler();
 *         if (setjmp(sigbus_handle.get_jmp_buf()) > 0) { ... handle sigbus ... }
 *
 *         sigbus_handle.enable();
 *         // NOW sigbus handling is active
 *     }
 *
 * Note: SIGBUS will trigger a longjmp back to the point where setjmp was called.
 * NO stack unwinding! - just a simple goto with "C" semantics.
 * That is why you have to call ``setjmp`` in your own code - wrapping it in a function
 * would break it.
 *
 * The first time a @ref sigbus_handler gets created it will register
 * the signal handler for SIGBUS.
 *
 * @addtogroup sigbus_handler
 * @{
 */

namespace impl {
	struct sigbus_handler_thread_state {
		std::jmp_buf m_sigbus_jmp_buf;
		volatile int m_result = 0;
		volatile bool m_active = false;
		bool m_in_use = false;
	};
} // namespace impl

/**
 * @brief class to wrap SIGBUS handling
 *
 *
 * There must be at most one @ref sigbus_handler object per thread.
 */
class sigbus_handler : private boost::noncopyable {
public:
	/**
	 * @brief construct sigbus_handler
	 *
	 * The first time a @ref sigbus_handler object gets created,
	 * this constructor registers the SIGBUS signal handler
	 */
	explicit sigbus_handler();
	~sigbus_handler();

	/**
	 * @brief enable SIGBUS handling (by default it is disabled).
	 * @param result the value ``setjmp()`` will return on failure. don't pass 0.
	 */
	void enable(int result = 1) {
		assert(0 != result);
		m_state.m_result = result;
		m_state.m_active = true;
	}

	/**
	 * @brief disable SIGBUS handling (by default it is disabled).
	 * a SIGBUS will trigger a ``std::abort()`` while disabled.
	 */
	void disable() {
		m_state.m_active = false;
		m_state.m_result = 0;
	}

	/** @brief whether the handler is enabled */
	explicit operator bool() const {
		return m_state.m_active;
	}

	/** @brief result set by last call to @ref enable() */
	int get_result() const {
		return m_state.m_result;
	}

	/** @brief ``std::jmp_buf`` to use with ``setjmp()`` */
	std::jmp_buf& get_jmp_buf() {
		return m_state.m_sigbus_jmp_buf;
	}

private:
	impl::sigbus_handler_thread_state& m_state;
};

//! @}

__CANEY_STREAMSV1_END
