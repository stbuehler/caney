#pragma once

#include "caney/std/private.hpp"

#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>
/* counter policies: */
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

namespace caney {
	namespace memory {
		inline namespace v1 {
			/* forward declarations */
			template<typename AllocatorT = std::allocator<void>, typename CounterPolicyT = boost::thread_safe_counter>
			class intrusive_buffer;

			template<typename AllocatorT, typename CounterPolicyT = boost::thread_safe_counter>
			inline boost::intrusive_ptr<intrusive_buffer<AllocatorT, CounterPolicyT>> alloc_intrusive_buffer(AllocatorT const& alloc, std::size_t size);
			template<typename AllocatorT, typename CounterPolicyT>
			inline void intrusive_ptr_add_ref(const intrusive_buffer<AllocatorT, CounterPolicyT>* p) BOOST_NOEXCEPT;
			template<typename AllocatorT, typename CounterPolicyT>
			inline void intrusive_ptr_release(const intrusive_buffer<AllocatorT, CounterPolicyT>* p) BOOST_NOEXCEPT;

			template<typename AllocatorT = std::allocator<void>, typename CounterPolicyT = boost::thread_safe_counter>
			using intrusive_buffer_ptr = boost::intrusive_ptr<intrusive_buffer<AllocatorT, CounterPolicyT>>;

			/* an intrusive buffer:
			 * - keeps track how it was allocated, how big the buffer is and how many pointers there are (using boost::intrusive_ptr)
			 * - the meta data and the buffer are allocated as one
			 * - uses AllocatorT::rebind_alloc<char> for memory management
			 */
			template<typename AllocatorT, typename CounterPolicyT>
			class intrusive_buffer final : private AllocatorT, private boost::noncopyable
			{
			public:
				typedef unsigned char* iterator;
				typedef unsigned char const* const_iterator;
				typedef unsigned char value_type;
				typedef std::size_t size_type;
				typedef std::ptrdiff_t difference_type;

				/* only public to be accessible by allocator::construct; use make_intrusive_buffer or alloc_intrusive_buffer instead */
				explicit intrusive_buffer(AllocatorT const& alloc, std::size_t size, caney::private_tag_t)
				: AllocatorT(alloc), m_size(size) {
				}

				size_type size() const { return m_size; }
				bool empty() const { return 0 == size(); }
				explicit operator bool() const { return !empty(); }

				/* the buffer data region starts directly after the meta object */
				unsigned char* data() const { return const_cast<unsigned char*>(reinterpret_cast<unsigned char const*>(this + 1)); }

				iterator begin() const { return data(); }
				iterator end() const { return data() + size(); }
				const_iterator cbegin() const { return data(); }
				const_iterator cend() const { return data() + size(); }

				unsigned char& operator[](size_type ndx) const {
					if (ndx >= size()) std::terminate();
					return data()[ndx];
				}

			private:
				typedef typename CounterPolicyT::type counter_t;
				mutable counter_t m_ref_counter{0};
				const std::size_t m_size{0};

				typedef intrusive_buffer<AllocatorT, CounterPolicyT> self_t;

				typedef typename std::allocator_traits<AllocatorT>::template rebind_alloc<self_t> allocator_t;
				typedef std::allocator_traits<allocator_t> allocator_traits_t;
				typedef typename std::allocator_traits<AllocatorT>::template rebind_alloc<char> mem_allocator_t;
				typedef std::allocator_traits<mem_allocator_t> mem_allocator_traits_t;

				friend boost::intrusive_ptr<intrusive_buffer<AllocatorT, CounterPolicyT>> alloc_intrusive_buffer<AllocatorT, CounterPolicyT>(AllocatorT const& alloc, std::size_t size);
				friend void intrusive_ptr_add_ref<AllocatorT, CounterPolicyT>(const intrusive_buffer<AllocatorT, CounterPolicyT>* p) BOOST_NOEXCEPT;
				friend void intrusive_ptr_release<AllocatorT, CounterPolicyT>(const intrusive_buffer<AllocatorT, CounterPolicyT>* p) BOOST_NOEXCEPT;
			};

			template<typename AllocatorT, typename CounterPolicyT>
			inline boost::intrusive_ptr<intrusive_buffer<AllocatorT, CounterPolicyT>> alloc_intrusive_buffer(AllocatorT const& alloc, std::size_t size) {
				typedef intrusive_buffer<AllocatorT, CounterPolicyT> buffer_t;
				typename buffer_t::allocator_t buffer_alloc(alloc);
				typename buffer_t::mem_allocator_t mem_alloc(alloc);

				typename buffer_t::mem_allocator_traits_t::pointer raw_ptr = buffer_t::mem_allocator_traits_t::allocate(mem_alloc, sizeof(buffer_t) + size);
				typename buffer_t::allocator_traits_t::pointer ptr = reinterpret_cast<typename buffer_t::allocator_traits_t::pointer>(raw_ptr);
				try {
					buffer_t::allocator_traits_t::construct(buffer_alloc, ptr, alloc, size, caney::private_tag);
				} catch (...) {
					buffer_t::mem_allocator_traits_t::deallocate(mem_alloc, raw_ptr, sizeof(buffer_t) + size);
					throw;
				}
				return boost::intrusive_ptr<buffer_t>(ptr);
			}

			template<typename CounterPolicyT = boost::thread_safe_counter>
			inline boost::intrusive_ptr<intrusive_buffer<std::allocator<void>, CounterPolicyT>> make_intrusive_buffer(std::size_t size) {
				return alloc_intrusive_buffer(std::allocator<void>(), size);
			}

			template<typename AllocatorT, typename CounterPolicyT>
			inline void intrusive_ptr_add_ref(const intrusive_buffer<AllocatorT, CounterPolicyT>* p) BOOST_NOEXCEPT
			{
				CounterPolicyT::increment(p->m_ref_counter);
			}

			template<typename AllocatorT, typename CounterPolicyT>
			inline void intrusive_ptr_release(const intrusive_buffer<AllocatorT, CounterPolicyT>* p) BOOST_NOEXCEPT
			{
				try {
					if (CounterPolicyT::decrement(p->m_ref_counter) == 0) {
						typedef intrusive_buffer<AllocatorT, CounterPolicyT> buffer_t;
						std::size_t size = p->m_size;
						typename buffer_t::allocator_t buffer_alloc(static_cast<AllocatorT const&>(*p));
						typename buffer_t::mem_allocator_t mem_alloc(static_cast<AllocatorT const&>(*p));
						buffer_t::allocator_traits_t::destroy(buffer_alloc, p);
						buffer_t::mem_allocator_traits_t::deallocate(mem_alloc, reinterpret_cast<char*>(const_cast<buffer_t*>(p)), sizeof(buffer_t) + size);
					}
				} catch (...) {
					std::terminate();
				}
			}
		}
	} // namespace memory
} // namespace caney
