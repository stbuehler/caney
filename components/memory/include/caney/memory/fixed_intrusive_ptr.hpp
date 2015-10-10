/** @file */

#pragma once

#include "internal.hpp"

#include <algorithm>
#include <atomic>
#include <exception>
#include <memory>
#include <type_traits>

__CANEY_MEMORYV1_BEGIN

/**
 * @defgroup fixed_intrusive_ptr fixed-type intrusive pointers
 *
 * @brief @ref fixed_intrusive_ptr pointers keep strong references to objects
 * deriving from @ref fixed_intrusive_ctr; they use "fix types", i.e. when the
 * last reference is dropped, it will use `~T()` to destroy the object (`T`
 * being the first typename given to @ref fixed_intrusive_ctr.
 *
 * @ref fixed_intrusive_ctr maintains the counter directly in the object, saving
 * a separate allocation.
 *
 * objects must be allocated with @ref make_fixed_intrusive() or
 * @ref allocate_fixed_intrusive() to get a @ref fixed_intrusive_ptr to
 * the object.
 *
 * @addtogroup fixed_intrusive_ptr
 * @{
 */

template<typename T>
class fixed_intrusive_ptr;

template<typename U, typename... Args>
fixed_intrusive_ptr<U> allocate_fixed_intrusive(typename U::allocator const& alloc, Args&&... args);

/**
 * @brief @ref fixed_intrusive_ctr is a templated base class; it maintains a counter
 * within the object to keep track of how many @ref fixed_intrusive_ptr pointers
 * reference the object.
 */
template<typename T, template<typename> class Alloc = std::allocator>
class fixed_intrusive_ctr {
public:
	/**
	 * @brief allocator type needs to be accessible in all subclasses
	 */
	using allocator = Alloc<T>;

	/** convenience typedef */
	using pointer = fixed_intrusive_ptr<T>;

private:
	using counter_t = unsigned int;
	using element_t = T;
	template<typename X>
	using allocator_template_t = Alloc<X>;
	using base_fixed_intrusive_ctr_t = fixed_intrusive_ctr<T, Alloc>;
	using alloc_traits = std::allocator_traits<allocator>;

	struct counter : allocator {
		// do NOT copy/move/assign members
		std::atomic<counter_t> m_value{0};

		counter() noexcept { }
		counter(counter const&) noexcept : allocator() { }
		counter(counter&&) noexcept : allocator() { }
		counter& operator=(counter const&) noexcept { return *this; }
		counter& operator=(counter&&) noexcept { return *this; }
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

	static void set(allocator const& alloc, fixed_intrusive_ctr* self) {
		((allocator&)(self->m_counter)) = allocator(alloc);
	}

	void destroy() const noexcept {
		typename alloc_traits::pointer obj = const_cast<typename alloc_traits::pointer>(static_cast<typename alloc_traits::const_pointer>(this));
		allocator& talloc(m_counter);

		try {
			alloc_traits::destroy(talloc, obj);
			alloc_traits::deallocate(talloc, obj, 1);
		} catch (...) {
			std::terminate();
		}
	}

	// stack allocation still works, but heap allocations are forced to go
	// through allocate_fixed_intrusive
	// static void* operator new (std::size_t size) = delete;
	// deleting delete doesn't work somehow...
	// static void operator delete (void *p);

	template<typename _T>
	friend class fixed_intrusive_ptr;

