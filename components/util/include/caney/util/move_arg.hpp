#pragma once

#include <utility>

namespace caney {
	namespace util {
		inline namespace v1 {
			/* use as wrapper for argument types in function<...> to enforce moving it
			 * when calling it or binding it.
			 * calling twice after binding such argument will lead to unexpected results
			 *
			 * use wrap_dispatch/wrap_post from dispatcher.hpp instead if possible.
			 */
			template<typename T>
			class move_arg {
			public:
				using value_type = T;

				/* implicit for automatic conversion on calling */
				move_arg(value_type&& value)
				: m_value(std::move(value)) {
				}

				move_arg(move_arg const& other)
				: m_value(std::move(other.m_value)) {
				}

				move_arg(move_arg&& other)
				: m_value(std::move(other.m_value)) {
				}

				/* only require CopyConstructible on arguments, not CopyAssignable */
				move_arg& operator=(move_arg const&) = delete;
				move_arg& operator=(move_arg&&) = delete;

				/* implicit for automatic conversion on calling the final target */
				operator value_type() const {
					return std::move(m_value);
				}

				void set(value_type&& value) const {
					m_value = std::move(value);
				}

				value_type& get() const {
					return m_value;
				}

				value_type extract() const {
					return std::move(m_value);
				}

			private:
				mutable value_type m_value;
			};

		}
	} // namespace util
} // namespace caney
