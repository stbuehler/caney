#include "caney/streams/sigbus.hpp"

#include <csignal>
#include <cstring>
#include <mutex>

__CANEY_STREAMSV1_BEGIN

namespace {
	thread_local impl::sigbus_handler_thread_state t_sigbus_handler_state;

	void sigbus_func(int /* signal */) {
		if (!t_sigbus_handler_state.m_active) std::abort();
		std::longjmp(t_sigbus_handler_state.m_sigbus_jmp_buf, t_sigbus_handler_state.m_result);
	}

	void register_sigbus() {
		static std::once_flag register_sigbus;

		std::call_once(register_sigbus, []() { std::signal(SIGBUS, &sigbus_func); });
	}
} // anonymous namespace

sigbus_handler::sigbus_handler() : m_state(t_sigbus_handler_state) {
	// make sure this is the only instance in this thread:
	if (m_state.m_in_use) std::abort();
	m_state.m_in_use = true;
	register_sigbus();
}

sigbus_handler::~sigbus_handler() {
	disable();
	memset(&m_state.m_sigbus_jmp_buf, 0, sizeof(m_state.m_sigbus_jmp_buf));
	m_state.m_in_use = false;
}

__CANEY_STREAMSV1_END
