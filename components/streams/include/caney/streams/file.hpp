#pragma once

#include "internal.hpp"
#include "unix_file.hpp"

__CANEY_STREAMSV1_BEGIN

/** @brief use unix file descriptors */
using file_descriptor = unix_file_descriptor;
/** @brief use unix file handles */
using file_handle = unix_file_handle;

__CANEY_STREAMSV1_END
