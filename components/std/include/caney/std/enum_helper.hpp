#pragma once

#include <type_traits>


namespace caney {
	inline namespace stdv1 {
		/*
		 * convert enum value to underlying integer type
		 *
		 * unless given in enum declaration the underlying type is implementation
		 * defined
		 */
		template<typename Enum>
		constexpr typename std::underlying_type<Enum>::type from_enum(Enum val) {
			return static_cast<typename std::underlying_type<Enum>::type>(val);
		}

		/*
		 * convert integer value to given enum type; requires that the value
		 * is implicitly convertible to the underlying type
		 */
		template<typename Enum>
		constexpr Enum to_enum(typename std::underlying_type<Enum>::type val) {
			return static_cast<Enum>(val);
		}

		namespace impl {
			template<typename Integral>
			class to_enum_wrapper {
			public:
				constexpr explicit to_enum_wrapper(Integral value)
				: m_value(value) {
				}

				// implicit cast operator!
				template<typename Enum>
				operator Enum() {
					return to_enum<Enum>(m_value);
				}

			private:
				Integral m_value;
			};
		}

		/*
		 * if you have the correct underlying type (integer literals might not work, you want an explicit type!),
		 * this variant doesn't need the enum type as template parameter if the returned value
		 * is converted into the wanted enum type automatically.
		 *
		 * Example:
		 *     enum class MyEnum : uint32_t { ... };
		 *     uint32_t i = 0; // e.g. deserialize from somewhere, sql query result, ...
		 *     MyEnum t = to_enum(i);
		 */
		template<typename Integral, std::enable_if_t<std::is_integral<Integral>::value>* = nullptr>
		constexpr impl::to_enum_wrapper<Integral> to_enum(Integral value) {
			return impl::to_enum_wrapper<Integral>(value);
		}
	}
} // namespace caney
