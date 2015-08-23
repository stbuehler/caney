#pragma once

#include "caney/std/synchronized.hpp"

#include <memory>

#include <boost/noncopyable.hpp>

namespace caney {
	namespace memory {
		inline namespace v1 {
			class allocator_pool : private boost::noncopyable {
			private:
				struct chunk_link {
					chunk_link* next{nullptr};
				};

				class pool : private boost::noncopyable {
				public:
					explicit pool(std::size_t size)
					: m_size{size} {
					}
					~pool();

					static char* allocate(pool* p, std::size_t n);
					static void deallocate(pool *p, char* obj, std::size_t n);

					std::size_t size() const { return m_size; }

				private:
					const std::size_t m_size{0};
					caney::synchronized<chunk_link*> m_front{nullptr};
				};

				class allocator_base {
				public:
					explicit allocator_base(std::weak_ptr<pool> p)
					: m_pool(p) {
					}

					char* allocate(std::size_t n) {
						return pool::allocate(m_pool.lock().get(), n);
					}

					void deallocate(char* obj, std::size_t n) {
						pool::deallocate(m_pool.lock().get(), obj, n);
					}

				private:
					std::weak_ptr<pool> m_pool;
				};

			public:
				template<typename Value>
				class allocator {
				public:
					typedef Value value_type;
					allocator(allocator_base const& base)
					: m_base(base) {
					}

					template<typename Other>
					allocator(allocator<Other> const& other)
					: m_base(other.m_base) {
					}

					value_type* allocate(std::size_t n) {
						return reinterpret_cast<value_type*>(m_base.allocate(sizeof(value_type) * n));
					}
					void deallocate(value_type* obj, std::size_t n) {
						m_base.deallocate(reinterpret_cast<char*>(obj), sizeof(value_type) * n);
					}

				private:
					allocator_base m_base;

					template<typename Other>
					friend class allocator;
				};

				explicit allocator_pool(std::size_t size);

				std::size_t size() const;
				allocator<void> alloc() const;

			private:
				std::shared_ptr<pool> m_pool;
			};

			/* all allocators can allocate and free anything */
			template<typename A, typename B>
			bool operator==(allocator_pool::allocator<A> const&, allocator_pool::allocator<B> const&) {
				return true;
			}
			template<typename A, typename B>
			bool operator!=(allocator_pool::allocator<A> const&, allocator_pool::allocator<B> const&) {
				return false;
			}
		}
	} // namespace memory
} // namespace caney
