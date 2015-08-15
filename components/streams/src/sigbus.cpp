#include "caney/streams/sigbus.hpp"

#include <csignal>
#include <cstring>
#include <mutex>

namespace caney {
	namespace streams {
		inline namespace v1 {
			namespace {
				thread_local sigbus_handler t_sigbus_handler;

				void sigbus_func(int /* signal */) {
					if (!t_sigbus_handler) std::abort();
					std::longjmp(t_sigbus_handler.get_jmp_buf(), t_sigbus_handler.get_result());
				}

				void register_sigbus() {
					static std::once_flag register_sigbus;

					std::call_once(register_sigbus, []() {
						std::signal(SIGBUS, &sigbus_func);
					});
				}
			} // anonymous namespace

			sigbus_handler_ptr make_sigbus_handler() {
				register_sigbus();
				return sigbus_handler_ptr(&t_sigbus_handler);
			}

			void sigbus_handler_disabler::operator()(sigbus_handler* handler) {
				handler->disable();
				memset(handler->get_jmp_buf(), 0, sizeof(std::jmp_buf));
			}
		}
	} // namespace streams
} // namespace caney
