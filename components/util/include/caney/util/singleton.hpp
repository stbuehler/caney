/** @file */

#pragma once

#include "internal.hpp"

#include <memory>
#include <mutex>

__CANEY_UTILV1_BEGIN

/**
 * create a shared default-constructed Value on demand, but doesn't
 * keep it alive if nothing else references it.
 */
template <typename Value>
class singleton {
public:
	/** type to create singleton of */
	using value_type = Value;

	/**
	 * @brief default constructor
	 */
	constexpr singleton() = default;

	/**
	 * @brief get singleton instance (create on demand if needed)
	 * @return singleton instance
	 */
	std::shared_ptr<value_type> get() {
		std::lock_guard<std::mutex> lock(m_mutex);
		std::shared_ptr<value_type> result = m_ref.lock();
		if (!result) { m_ref = result = std::make_shared<value_type>(); }
		return result;
	}

private:
	std::mutex m_mutex;
	std::weak_ptr<value_type> m_ref;
};

__CANEY_UTILV1_END
