/** @file */

#pragma once

#include "caney/std/synchronized.hpp"

#include "internal.hpp"

#include <memory>

#include <boost/noncopyable.hpp>

__CANEY_MEMORYV1_BEGIN

/**
 * @brief An allocator which keeps allocations of a given size in a pool for later reuse.
 *
 * @ref allocator_pool represents the pool; to get an actual allocator call
 * @ref allocator_pool::alloc() which returns an allocator compatible with
 * `std::allocator_traits` ("Allocator concept").
 *
 * When allocating/deallocating an object with the size the @ref allocator_pool
 * was created for, the allocator will use the internal pool (a simple
 * linked list) to remember and reuse allocations.
 *
 * If the pool is empty on allocation it uses `std::allocator` for the initial
 * allocation; `std::allocator` is also used to free entries in the internal pool.
 *
 * When allocation/deallocating objects of different sizes it just uses the
 * default `std::allocator` instead.
 *
 * The @ref allocator_pool has a shared state; all @ref allocator_pool::allocator -s
 * created from it share this state, but only keep a weak reference.
 * So when the @ref allocator_pool is released the pool will get freed, and no
 * further allocations/deallocations are cached and go instead to `std::allocator`.
 *
 * All allocations are allocating at least sizeof(void*); this means the
 * @ref allocator_pool allocator is not directly compatible to `std::allocator`.
 * Also the pool size should be at least sizeof(void*), otherwise it won't cache
 * anything.
 */
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

		/** @brief allocate object from (possible nullptr) pool */
		static char* allocate(pool* p, std::size_t n);
		/** @brief deallocate object in (possible nullptr) pool */
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
	/**
	 * @brief allocator implementing the C++ Allocator concept
	 *
	 * Usable with `std::allocator_traits`.
	 */
	template<typename Value>
	class allocator {
	public:
		/** the object type to allocate (required by Allocator concept) */
		typedef Value value_type;

		/**
		 * @brief internal constructor to create initial allocator instance from pool
		 * @param base internal allocator_base containing weak reference to pool
		 * @internal
		 */
		allocator(allocator_base const& base)
		: m_base(base) {
		}

		/**
		 * @brief constructor to change value_type (required by Allocator concept for rebind)
		 * @param other other constructor to share the pool from
		 */
		template<typename Other>
		allocator(allocator<Other> const& other)
		: m_base(other.m_base) {
		}

		/**
		 * @brief allocate `n` objects of type @ref value_type (required by Allocator concept)
		 * @param n number of objects to allocate
		 * @return pointer to first object in the allocated array of `n` entries of type @ref value_type
		 *
		 * Must be followed by a matching deallocation later, otherwise memory will leak.
		 */
		value_type* allocate(std::size_t n) {
			return reinterpret_cast<value_type*>(m_base.allocate(sizeof(value_type) * n));
		}

		/**
		 * @brief free `n` objects of type @ref value_type (required by Allocator concept)
		 * @param obj pointer to first object in the allocated array of `n` entries of type @ref value_type
		 * @param n   number of objects to free
		 *
		 * Must match a previous allocation through allocate.
		 */
		void deallocate(value_type* obj, std::size_t n) {
			m_base.deallocate(reinterpret_cast<char*>(obj), sizeof(value_type) * n);
		}

	private:
		allocator_base m_base;

		template<typename Other>
		friend class allocator;
	};

	/**
	 * @brief initialize @ref allocator_pool
	 * @param size the size of objects the pool should cache allocations for
	 */
	explicit allocator_pool(std::size_t size);

	/**
	 * @brief return the object size the pool caches allocation for
	 * @return size the pool was created with
	 */
	std::size_t size() const;

	/**
	 * @brief create an allocator for the pool; you need to rebind it to a
	 * specific value_type to actually use it.
	 *
	 * @return an allocator implementing the C++ Allocator concept
	 */
	allocator<void> alloc() const;

private:
	std::shared_ptr<pool> m_pool;
};

/**
 * @{
 * @brief compare two pool allocators; all pool allocators are compatible and therefor "equal" -
 * if the size doesn't match they just won't use the cache.
 */
template<typename A, typename B>
bool operator==(allocator_pool::allocator<A> const&, allocator_pool::allocator<B> const&) {
	return true;
}
template<typename A, typename B>
bool operator!=(allocator_pool::allocator<A> const&, allocator_pool::allocator<B> const&) {
	return false;
}
/** @} */

__CANEY_MEMORYV1_END
