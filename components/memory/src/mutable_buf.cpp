#include "caney/memory/mutable_buf.hpp"

#include "caney/memory/unique_buf.hpp"

#include <boost/asio/buffer.hpp>

__CANEY_MEMORYV1_BEGIN

/* mutable_buf */

mutable_buf::operator boost::asio::const_buffer() const {
	return boost::asio::const_buffer(data(), size());
}

mutable_buf::operator boost::asio::mutable_buffer() const {
	return boost::asio::mutable_buffer(data(), size());
}

unique_buf mutable_buf::copy() const {
	return unique_buf::copy(*this);
}

/* raw_mutable_buf */

raw_mutable_buf::raw_mutable_buf(boost::asio::mutable_buffer data)
: raw_mutable_buf(boost::asio::buffer_cast<unsigned char*>(data), boost::asio::buffer_size(data)) {
}

__CANEY_MEMORYV1_END
