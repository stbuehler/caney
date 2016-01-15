/** @file */

#pragma once

#include "caney/std/tags.hpp"

#include "internal.hpp"

#include <cstring>
#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>
/* counter policies: */
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

__CANEY_MEMORYV1_BEGIN

template <typename Object, typename CounterPolicyT = boost::thread_safe_counter, typename Allocator = std::allocator<void>>
class intrusive_base;

namespace impl {
	/* wrap allocator to allow "late" construction; that
	 * the allocator doesn't need to go through all constructors
	 */
	template <typename Allocator>
	class intrusive_allocator {
	private:
		union inner {
			inner() {}
			inner(inner const&) = delete;
			~inner() {}
			Allocator m_allocator;
		};
		inner m_inner;

	public:
		void init_allocator(Allocator const& alloc) noexcept {
			new (&m_inner.m_allocator) Allocator(alloc);
		}

		void clear_allocator() noexcept {
			m_inner.m_allocator.~Allocator();
		}

		Allocator& allocator() noexcept {
			return m_inner.m_allocator;
		}
	};

	/* wrap counter to prevent copying */
	template <typename CounterPolicyT>
	class intrusive_counter {
	private:
		using counter_type = typename CounterPolicyT::type;
		counter_type m_counter{0};

	public:
		intrusive_counter() noexcept {}
		intrusive_counter(intrusive_counter const&) = delete;
		~intrusive_counter() = default;

		auto load() const noexcept -> decltype(CounterPolicyT::load(m_counter)) {
			return CounterPolicyT::load(m_counter);
		}

		void increment() noexcept {
			CounterPolicyT::increment(m_counter);
		}

		auto decrement() noexcept -> decltype(CounterPolicyT::decrement(m_counter)) {
			return CounterPolicyT::decrement(m_counter);
		}
	};

	// actual traits type
	template <typename Object, typename CounterPolicyT, typename Allocator>
	struct intrusive_traits_impl {
		using object_t = Object;
		using counter_policy_t = CounterPolicyT;
		using allocator_t = Allocator;
		using base_t = intrusive_base<Object, CounterPolicyT, Allocator>;

		template <typename Derived>
		static void add_ref(Derived* p) noexcept;

		template <typename Derived>
		static void release(Derived* p) noexcept;

		template <typename Derived>
		static void init_allocator(Derived* p, Allocator const& alloc) noexcept;
	};

	// if `Base` is a `intrusive_base`, `::type` type member will be `intrusive_traits_impl`
	template <typename Base>
	struct intrusive_traits_base {};

	template <typename Object, typename CounterPolicyT, typename Allocator>
	struct intrusive_traits_base<intrusive_base<Object, CounterPolicyT, Allocator>> {
		using type = intrusive_traits_impl<Object, CounterPolicyT, Allocator>;
	};

	// detect a `intrusive_base<...>` base type of `Derived`
	template <typename Derived>
	struct intrusive_traits_detect {
	private:
		template <typename T, typename Object, typename CounterPolicyT, typename Allocator>
		static intrusive_base<Object, CounterPolicyT, Allocator> test(intrusive_base<Object, CounterPolicyT, Allocator> const*);
		template <typename T>
		static void test(void const*, T* = nullptr);

	public:
		// `void` or the `intrusive_base<...>` base type of `Derived`
		using base = decltype(test<Derived>((Derived*) nullptr));
		// `type` will have a `::type` type member if `base` is not `void`
		using type = intrusive_traits_base<base>;
	};

	// get traits for a `Derived` type (must be derived from `intrusive_base<...>` or this fails)
	template <typename Derived>
	using intrusive_traits = typename intrusive_traits_detect<Derived>::type::type;
}

/**
 * @brief similar to `boost::intrusive_ref_counter` this is a base class to derive
 * from if you want to manager your objects with `boost::intrusive_ptr`, but supports
 * custom allocators.
 *
 * @tparam Object         the object deriving from this class. if you want to derive from Object
 *                        as well, Object needs a virtual destructor
 * @tparam CounterPolicyT same as CounterPolicyT `boost::intrusive_ref_counter`
 * @tparam Allocator      allocator to allocate and deallocate objects with
 */
template <typename Object, typename CounterPolicyT /* = boost::thread_safe_counter */, typename Allocator /* = std::allocator<void> */>
class intrusive_base {
private:
	mutable impl::intrusive_counter<CounterPolicyT> m_counter;
	mutable impl::intrusive_allocator<Allocator> m_allocator;

	friend struct impl::intrusive_traits_impl<Object, CounterPolicyT, Allocator>;

protected:
	/**
	 * @brief allocator used to allocate this object. not valid during construction.
	 */
	Allocator const& allocator() const {
		return m_allocator.allocator();
	}

public:
	/**
	 * @brief how many pointers reference the object
	 */
	auto use_count() const -> decltype(m_counter.load()) {
		return m_counter.load();
	}

	/**
	 * @brief whether there is exactly one reference
	 */
	bool unique() const {
		return 1 == m_counter.load();
	}

	/**
	 * @brief shortcut for @ref allocate_intrusive(), but only allocates
	 * objects of types derived from `Object`.
	 *
	 * Creates objects of type `Object` by default, even when called through
	 * a derived class like `Derived::allocate` - call `Derived::allocate<Derived>`
	 * or `Object::allocate<Derived>` instead.
	 */
	template <typename Derived = Object, typename... Args>
	static boost::intrusive_ptr<Derived> allocate(Allocator const& alloc, Args&&... args);

