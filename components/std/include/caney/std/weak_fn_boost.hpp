/** @file */

#pragma once

#include "internal.hpp"
#include "weak_fn.hpp"

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>

__CANEY_STDV1_BEGIN

/**
 * @addtogroup weak_fn
 * @{
 */

namespace weakfn_traits {
	/**
	 * @addtogroup weakfn_traits
	 * @{
	 */

	namespace impl {
		template<typename Object>
		struct boost_ptr_traits {
			template<typename T>
			using shared_ptr = boost::shared_ptr<T>;

			template<typename T>
			using weak_ptr = boost::weak_ptr<T>;

			using object_t = Object;

			using pointer_t = shared_ptr<object_t>;
			using weak_pointer_t = weak_ptr<object_t>;
		};
	}

	/**
	  * @brief support `boost::shared_ptr<Object>` for all `Object` types in `shared_ptr_traits`
	  */
	template<typename Object>
	struct shared_ptr_traits<boost::shared_ptr<Object>> : impl::boost_ptr_traits<Object> {
	};

	/**
	  * @brief support `boost::weak_ptr<Object>` for all `Object` types in `shared_ptr_traits`
	  */
	template<typename Object>
	struct weak_ptr_traits<boost::weak_ptr<Object>> : impl::boost_ptr_traits<Object> {
	};

	/** @} */
} // namespace weakfn_traits

/** @} */

__CANEY_STDV1_END
