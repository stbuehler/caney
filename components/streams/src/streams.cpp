#include "caney/streams/streams.hpp"

namespace caney {
	namespace streams {
		inline namespace v1 {
			bool sink_base::is_paused() const {
				return m_is_paused;
			}

			void sink_base::pause() {
				if (m_is_paused) return;
				m_is_paused = true;
				if (m_origin) {
					m_origin_pause = m_origin->pause();
				}
			}

			void sink_base::resume() {
				if (!m_is_paused) return;
				m_is_paused = false;
				m_origin_pause.reset();
			}

			std::shared_ptr<origin> const& sink_base::get_origin() const {
				return m_origin;
			}

			void sink_base::on_new_origin(std::shared_ptr<origin> const&) {
				/* empty base implementation */
			}

			void sink_base::set_new_origin(std::shared_ptr<origin> new_origin) {
				if (m_origin == new_origin) return;
				m_origin = new_origin;

				if (m_is_paused && new_origin) {
					m_origin_pause = new_origin->pause();
				} else {
					m_origin_pause.reset();
				}

				// check whether pause/reset already triggered a new set_new_origin
				if (m_origin != new_origin) return;

				// on_new_origin takes reference; give reference to local copy to keep it stable
				on_new_origin(new_origin);
			}

			std::weak_ptr<origin> const& source_base::get_origin() const {
				return m_origin;
			}
		}
	} // namespace streams
} // namespace caney
