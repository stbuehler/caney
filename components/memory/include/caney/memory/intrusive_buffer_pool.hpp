#pragma once

#include "allocator_pool.hpp"
#include "intrusive_buffer.hpp"

namespace caney {
	namespace memory {
		inline namespace v1 {
			template<typename CounterPolicyT = boost::thread_safe_counter>
			class intrusive_buffer_pool {
			public:
				typedef generic_intrusive_buffer<allocator_pool::allocator<void>, CounterPolicyT> buffer_t;
				typedef generic_intrusive_buffer_ptr<allocator_pool::allocator<void>, CounterPolicyT> buffer_ptr_t;

				explicit intrusive_buffer_pool(std::size_t n)
				: m_pool(sizeof(buffer_t) + n) {
				}

				std::size_t size() const {
					return m_pool.size() - sizeof(buffer_t);
				}

				buffer_ptr_t allocate() {
					return alloc_intrusive_buffer(m_pool.alloc(), size());
				}

			private:
				allocator_pool m_pool;
			};

		}
	} // namespace memory
} // namespace caney
