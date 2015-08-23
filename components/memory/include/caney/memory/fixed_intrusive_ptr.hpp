#pragma once

#include <algorithm>
#include <atomic>
#include <exception>
#include <memory>
#include <type_traits>

/* fixed_intrusive_ctr is a templated base class; it maintains a counter
 * within the object to keep track of how many (fixed_intrusive_ptr) pointers
 * reference the object.
 *
 * objects must be allocated with make_fixed_intrusive or
 * allocate_fixed_intrusive to use fixed_intrusive_ptr on the object.
 *
 * the object is always destroyed through ~T() ("fixed type"); if you want to
 * manage inherited objects ~T() must be virtual.
 */

namespace caney {
	namespace memory {
		inline namespace v1 {
			template<typename T, typename Alloc>
			class fixed_intrusive_ptr;

			template<typename T, typename Alloc>
			class fixed_intrusive_ctr {
			public:
				using counter_t = unsigned int;

			private:
				typedef typename std::template allocator_traits<Alloc>::template rebind_alloc<T> _Alloc;
				typedef typename std::template allocator_traits<_Alloc> TT;

				struct counter : _Alloc {
					// do NOT copy/move/assign members
					std::atomic<counter_t> m_value{0};

					counter() noexcept { }
					counter(counter const&) noexcept : _Alloc() { }
					counter(counter&&) noexcept : _Alloc() { }
					counter& operator=(counter const&) noexcept { return *this; }
					counter& operator=(counter &&) noexcept { return *this; }
				};
				mutable counter m_counter;

				void acquire_alive() const noexcept {
					auto before = m_counter.m_value++;
					if (0 == before || 0 == before + 1) std::terminate();
				}
				void acquire() const noexcept {
					auto before = m_counter.m_value++;
					if (0 == before + 1) std::terminate();
				}
				void release() const {
					auto before = m_counter.m_value--;
					if (0 == before) std::terminate();
					if (1 == before) destroy();
				}

				static void set(Alloc const &alloc, fixed_intrusive_ctr* self) {
					((_Alloc&)(self->m_counter)) = _Alloc(alloc);
				}

				void destroy() const noexcept {
					typename TT::pointer obj = const_cast<typename TT::pointer>(static_cast<typename TT::const_pointer>(this));
					_Alloc& talloc(m_counter);

					try {
						TT::destroy(talloc, obj);
						TT::deallocate(talloc, obj, 1);
					} catch (...) {
						std::terminate();
					}
				}

				// stack allocation still works, but heap allocations are forced to go
				// through allocate_fixed_intrusive
				// static void* operator new (std::size_t size) = delete;
				// deleting delete doesn't work somehow...
				// static void operator delete (void *p);

				template<typename _T, typename _Alloc>
				friend class fixed_intrusive_ptr;

				template<typename U, typename _Alloc, typename... Args>
				friend
				fixed_intrusive_ptr<U, _Alloc> allocate_fixed_intrusive(_Alloc&& alloc, Args&&... args);
			};

			template<typename T, typename Alloc>
			class fixed_intrusive_ptr {
			private:
				typedef typename std::template remove_const<T>::type NoConstT;

				// construct first pointer (refcount 0)
				explicit fixed_intrusive_ptr(T* ptr) noexcept : m_p(ptr) {
					if (nullptr != ptr) ctr(ptr)->acquire();
				}

			public:
				using counter_t = typename fixed_intrusive_ctr<T, Alloc>::counter_t;

				fixed_intrusive_ptr() noexcept = default;
				fixed_intrusive_ptr(decltype(nullptr)) noexcept { };
				fixed_intrusive_ptr(fixed_intrusive_ptr const &other) noexcept { reset(other.m_p); }
				template< typename = std::enable_if< std::is_const<T>::value > >
				fixed_intrusive_ptr(fixed_intrusive_ptr< NoConstT, Alloc> const &other) noexcept { reset(other.get()); }
				fixed_intrusive_ptr(fixed_intrusive_ptr &&other) noexcept : m_p(other.m_p) {
					other.m_p = nullptr;
				}
				fixed_intrusive_ptr<T, Alloc>& operator=(fixed_intrusive_ptr const &other) {
					reset(other.m_p);
					return *this;
				}
				fixed_intrusive_ptr<T, Alloc>& operator=(fixed_intrusive_ptr&& other) noexcept {
					std::swap(m_p, other.m_p);
					return *this;
				}
				~fixed_intrusive_ptr() { reset(); }


				void reset() {
					if (nullptr == m_p) return;
					ctr(m_p)->release();
					m_p = nullptr;
				}

				T* get() const noexcept { return m_p; }
				T& operator*() const noexcept { return *m_p; }
				T* operator->() const noexcept { return m_p; }

				explicit operator bool () const noexcept { return nullptr != m_p; }

				bool operator==(fixed_intrusive_ptr const &other) const { return m_p == other.m_p; }
				bool operator!=(fixed_intrusive_ptr const &other) const { return m_p != other.m_p; }

				static bool alive(T* ptr) {
					return ctr(ptr)->m_counter.m_value.load() > 0;
				}

				static counter_t count(T* ptr) {
					return ctr(ptr)->m_counter.m_value.load();
				}

			private:
				static fixed_intrusive_ctr<NoConstT, Alloc> const* ctr(T const* ptr) {
					return static_cast< fixed_intrusive_ctr<NoConstT, Alloc> const* >(ptr);
				}

				void reset(NoConstT* ptr) {
					if (m_p == ptr) return;
					if (nullptr != ptr) ctr(ptr)->acquire_alive();
					if (nullptr != m_p) ctr(m_p)->release();
					m_p = ptr;
				}

				T* m_p = nullptr;

				template<typename U, typename _Alloc, typename... Args>
				friend
				fixed_intrusive_ptr<U, _Alloc> allocate_fixed_intrusive(_Alloc&& alloc, Args&&... args);
			};

			template<typename T, typename Alloc, typename... Args>
			inline
			fixed_intrusive_ptr<T, Alloc> allocate_fixed_intrusive(Alloc&& alloc, Args&&... args) {
				typedef typename std::template remove_const<T>::type U;
				typedef std::allocator_traits< Alloc > TT;
				typedef typename TT::template rebind_alloc<U> UAlloc;
				UAlloc ualloc(alloc);
				typedef std::allocator_traits< UAlloc > UT;

				typename UT::pointer ptr = UT::allocate(ualloc, 1);
				try {
					UT::construct(ualloc, ptr, std::forward<Args>(args)...);
					fixed_intrusive_ctr<U, Alloc>::set(ualloc, ptr);
				} catch (...) {
					UT::deallocate(ualloc, ptr, 1);
					throw;
				}

				return fixed_intrusive_ptr<T, Alloc>(ptr);
			}

			template<typename T, typename Alloc, typename... Args>
			inline
			fixed_intrusive_ptr<T, Alloc> make_fixed_intrusive(Args&&... args) {
				return allocate_fixed_intrusive<T, Alloc>(Alloc(), std::forward<Args>(args)...);
			}
		}
	} // namespace memory
} // namespace caney
