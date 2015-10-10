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
	typedef generic_intrusive_buffer<allocator_pool::allocator<void>, CounterPolicyT> buffer_t;
	/** intrusive pointer to buffer */
	typedef generic_intrusive_buffer_ptr<allocator_pool::allocator<void>, CounterPolicyT> buffer_ptr_t;

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
		return alloc_intrusive_buffer(m_pool.alloc(), size());
	}

private:
	allocator_pool m_pool;
};

__CANEY_MEMORYV1_END
