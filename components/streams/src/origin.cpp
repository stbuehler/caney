#include "caney/streams/origin.hpp"

__CANEY_STREAMSV1_BEGIN

namespace impl {
	class origin_pause_watcher {
	public:
		origin_pause_watcher(origin* origin) : m_origin(origin) {}
		origin_pause_watcher(origin_pause_watcher const&) = delete;
		origin_pause_watcher& operator=(origin_pause_watcher const&) = delete;

		~origin_pause_watcher() {
			if (nullptr != m_origin) m_origin->resume();
		}

		origin* m_origin = nullptr;
	};
} // namespace impl

origin::~origin() {
	std::shared_ptr<impl::origin_pause_watcher> p = m_weak_pause.lock();
	if (p) p->m_origin = nullptr;
}

origin_pause origin::pause() {
	std::shared_ptr<impl::origin_pause_watcher> p = m_weak_pause.lock();
	if (!p) p = std::make_shared<impl::origin_pause_watcher>(this);
	return origin_pause(std::move(p));
}

bool origin::is_paused() const {
	return (bool) !m_weak_pause.expired();
}

void origin::resume() {
	m_weak_pause.reset();
	on_resume();
}

origin_pause::operator bool() {
	return (bool) m_watcher;
}

void origin_pause::reset() {
	m_watcher.reset();
}

__CANEY_STREAMSV1_END
