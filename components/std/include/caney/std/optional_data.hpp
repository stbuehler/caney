/**
 * @file
 */

#pragma once

#include "internal.hpp"
#include "optional_ptr.hpp"

__CANEY_STDV1_BEGIN

/**
 * pick @ref optional or @ref optional_ptr depending on the size of @ref optional;
 * the goal is to embed small @ref optional instances and use pointers for larger objects
 */
template<typename ValueType, size_t EmbeddedSizeLimit = 4*sizeof(void*)>
using optional_data = typename std::conditional<sizeof(optional<ValueType>) <= EmbeddedSizeLimit, optional<ValueType>, optional_ptr<ValueType>>::type;

__CANEY_STDV1_END
