#pragma once

#include <memory>
#include <mutex>

namespace caney {
	namespace util {
		inline namespace v1 {
			/* create a shared default-constructed T on demand, but doesn't keep it alive */
			template<typename T>
			class singleton {
			public:
				using value_type = T;

				constexpr singleton() = default;
				std::shared_ptr<value_type> get() {
					std::lock_guard<std::mutex> lock(m_mutex);
					std::shared_ptr<value_type> result = m_ref.lock();
					if (!result) {
						m_ref = result = std::make_shared<value_type>();
					}
					return result;
				}

			private:
				std::mutex m_mutex;
				std::weak_ptr<value_type> m_ref;
			};
		}
	} // namespace util
} // namespace caney
