/** @file */

#pragma once

#include "internal.hpp"

#include <type_traits>

__CANEY_STDV1_BEGIN

/**
 * @defgroup enum_helper enum_helper
 *
 * @brief Helper functions to deal with conversions between enum and underlying integral types.
 *
 * Unless given in enum declaration the underlying type is implementation
 * defined. You should use explicit underlying types if you want to serialize
 * the value (network, IPC, storing in files).
 *
 * Example (unscoped enums, C++-11 for underlying type, might also work with plain C / Objective-C):
 * @code
 *     enum MyEnum : uint32_t { MyEnum_First, MyEnum_Second };
 * @endcode
 *
 * Example (scoped enums, C++-11):
 * @code
 *     enum class MyEnum : uint32_t { First, Second };
 * @endcode
 *
 * @addtogroup enum_helper
 * @{
 */

/**
 * @brief convert enum value to underlying integer type
 *
 * @param val enum value to convert to underlying integer type
 * @return val casted to underlying integer type
 *
 * Example (scoped enums, C++-11):
 * @code
 *     enum class MyEnum : uint32_t { First, Second };
 *     auto i = from_enum(MyEnum::First);
 *     // typeof(i) == uint32_t
 * @endcode
 */
template <typename Enum>
constexpr typename std::underlying_type<Enum>::type from_enum(Enum val) {
	return static_cast<typename std::underlying_type<Enum>::type>(val);
}

/**
 * @brief convert integer value to given enum type; makes sure the type of the
 * given value is the underyling type of the enum (or implicitly convertible on calling)
 *
 * See @ref from_enum for specifying underling types.
 *
 * Example (scoped enums, C++-11):
 * @code
 *     enum class MyEnum : uint32_t { First, Second };
 *     uint32_t i = 0; // some deserialize result
 *     auto e = to_enum<MyEnum>(i);
 *     // typeof(e) == MyEnum
 * @endcode
 */
template <typename Enum>
constexpr Enum to_enum(typename std::underlying_type<Enum>::type val) {
	return static_cast<Enum>(val);
}

namespace impl {
	template <typename Integral>
	class to_enum_wrapper {
	public:
		constexpr explicit to_enum_wrapper(Integral value) : m_value(value) {}

		// implicit cast operator!
		template <typename Enum>
		operator Enum() {
			return to_enum<Enum>(m_value);
		}

	private:
		Integral m_value;
	};
}

#if defined(DOXYGEN)
/**
 * @brief if you have the correct underlying type (integer literals might not work, you want an explicit type!),
 * this variant of @ref to_enum doesn't need the enum type as template parameter if the returned value
 * is converted into the wanted enum type automatically.
 *
 * Example:
 * @code
 *     enum class MyEnum : uint32_t { First, Second };
 *     uint32_t i = 0; // some deserialize result
 *     MyEnum e = to_enum(i);
 * @endcode
 */
template <typename Integral, std::enable_if_t<std::is_integral<Integral>::value>* = nullptr>
constexpr impl_defined to_enum(Integral value);
#else
template <typename Integral, std::enable_if_t<std::is_integral<Integral>::value>* = nullptr>
constexpr impl::to_enum_wrapper<Integral> to_enum(Integral value) {
	return impl::to_enum_wrapper<Integral>(value);
}
#endif

/** @} */

__CANEY_STDV1_END
