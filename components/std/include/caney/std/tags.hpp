/** @file */

#pragma once

#include "internal.hpp"

__CANEY_STDV1_BEGIN

/**
 * this type can be used to mark constructors that should be protected/private
 * but can't (for various reasons: make_shared, ...).
 * Simply add an argument of type @ref private_tag_t .
 * Anyone calling a constructor with a @ref private_tag knows he did it wrong.
 *
 * Example:
 * @code
 *     class Object : public std::enable_shared_from_this<Object>, private boost::noncopyable {
 *     public:
 *         Object(private_tag_t);
 *         static std::shared_ptr<Object> create() {
 *             return std::make_shared<Object>(private_tag);
 *         }
 *     };
 * @endcode
 */
struct private_tag_t {
	//! @cond INTERNAL
	enum class tag_t { tag };
	explicit constexpr private_tag_t(tag_t) { };
	//! @endcond
};

/**
 * @brief @ref private_tag_t instance to call constructors with
 */
constexpr private_tag_t private_tag{private_tag_t::tag_t::tag};

/**
 * @brief tag type to signal constructing some internal element from following arguments
 */
struct in_place_t {
};

/**
 * @brief @ref in_place_t instance
 */
constexpr in_place_t in_place{};

__CANEY_STDV1_END
