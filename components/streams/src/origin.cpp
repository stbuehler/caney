#include "caney/streams/origin.hpp"

namespace caney {
	namespace streams {
		inline namespace v1 {
			class origin::pause_watcher {
			public:
				pause_watcher(origin* origin)
				: m_origin(origin) {
				}
				pause_watcher(pause_watcher const&) = delete;
				pause_watcher& operator=(pause_watcher const&) = delete;

				~pause_watcher() {
					if (nullptr != m_origin) m_origin->resume();
				}

				origin* m_origin = nullptr;
			};

			origin::~origin() {
				std::shared_ptr<pause_watcher> p = m_weak_pause.lock();
				if (p) p->m_origin = nullptr;
			}

			origin_pause origin::pause() {
				std::shared_ptr<pause_watcher> p = m_weak_pause.lock();
				if (!p) p = std::make_shared<pause_watcher>(this);
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
		}
	} // namespace streams
} // namespace caney
