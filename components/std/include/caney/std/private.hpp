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
class private_tag_t {
private:
	friend private_tag_t make_private_tag();
	private_tag_t() = default;
};

/**
 * @brief @ref private_tag_t instance to call constructors with
 */
extern private_tag_t private_tag;

__CANEY_STDV1_END
