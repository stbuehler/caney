#pragma once

#include <memory>

#include <csetjmp>

/* USAGE:
 *
 * {
 *     sigbus_handler_ptr sigbus_handle = make_sigbus_handler();
 *     if (setjmp(sigbus_handle->get_jmp_buf() > 0) { ... handle sigbus ... }
 *
 *     sigbus_handle->enable();
 *     // NOW sigbus handling is active
 * }
 *
 * Note: SIGBUS will trigger a longjmp back to the point where setjmp was called.
 * NO stack unwinding! - just a simple goto with "C" semantics.
 */

namespace caney {
	namespace streams {
		inline namespace v1 {
			class sigbus_handler {
			public:
				// the 0 result is what setjmp() returns on the first call - use something else to distinguish
				void enable(int result = 1) {
					m_result = result;
					m_active = true;
				}

				void disable() {
					m_active = false;
					m_result = 0;
				}

				explicit operator bool() const {
					return m_active;
				}

				int get_result() const {
					return m_result;
				}

				std::jmp_buf& get_jmp_buf() {
					return m_sigbus_jmp_buf;
				}

			private:
				std::jmp_buf m_sigbus_jmp_buf;
				volatile int m_result = 0;
				volatile bool m_active = false;
			};

			class sigbus_handler_disabler {
			public:
				void operator()(sigbus_handler* handler);
			};

			using sigbus_handler_ptr = std::unique_ptr<sigbus_handler, sigbus_handler_disabler>;

			sigbus_handler_ptr make_sigbus_handler();
		}
	} // namespace streams
} // namespace caney
