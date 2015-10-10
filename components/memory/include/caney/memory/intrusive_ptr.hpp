/** @file */

#pragma once

#include "internal.hpp"

#include <atomic>
#include <cstdint>
#include <exception>
#include <limits>
#include <memory>
#include <type_traits>
#include <utility>

__CANEY_MEMORYV1_BEGIN

/**
 * @defgroup intrusive_ptr intrusive pointers
 *
 * @brief @ref intrusive_ptr pointers keep strong references to objects
 * deriving from @ref intrusive_ctr
 *
 * @ref intrusive_ctr maintains the counter directly in the object, saving
 * a separate allocation.
 *
 * objects must be allocated with @ref make_intrusive or @ref allocate_intrusive to
 * use @ref intrusive_ptr on the object.
 *
 * @ref make_intrusive and @ref allocate_intrusive remembers the type the object was
 * created as (and also the allocator), there is no need for the destructor to
 * be virtual.
 *
 * @addtogroup intrusive_ptr
 * @{
 */

template<typename T>
class intrusive_ptr;

namespace impl {
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

/**
 * @brief @ref intrusive_ctr is a generic base class; it maintains a counter within the
 * object to keep track of how many @ref intrusive_ptr pointers reference the
 * object.
 *
 * @tparam Alloc allocator to use
 */
template<template<typename> class Alloc = std::allocator>
class intrusive_ctr {
public:
	/**
	 * @brief allocator type needs to be accessible in all subclasses
	 */
	using allocator = Alloc<intrusive_ctr>;

private:
	using counter_t = unsigned int;
	using base_intrusive_ctr_t = intrusive_ctr<Alloc>;

	struct counter {
		// do NOT copy/move/assign members
		std::atomic<counter_t> m_value{0};

		// remember how to delete ourself
		impl::intrusive_ctr_deleter<allocator> m_deleter;

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
	static void set(allocator const& alloc, T* self) {
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

	template<typename T>
	friend class intrusive_ptr;

	template<class U, class... Args>
	friend intrusive_ptr<U> allocate_intrusive(typename U::allocator const& alloc, Args&&... args);
};

/**
 * @brief An @ref intrusive_ptr either is empty or points to an object of type `T` - which
 * must derive from @ref intrusive_ctr - and keeps said object alive.
 */
template<typename T>
class intrusive_ptr {
private:
	typedef typename std::template remove_const<T>::type NoConstT;

	// construct first pointer (refcount 0)
	struct InitMarker { };
	explicit intrusive_ptr(T* ptr, InitMarker) noexcept : m_p(ptr) {
		if (nullptr != ptr) ctr(ptr)->acquire();
	}

	// requires the object to be still managed by some intrusive_ptr !
	explicit intrusive_ptr(T* p) noexcept { reset(p); }

	using base_intrusive_ctr_t = typename T::base_intrusive_ctr_t;
public:
	/** result type for @ref count() */
	using counter_t = typename base_intrusive_ctr_t::counter_t;

	/** construct empty pointer */
	intrusive_ptr() noexcept = default;

	/** construct empty pointer */
	intrusive_ptr(decltype(nullptr)) noexcept { };

	/** copy pointer */
	intrusive_ptr(intrusive_ptr const &other) noexcept { reset(other.m_p); }

	/** move pointer */
	intrusive_ptr(intrusive_ptr &&other) noexcept : m_p(other.m_p) {
		other.m_p = nullptr;
	}

	/** copy pointer */
	intrusive_ptr<T>& operator=(intrusive_ptr const &other) {
		intrusive_ptr tmp;
		std::swap(m_p, tmp.m_p);
		reset(other.m_p);
		// now old object might get destroyed
		return *this;
	}

	/** move pointer */
	intrusive_ptr<T>& operator=(intrusive_ptr&& other) noexcept {
		intrusive_ptr tmp;
		std::swap(m_p, tmp.m_p);
		// now tmp has the old object, m_p is nullptr
		std::swap(m_p, other.m_p);
		// now other.m_p is nullptr, *this has new value
		// now old object might get destroyed
		return *this;
	}

	/** convert to pointer to base class */
	template<typename U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
	intrusive_ptr(intrusive_ptr<U> const& other) noexcept {
		reset(other.get());
	}

	/** move-convert to pointer to base class */
	template<typename U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
	intrusive_ptr(intrusive_ptr<U>&& other) : m_p(other.m_p) {
		other.m_p = nullptr;
	}

	/** copy-convert pointer */
	template<typename U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
	intrusive_ptr<T>& operator=(intrusive_ptr<U> const& other) {
		intrusive_ptr tmp;
		std::swap(m_p, tmp.m_p);
		reset(other.m_p);
		// now old object might get destroyed
		return *this;
	}

	/** move-convert pointer */
	template<typename U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
	intrusive_ptr<T>& operator=(intrusive_ptr<U>&& other) noexcept {
		intrusive_ptr tmp;
		std::swap(m_p, tmp.m_p);
		std::swap(m_p, other.m_p);
		// now old object might get destroyed
		return *this;
	}

	/** release reference for current object */
	~intrusive_ptr() { reset(); }

	/** release reference for current object; empty afterwards */
	void reset() {
		if (nullptr == m_p) return;
		ctr(m_p)->release();
		m_p = nullptr;
	}

	/**
	 * @brief access current object
	 * @return pointer to current object
	 */
	T* get() const noexcept { return m_p; }

	/**
	 * @brief access current object (pointer should not be empty)
	 * @return reference to current object
	 */
	T& operator*() const noexcept { return *m_p; }

