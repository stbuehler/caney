#pragma once

namespace caney {
	inline namespace stdv1 {
		/* base class to enforce virtual destructor; should be inherited
		* virtual; type shouldn't be used directly, so destructor is hidden.
		* copy and move probably doesn't make sense at all, require user to
		* override it if needed.
		*/
		class object {
		protected:
			object() = default;
			virtual ~object() = default;

			object(const object&) = delete;
			object& operator=(const object&) = delete;
			object(object&&) = delete;
			object&& operator=(object&&) = delete;
		};
	}
} // namespace caney
