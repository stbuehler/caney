#pragma once

#include <mutex>
#include <condition_variable>
#include <list>

#include <boost/noncopyable.hpp>

namespace caney {
	inline namespace stdv1 {
		template<typename T>
		class wqueue : private boost::noncopyable {
		public:
			using value_type = T;

			template<typename... Args>
			void emplace(Args&&... args) {
				{
					std::lock_guard<std::mutex> lock(m_mutex);
					m_queue.emplace_back(std::forward<Args>(args)...);
				}
				m_cond.notify_one();
			}

			value_type wait() {
				std::unique_lock<std::mutex> lock(m_mutex);
				while (m_queue.empty()) m_cond.wait(lock);
				value_type elem = m_queue.front();
				m_queue.pop_front();
				return elem;
			}

		private:
			std::list<value_type> m_queue;
			std::mutex m_mutex;
			std::condition_variable m_cond;
		};
	}
} // namespace caney