	template<typename U, typename... Args>
	friend
	fixed_intrusive_ptr<U> allocate_fixed_intrusive(typename U::allocator const& alloc, Args&&... args);
};

/**
 * @brief smart pointer keeping a heap allocated `T` object alive.
 * @tparam T type @ref fixed_intrusive_ptr is pointing to
 */
template<typename T>
class fixed_intrusive_ptr {
private:
	using base_fixed_intrusive_ctr_t = typename T::base_fixed_intrusive_ctr_t;
	using allocator = typename base_fixed_intrusive_ctr_t::allocator;
	using element_t = T;
	using element_no_const_t = typename std::remove_const<T>::type;
	static_assert(
		std::is_same<
			fixed_intrusive_ctr<typename base_fixed_intrusive_ctr_t::element_t, base_fixed_intrusive_ctr_t::template allocator_template_t>,
			base_fixed_intrusive_ctr_t>::value,
		"invalid element type");
	static_assert(
		std::is_same<element_no_const_t, typename base_fixed_intrusive_ctr_t::element_t>::value
			|| std::has_virtual_destructor<typename base_fixed_intrusive_ctr_t::element_t>::value,
		"cannot derive from a class derived from fixed_intrusive_ctr unless it has a virtual destructor");

	// construct first pointer (refcount 0)
	explicit fixed_intrusive_ptr(T* ptr) noexcept : m_p(ptr) {
		if (nullptr != ptr) ctr(ptr)->acquire();
	}

public:
	/** result type for @ref count() */
	using counter_t = typename base_fixed_intrusive_ctr_t::counter_t;

	/** construct empty pointer */
	fixed_intrusive_ptr() noexcept = default;

	/** construct empty pointer */
	fixed_intrusive_ptr(decltype(nullptr)) noexcept { };

	/** copy pointer */
	fixed_intrusive_ptr(fixed_intrusive_ptr const& other) noexcept { reset(other.m_p); }

	/** move pointer */
	fixed_intrusive_ptr(fixed_intrusive_ptr&& other) noexcept : m_p(other.m_p) {
		other.m_p = nullptr;
	}

	/** copy pointer */
	fixed_intrusive_ptr<T>& operator=(fixed_intrusive_ptr const& other) {
		fixed_intrusive_ptr tmp;
		std::swap(m_p, tmp.m_p);
		reset(other.m_p);
		// now old object might get destroyed
		return *this;
	}

	/** move pointer */
	fixed_intrusive_ptr<T>& operator=(fixed_intrusive_ptr&& other) noexcept {
		fixed_intrusive_ptr tmp;
		std::swap(m_p, tmp.m_p);
		// now tmp has the old object, m_p is nullptr
		std::swap(m_p, other.m_p);
		// now other.m_p is nullptr, *this has new value
		// now tmp (old object) gets destroyed
		return *this;
	}

	/** convert to pointer to base class */
	template<typename U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
	fixed_intrusive_ptr(fixed_intrusive_ptr<U> const& other) noexcept {
		reset(other.get());
	}

	/** move-convert to pointer to base class */
	template<typename U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
	fixed_intrusive_ptr(fixed_intrusive_ptr<U>&& other) : m_p(other.m_p) {
		other.m_p = nullptr;
	}

	/** copy-convert pointer */
	template<typename U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
	fixed_intrusive_ptr<T>& operator=(fixed_intrusive_ptr<U> const& other) {
		fixed_intrusive_ptr tmp;
		std::swap(m_p, tmp.m_p);
		reset(other.m_p);
		// now old object might get destroyed
		return *this;
	}

	/** move-convert pointer */
	template<typename U, class = typename std::enable_if<std::is_convertible<U*, T*>::value>::type>
	fixed_intrusive_ptr<T>& operator=(fixed_intrusive_ptr<U>&& other) noexcept {
		fixed_intrusive_ptr tmp;
		std::swap(m_p, tmp.m_p);
		std::swap(m_p, other.m_p);
		// now old object might get destroyed
		return *this;
	}

	/** release reference for current object */
	~fixed_intrusive_ptr() { reset(); }


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
	bool operator==(fixed_intrusive_ptr const& other) const { return m_p == other.m_p; }
	/** check whether `other` pointer points not to same object */
	bool operator!=(fixed_intrusive_ptr const& other) const { return m_p != other.m_p; }

