/** @file */

#pragma once

#include "internal.hpp"

#include <system_error>

__CANEY_UTILV1_BEGIN

/**
  * @brief writes as many bytes as possible, returning the number of bytes written.
  * if (and only if) not all bytes could be written `ec` will contain the reason
  *
  * @param fd file descriptor to write to
  * @param buf start of memory to write
  * @param len length of data to write
  * @param[out] ec error code containing error if not all data could be written
  * @return number of bytes written
  *
  * retries on `errno == EINTR`
  */
std::size_t write_all(int fd, void const* buf, std::size_t len, std::error_code& ec);

__CANEY_UTILV1_END
