#pragma once

#include <atomic>
#include <cstdint>
#include <exception>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

/* intrusive_ctr is a generic base class; it maintains a counter within the
 * object to keep track of how many (intrusive_ptr) pointers reference the
 * object.
 *
 * objects must be allocated with make_intrusive or allocate_intrusive to
 * use intrusive_ptr on the object.
 *
 * make_intrusive and allocate_intrusive remembers the type the object was
 * created as (and also the allocator), there is no need for the destructor to
 * be virtual.
 */

namespace caney {
	namespace memory {
		inline namespace v1 {
			template<typename T, typename Alloc>
			class intrusive_ptr;

			namespace detail {
				template<typename Alloc>
				class intrusive_ctr_deleter {
				private:
					struct deleter : public Alloc {
						// do NOT copy/move/assign members
						void (*m_delete)(Alloc const &alloc, void* ptr) = nullptr;
						unsigned int m_self_offset = 0;

						deleter() = default;
						deleter(deleter const&) { }
						deleter(deleter&&) { }
						deleter& operator=(deleter const&) { return *this; }
						deleter& operator=(deleter&&) { return *this; }
					};
					deleter m_deleter;

				public:
					/* intrusive_ctr_deleter has to be part of obj somehow at a
					 * fixed offset. remember offset and erase type through
					 * function pointer.
					 */
					template<typename T>
					void set(Alloc const &alloc, T* obj) {
						// m_self_offset <= sizeof(T) --> making sure sizeof(T) isn't too large makes sure m_self_offset doesn't overflow
						static_assert(sizeof(T) <= std::numeric_limits<decltype(m_deleter.m_self_offset)>::max(), "Struct too large");
						// make sure "this" is actually part of obj
						if (reinterpret_cast<std::uintptr_t>(this) < reinterpret_cast<std::uintptr_t>(obj)
							|| reinterpret_cast<std::uintptr_t>(this) >= reinterpret_cast<std::uintptr_t>(obj) + sizeof(T)) {
							std::terminate();
						}

						((Alloc&)m_deleter) = alloc;
						m_deleter.m_delete = &delete_cb<T>;
						m_deleter.m_self_offset = reinterpret_cast<std::uintptr_t>(this) - reinterpret_cast<std::uintptr_t>(obj);
					}

					// delete the managed object
					void run() noexcept {
						m_deleter.m_delete(m_deleter, get());
					}

				private:
					// restore the T* obj pointer passed to set()
					void* get() noexcept {
						return reinterpret_cast<void*>(reinterpret_cast<char*>(this) - m_deleter.m_self_offset);
					}

					template<typename T>
					static void delete_cb(Alloc const &alloc, void* ptr) noexcept {
						typedef typename std::template allocator_traits<Alloc>::template rebind_alloc<T> TAlloc;
						TAlloc talloc(alloc);
						typedef std::allocator_traits< TAlloc > TT;

						auto obj = static_cast<typename TT::pointer>(ptr);
						try {
							TT::destroy(talloc, obj);
							TT::deallocate(talloc, obj, 1);
						} catch (...) {
							std::terminate();
						}
					}
				};
			}

			template<typename Alloc>
			class intrusive_ctr {
			public:
				using counter_t = unsigned int;

			private:
				struct counter {
					// do NOT copy/move/assign members
					std::atomic<counter_t> m_value{0};

					// remember how to delete ourself
					detail::intrusive_ctr_deleter<Alloc> m_deleter;

					counter() noexcept { }
					counter(counter const&) noexcept { }
					counter(counter&&) noexcept { }
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
				void release() const noexcept {
					auto before = m_counter.m_value--;
					if (0 == before) std::terminate();
					if (1 == before) destroy();
				}

				template<typename T>
				static void set(Alloc const& alloc, T* self) {
					intrusive_ctr *ctr = static_cast<intrusive_ctr*>(self);
					ctr->m_counter.m_deleter.set(alloc, self);
				}

				void destroy() const noexcept {
					m_counter.m_deleter.run();
				}

				// stack allocation still works, but heap allocations are forced to go
				// through allocate_intrusive
				// static void* operator new (std::size_t size) = delete;
				// deleting delete doesn't work somehow...
				// static void operator delete (void *p);

				template<typename T, typename _Alloc>
				friend class intrusive_ptr;

				template<class T, class _Alloc, class... Args>
				friend intrusive_ptr<T, _Alloc> allocate_intrusive(_Alloc&& alloc, Args&&... args);
			};

			template<typename T, typename Alloc>
			class intrusive_ptr {
			private:
				typedef typename std::template remove_const<T>::type NoConstT;

				// construct first pointer (refcount 0)
				struct InitMarker { };
				explicit intrusive_ptr(T* ptr, InitMarker) noexcept : m_p(ptr) {
					if (nullptr != ptr) ctr(ptr)->acquire();
				}

			public:
				using counter_t = typename intrusive_ctr<Alloc>::counter_t;

				intrusive_ptr() noexcept = default;
				intrusive_ptr(decltype(nullptr)) noexcept { };
				intrusive_ptr(intrusive_ptr const &other) noexcept { reset(other.m_p); }
				template<typename U, class = typename std::enable_if<std::is_base_of<T, U>::value>::type>
				intrusive_ptr(intrusive_ptr<U, Alloc> const &other) noexcept { reset(other.get()); }
				intrusive_ptr(intrusive_ptr &&other) noexcept : m_p(other.m_p) {
					other.m_p = nullptr;
				}
				intrusive_ptr<T, Alloc>& operator=(intrusive_ptr const &other) {
					reset(other.m_p);
					return *this;
				}
				intrusive_ptr<T, Alloc>& operator=(intrusive_ptr&& other) noexcept {
					std::swap(m_p, other.m_p);
					return *this;
				}
				~intrusive_ptr() { reset(); }

				// requires the object to be still managed by some intrusive_ptr !
				// use allocate_intrusive to create new objects.
				explicit intrusive_ptr(T* p) noexcept { reset(p); }
				void reset(T* ptr) {
					if (m_p == ptr) return;
					if (nullptr != ptr) ctr(ptr)->acquire_alive();
					if (nullptr != m_p) ctr(m_p)->release();
					m_p = ptr;
				}


				void reset() {
					if (nullptr == m_p) return;
					ctr(m_p)->release();
					m_p = nullptr;
				}

				T* get() const noexcept { return m_p; }
				T& operator*() const noexcept { return *m_p; }
				T* operator->() const noexcept { return m_p; }

				explicit operator bool () const noexcept { return nullptr != m_p; }

				bool operator==(intrusive_ptr const &other) const { return m_p == other.m_p; }
				bool operator!=(intrusive_ptr const &other) const { return m_p != other.m_p; }

				static bool alive(T* ptr) {
					return ctr(ptr)->m_counter.m_value.load() > 0;
				}

				static counter_t count(T* ptr) {
					return ctr(ptr)->m_counter.m_value.load();
				}

			private:
				static intrusive_ctr<Alloc> const* ctr(T const* ptr) {
					return static_cast< intrusive_ctr<Alloc> const* >(ptr);
				}

				T* m_p = nullptr;

				template<typename U, typename _Alloc, typename... Args>
				friend
				intrusive_ptr<U, _Alloc> allocate_intrusive(_Alloc&& alloc, Args&&... args);
			};

			template<typename T, typename Alloc, typename... Args>
			inline
			intrusive_ptr<T, Alloc> allocate_intrusive(Alloc&& alloc, Args&&... args) {
				typedef typename std::template remove_const<T>::type U;
				typedef std::allocator_traits< Alloc > TT;
				typedef typename TT::template rebind_alloc<U> UAlloc;
				UAlloc ualloc(alloc);
				typedef std::allocator_traits< UAlloc > UT;

				typename UT::pointer ptr = UT::allocate(ualloc, 1);
				try {
					UT::construct(ualloc, ptr, std::forward<Args>(args)...);
					intrusive_ctr<Alloc>::set(alloc, ptr);
				} catch (...) {
					UT::deallocate(ualloc, ptr, 1);
					throw;
				}

				return intrusive_ptr<T, Alloc>(ptr, typename intrusive_ptr<T, Alloc>::InitMarker());
			}

			template<typename T, typename Alloc, typename... Args>
			inline
			intrusive_ptr<T, Alloc> make_intrusive(Args&&... args) {
				return allocate_intrusive<T, Alloc>(Alloc(), std::forward<Args>(args)...);
			}

			template<typename U, typename V, typename Alloc>
			intrusive_ptr<U, Alloc> static_intrusive_ptr_cast(intrusive_ptr<V, Alloc> const& r) {
				return intrusive_ptr<U, Alloc>(static_cast<U>(r.get()));
			}
			template<typename U, typename V, typename Alloc>
			intrusive_ptr<U, Alloc> dynamic_intrusive_ptr_cast(intrusive_ptr<V, Alloc> const& r) {
				return intrusive_ptr<U, Alloc>(dynamic_cast<U>(r.get()));
			}
			template<typename U, typename V, typename Alloc>
			intrusive_ptr<U, Alloc> const_intrusive_ptr_cast(intrusive_ptr<V, Alloc> const& r) {
				return intrusive_ptr<U, Alloc>(const_cast<U>(r.get()));
			}
		}
	} // namespace memory
} // namespace caney