	/**
	 * @brief shortcut for @ref make_intrusive(), but only creates
	 * objects of types derived from `Object`.
	 *
	 * Creates objects of type `Object` by default, even when called through
	 * a derived class like `Derived::allocate` - call `Derived::create<Derived>`
	 * or `Object::create<Derived>` instead.
	 */
	template <typename Derived = Object, typename... Args>
	static boost::intrusive_ptr<Derived> create(Args&&... args);
};

template <typename Object, typename CounterPolicyT, typename Allocator>
template <typename Derived>
/* static */
void impl::intrusive_traits_impl<Object, CounterPolicyT, Allocator>::add_ref(Derived* p) noexcept {
	p->base_t::m_counter.increment();
}

template <typename Object, typename CounterPolicyT, typename Allocator>
template <typename Derived>
/* static */
void impl::intrusive_traits_impl<Object, CounterPolicyT, Allocator>::release(Derived* p) noexcept {
	if (0 == p->base_t::m_counter.decrement()) {
		using derived_no_cv = typename std::remove_cv<Derived>::type;

		static_assert(
			std::is_same<derived_no_cv, object_t>::value || std::has_virtual_destructor<object_t>::value,
			"Can only derive from Object if it has a virtual destructor");

		using derived_alloc_t = typename std::allocator_traits<allocator_t>::template rebind_alloc<derived_no_cv>;
		using derived_alloc_traits = std::allocator_traits<derived_alloc_t>;
		derived_alloc_t derived_alloc(p->base_t::m_allocator.allocator());
		derived_no_cv* ptr = const_cast<derived_no_cv*>(p);

		p->base_t::m_allocator.clear_allocator();
		derived_alloc_traits::destroy(derived_alloc, ptr);
		derived_alloc_traits::deallocate(derived_alloc, ptr, 1);
	}
}

template <typename Object, typename CounterPolicyT, typename Allocator>
template <typename Derived>
/* static */
void impl::intrusive_traits_impl<Object, CounterPolicyT, Allocator>::init_allocator(Derived* p, Allocator const& alloc) noexcept {
	p->base_t::m_allocator.init_allocator(alloc);
}

/**
 * @brief helper function for boost::intrusive_ptr reference counter management
 * @internal
 */
template <typename Derived, typename traits = impl::intrusive_traits<Derived>>
void intrusive_ptr_add_ref(Derived* p) noexcept {
	traits::add_ref(p);
}

/**
 * @brief helper function for boost::intrusive_ptr reference counter management
 * @internal
 */
template <typename Derived, typename traits = impl::intrusive_traits<Derived>>
void intrusive_ptr_release(Derived* p) noexcept {
	traits::release(p);
}

/**
 * allocate object of type `Derived` (which must derive from @ref intrusive_base) and
 * return a `boost::intrusive_ptr` to it.
 * @param alloc    Allocator to allocate object with
 * @param args     Arguments to pass to `Derived` constructor
 * @tparam Derived type of object to create
 */
template <typename Derived, typename traits = impl::intrusive_traits<Derived>, typename... Args>
boost::intrusive_ptr<Derived> allocate_intrusive(typename traits::allocator_t const& alloc, Args&&... args) {
	using alloc_t = typename traits::allocator_t;
	using derived_alloc_t = typename std::allocator_traits<alloc_t>::template rebind_alloc<Derived>;
	using derived_alloc_traits = std::allocator_traits<derived_alloc_t>;
	derived_alloc_t derived_alloc(alloc);

	typename derived_alloc_traits::pointer ptr = derived_alloc_traits::allocate(derived_alloc, 1);
	typename traits::base_t* base_ptr = ptr;
	try {
		derived_alloc_traits::construct(derived_alloc, ptr, std::forward<Args>(args)...);
		traits::init_allocator(base_ptr, alloc);
	} catch (...) {
		derived_alloc_traits::deallocate(derived_alloc, ptr, 1);
		throw;
	}

	return boost::intrusive_ptr<Derived>(ptr);
}

/**
 * @brief create a new object of type `Derived` which must derive from @ref intrusive_base)
 * using a default constructed allocator.
 * @param args     Arguments to forward to contructor of `Derived`
 * @tparam Derived type of object to create
 */
template <typename Derived, typename traits = impl::intrusive_traits<Derived>, typename... Args>
boost::intrusive_ptr<Derived> make_intrusive(Args&&... args) {
	using alloc_t = typename traits::allocator_t;
	return allocate_intrusive<Derived>(alloc_t(), std::forward<Args>(args)...);
}

template <typename Object, typename CounterPolicyT, typename Allocator>
template <typename Derived, typename... Args>
/* static */
boost::intrusive_ptr<Derived> intrusive_base<Object, CounterPolicyT, Allocator>::allocate(Allocator const& alloc, Args&&... args) {
	static_assert(std::is_base_of<Object, Derived>::value, "only create derived objects");
	return allocate_intrusive<Derived>(alloc, std::forward<Args>(args)...);
}

template <typename Object, typename CounterPolicyT, typename Allocator>
template <typename Derived, typename... Args>
/* static */
boost::intrusive_ptr<Derived> intrusive_base<Object, CounterPolicyT, Allocator>::create(Args&&... args) {
	static_assert(std::is_base_of<Object, Derived>::value, "only create derived objects");
	return make_intrusive<Derived>(std::forward<Args>(args)...);
}

__CANEY_MEMORYV1_END
