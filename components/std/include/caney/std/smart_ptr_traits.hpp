/** @file */

#pragma once

#include "internal.hpp"

#include <memory>

__CANEY_STDV1_BEGIN

/** @namespace caney::util::v1::smart_ptr_traits */
namespace smart_ptr_traits {
	/** @defgroup smart_ptr_traits traits for smart/shared pointers
	 *
	 * @ref weak_fn uses @ref shared_ptr_traits and @ref weak_ptr_traits to handle
	 * different `shared_ptr` implementations; the basic requirement is that such
	 * implementation provides strong (`shared_ptr`) and weak (`weak_ptr`) smart references,
	 * where strong references keep objects alive and weak references don't; weak references
	 * are supposed to construct from strong references, and weak references can be `locked`
	 * to either get an empty strong reference or the original strong reference if the object
	 * is still alive.
	 *
	 * The @ref shared_ptr_traits (and @ref weak_ptr_traits) take a strong (or weak) reference type
	 * and are supposed to know the following types:
	 * - `shared_ptr` - a template taking one type argument for the object a strong reference points to
	 * - `weak_ptr` - a template taking one type argument for the object a weak reference points to
	 * - `object_t` - the type the pointer given to the traits template points to
	 * - `pointer_t` - `shared_ptr<object_t>`
	 * - `weak_pointer_t` - `weak_ptr<object_t>`
	 *
	 * @addtogroup smart_ptr_traits
	 * @{
	 */

	/**
	 * @brief traits to handle a strong reference type
	 * @tparam Ptr a strong reference to some object, for example `std::shared_ptr<uint32_t>`
	 */
	template <typename Ptr>
	struct shared_ptr_traits;

	/**
	 * @brief traits to handle a weak reference type
	 * @tparam Ptr a weak reference to some object, for example `std::weak_ptr<uint32_t>`
	 */
	template <typename Ptr>
	struct weak_ptr_traits;

	namespace impl {
		template <typename Object>
		struct std_ptr_traits {
			template <typename T>
			using shared_ptr = std::shared_ptr<T>;

			template <typename T>
			using weak_ptr = std::weak_ptr<T>;

			using object_t = Object;

			using pointer_t = shared_ptr<object_t>;

			using weak_pointer_t = weak_ptr<object_t>;
		};
	}

	/**
	  * @brief support `std::shared_ptr<Object>` for all `Object` types in `shared_ptr_traits`
	  */
	template <typename Object>
	struct shared_ptr_traits<std::shared_ptr<Object>> : impl::std_ptr_traits<Object> {};

	/**
	  * @brief support `std::weak_ptr<Object>` for all `Object` types in `weak_ptr_traits`
	  */
	template <typename Object>
	struct weak_ptr_traits<std::weak_ptr<Object>> : impl::std_ptr_traits<Object> {};
	/** @} */
} // namespace smart_ptr_traits

__CANEY_STDV1_END
