/** @file */

#pragma once

#include "allocator_pool.hpp"
#include "internal.hpp"
#include "intrusive_buffer.hpp"

__CANEY_MEMORYV1_BEGIN

/**
 * @brief pool for @ref generic_intrusive_buffer instances
 * @tparam CounterPolicyT counter policy to use for @ref generic_intrusive_buffer
 */
template<typename CounterPolicyT = boost::thread_safe_counter>
class intrusive_buffer_pool {
public:
	/** intrusive buffer type */
	using buffer_t = generic_intrusive_buffer<CounterPolicyT, allocator_pool::allocator<void>>;
	/** intrusive pointer to buffer */
	using buffer_ptr_t = generic_intrusive_buffer_ptr<CounterPolicyT, allocator_pool::allocator<void>>;

	/**
	 * @brief initialize pool
	 * @param size size of buffers the pool will allocate
	 */
	explicit intrusive_buffer_pool(std::size_t size)
	: m_pool(sizeof(buffer_t) + size) {
	}

	/**
	 * @brief size of buffers this pool will allocate
	 */
	std::size_t size() const {
		return m_pool.size() - sizeof(buffer_t);
	}

	/**
	 * @brief allocate a buffer (or take one from the pool if available)
	 * @return allocated buffer
	 */
	buffer_ptr_t allocate() {
		return buffer_t::allocate(m_pool.alloc(), size());
	}

private:
	allocator_pool m_pool;
};

__CANEY_MEMORYV1_END
