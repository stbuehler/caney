#pragma once

#include "weak_fn.hpp"

#include <boost/smart_ptr/shared_ptr.hpp>
#include <boost/smart_ptr/weak_ptr.hpp>

namespace caney {
	inline namespace stdv1 {
		namespace weakfn_traits {
			// add support for boost::shared_ptr/weak_ptr
			template<typename Object>
			struct shared_ptr_traits<boost::shared_ptr<Object>> {
				template<typename T>
				using shared_ptr = boost::shared_ptr<T>;

				template<typename T>
				using weak_ptr = boost::weak_ptr<T>;

				using object_t = Object;

				using pointer_t = shared_ptr<object_t>;
				using weak_pointer_t = weak_ptr<object_t>;
			};

			template<typename Object>
			struct weak_ptr_traits<boost::weak_ptr<Object>> : shared_ptr_traits<boost::shared_ptr<Object>> {
			};
		} // namespace weakfn_traits
	}
} // namespace caney
