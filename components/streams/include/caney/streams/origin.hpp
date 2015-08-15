#pragma once

#include "caney/std/object.hpp"

#include <memory>

namespace caney {
	namespace streams {
		inline namespace v1 {
			class origin;
			class origin_pause;

			// the origin receives the pause/resume notifications
			class origin : private caney::object {
			public:
				origin() = default;
				~origin();

				origin_pause pause();

				bool is_paused() const;

			protected:
				virtual void on_pause() = 0;
				virtual void on_resume() = 0;

			private:
				friend class origin_pause;
				class pause_watcher;

				void resume();

				std::weak_ptr<pause_watcher> m_weak_pause;
			};

			class origin_pause {
			public:
				origin_pause() = default;

				explicit operator bool();

				void reset();

			private:
				friend class origin;

				origin_pause(std::shared_ptr<origin::pause_watcher> watcher)
				: m_watcher(std::move(watcher)) {
				}

				std::shared_ptr<origin::pause_watcher> m_watcher;
			};
		}
	} // namespace streams
} // namespace caney