	/**
	 * @brief access current object
	 * @return pointer to current object
	 */
	T* operator->() const noexcept { return m_p; }

	/**
	 * @brief check whether pointer is not empty
	 * @return whether pointer is not empty
	 */
	explicit operator bool () const noexcept { return nullptr != m_p; }

	/** check whether `other` pointer points to same object */
	bool operator==(intrusive_ptr const &other) const { return m_p == other.m_p; }
	/** check whether `other` pointer points not to same object */
	bool operator!=(intrusive_ptr const &other) const { return m_p != other.m_p; }

	/**
	 * count references for object
	 * @return how many @ref intrusive_ptr reference given object
	 */
	counter_t count() const {
		return m_p ? ctr(m_p)->m_counter.m_value.load() : 0;
	}

	/**
	 * check whether object is kept alive by a @ref intrusive_ptr
	 * @return whether object is kept alive by a @ref intrusive_ptr
	 */
	static bool alive(T* ptr) {
		return ctr(ptr)->m_counter.m_value.load() > 0;
	}

	/**
	 * count references for object
	 * @return how many @ref intrusive_ptr reference given object
	 */
	static counter_t count(T* ptr) {
		return ctr(ptr)->m_counter.m_value.load();
	}

	/** shortcut for @ref allocate_intrusive, defaults to contained element type for construction */
	template<typename U = T, typename... Args>
	static intrusive_ptr<U> allocate(typename U::allocator alloc, Args&&... args);

	/** shortcut for @ref make_intrusive, defaults to contained element type for construction */
	template<typename U = T, typename... Args>
	static intrusive_ptr<U> create(Args&&... args) {
		using alloc_t = typename U::allocator;
		return allocate<U>(alloc_t(), std::forward<Args>(args)...);
	}

private:
	static base_intrusive_ctr_t const* ctr(T const* ptr) {
		return static_cast<base_intrusive_ctr_t const*>(ptr);
	}

	void reset(T* ptr) {
		if (m_p == ptr) return;
		if (nullptr != ptr) ctr(ptr)->acquire_alive();
		if (nullptr != m_p) ctr(m_p)->release();
		m_p = ptr;
	}

	T* m_p = nullptr;

	template<typename U, typename... Args>
	friend
	intrusive_ptr<U> allocate_intrusive(typename U::allocator const& alloc, Args&&... args);

	template<typename U, typename V>
	friend
	intrusive_ptr<U> static_intrusive_ptr_cast(intrusive_ptr<V> const& r);

	template<typename U, typename V>
	friend
	intrusive_ptr<U> dynamic_intrusive_ptr_cast(intrusive_ptr<V> const& r);

	template<typename U, typename V>
	friend
	intrusive_ptr<U> const_intrusive_ptr_cast(intrusive_ptr<V> const& r);
};

/**
 * allocate object of type T and initialize first @ref intrusive_ptr
 * pointer keeping it alive
 * @param alloc Allocator to allocate storage with
 * @param args  Arguments to pass to `T` constructor
 * @tparam U    object type to create
 */
template<typename U, typename... Args>
intrusive_ptr<U> allocate_intrusive(typename U::allocator const& alloc, Args&&... args) {
	using base_ctr = typename U::base_intrusive_ctr_t;
	typedef typename std::remove_const<U>::type U_no_const;
	typedef std::allocator_traits<typename U::allocator> TT;
	typedef typename TT::template rebind_alloc<U_no_const> UAlloc;
	UAlloc ualloc(alloc);
	typedef std::allocator_traits< UAlloc > UT;

	typename UT::pointer ptr = UT::allocate(ualloc, 1);
	try {
		UT::construct(ualloc, ptr, std::forward<Args>(args)...);
		base_ctr::set(alloc, ptr);
	} catch (...) {
		UT::deallocate(ualloc, ptr, 1);
		throw;
	}

	return intrusive_ptr<U>(ptr, typename intrusive_ptr<U>::InitMarker());
}

/**
 * @brief create a new object of type `T` using a default constructed
 * allocator.
 * @param args Arguments to forward to contructor of `T`
 * @tparam T   object type to create
 */
template<typename T, typename... Args>
intrusive_ptr<T> make_intrusive(Args&&... args) {
	return allocate_intrusive<T>(typename T::allocator(), std::forward<Args>(args)...);
}

template<typename T> template<typename U, typename... Args>
intrusive_ptr<U> intrusive_ptr<T>::allocate(typename U::allocator alloc, Args&&... args) {
	return allocate_intrusive<U>(std::move(alloc), std::forward<Args>(args)...);
}

/** `static_cast` on pointer for @ref intrusive_ptr */
template<typename U, typename V>
intrusive_ptr<U> static_intrusive_ptr_cast(intrusive_ptr<V> const& r) {
	return intrusive_ptr<U>(static_cast<U*>(r.get()));
}

/** `dynamic_cast` on pointer for @ref intrusive_ptr */
template<typename U, typename V>
intrusive_ptr<U> dynamic_intrusive_ptr_cast(intrusive_ptr<V> const& r) {
	return intrusive_ptr<U>(dynamic_cast<U*>(r.get()));
}

/** `const_cast` on pointer for @ref intrusive_ptr */
template<typename U, typename V>
intrusive_ptr<U> const_intrusive_ptr_cast(intrusive_ptr<V> const& r) {
	return intrusive_ptr<U>(const_cast<U*>(r.get()));
}

/** @} */

__CANEY_MEMORYV1_END
