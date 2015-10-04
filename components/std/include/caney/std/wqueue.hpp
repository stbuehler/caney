/** @file */

#pragma once

#include "internal.hpp"

#include <mutex>
#include <condition_variable>
#include <list>

#include <boost/noncopyable.hpp>

__CANEY_STDV1_BEGIN

/**
 * waitqueue for some value type
 *
 * @tparam Value values stored in the queue
 */
template<typename Value>
class wqueue : private boost::noncopyable {
public:
	/** value type stored in the queue */
	using value_type = Value;

	/**
	 * @brief store a new value in the queue
	 * @param args args to construct new value from
	 *
	 * Holds lock while construcing new value and emplacing it at the end
	 * of the queue, then wakes up one thread @ref wait() -ing for new
	 * values (if there is any).
	 */
	template<typename... Args>
	void emplace(Args&&... args) {
		{
			std::lock_guard<std::mutex> lock(m_mutex);
			m_queue.emplace_back(std::forward<Args>(args)...);
		}
		m_cond.notify_one();
	}

	/**
	 * @brief possibly wait and pop and get first element from the queue
	 * @return (previously) first element in the queue
	 *
	 * Waits until there is at least one element in the queue,
	 * reads the first element and then removes it.
	 */
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

__CANEY_STDV1_END