	/**
	 * count references for object
	 * @return how many @ref fixed_intrusive_ptr reference given object
	 */
	counter_t count() const {
		return m_p ? ctr(m_p)->m_counter.m_value.load() : 0;
	}

	/**
	 * check whether object is kept alive by a @ref fixed_intrusive_ptr
	 * @return whether object is kept alive by a @ref fixed_intrusive_ptr
	 */
	static bool alive(T* ptr) {
		return ptr ? (ctr(ptr)->m_counter.m_value.load() > 0) : 0;
	}

	/**
	 * count references for object
	 * @return how many @ref fixed_intrusive_ptr reference given object
	 */
	static counter_t count(T* ptr) {
		return ptr ? ctr(ptr)->m_counter.m_value.load() : 0;
	}

	/** shortcut for @ref allocate_fixed_intrusive, defaults to contained element type for construction */
	template<typename U = T, typename... Args>
	static fixed_intrusive_ptr<U> allocate(typename U::allocator alloc, Args&&... args);

	/** shortcut for @ref make_fixed_intrusive, defaults to contained element type for construction */
	template<typename U = T, typename... Args>
	static fixed_intrusive_ptr<U> create(Args&&... args) {
		using alloc_t = typename U::allocator;
		return allocate<U>(alloc_t(), std::forward<Args>(args)...);
	}

private:
	static base_fixed_intrusive_ctr_t const* ctr(T const* ptr) {
		return static_cast<base_fixed_intrusive_ctr_t const*>(ptr);
	}

	void reset(element_no_const_t* ptr) {
		if (m_p == ptr) return;
		if (nullptr != ptr) ctr(ptr)->acquire_alive();
		if (nullptr != m_p) ctr(m_p)->release();
		m_p = ptr;
	}

	T* m_p = nullptr;

	template<typename U, typename... Args>
	friend
	fixed_intrusive_ptr<U> allocate_fixed_intrusive(typename U::allocator const& alloc, Args&&... args);
};

/**
 * allocate object of type T and initialize first @ref fixed_intrusive_ptr
 * pointer keeping it alive
 * @param alloc Allocator to allocate storage with
 * @param args  Arguments to pass to `T` constructor
 * @tparam U    object type to create
 */
template<typename U, typename... Args>
fixed_intrusive_ptr<U> allocate_fixed_intrusive(typename U::allocator const& alloc, Args&&... args) {
	using base_ctr = typename fixed_intrusive_ptr<U>::base_fixed_intrusive_ctr_t;
	using base_alloc_traits = std::allocator_traits<typename U::allocator>;
	using alloc_t = typename base_alloc_traits::template rebind_alloc<typename std::remove_const<U>::type>;
	using alloc_traits = std::allocator_traits<alloc_t>;
	alloc_t local_alloc(alloc);

	typename alloc_traits::pointer ptr = alloc_traits::allocate(local_alloc, 1);
	try {
		alloc_traits::construct(local_alloc, ptr, std::forward<Args>(args)...);
		base_ctr::set(alloc, ptr);
	} catch (...) {
		alloc_traits::deallocate(local_alloc, ptr, 1);
		throw;
	}

	return fixed_intrusive_ptr<U>(ptr);
}

/**
 * @brief create a new object of type `T` using a default constructed
 * allocator.
 * @param args Arguments to forward to contructor of `T`
 * @tparam T   object type to create
 */
template<typename T, typename... Args>
fixed_intrusive_ptr<T> make_fixed_intrusive(Args&&... args) {
	using alloc_t = typename T::allocator;
	return allocate_fixed_intrusive<T>(alloc_t(), std::forward<Args>(args)...);
}

template<typename T> template<typename U, typename... Args>
fixed_intrusive_ptr<U> fixed_intrusive_ptr<T>::allocate(typename U::allocator alloc, Args&&... args) {
	return allocate_fixed_intrusive<U>(std::move(alloc), std::forward<Args>(args)...);
}

/** @} */

__CANEY_MEMORYV1_END
