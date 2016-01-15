#include "caney/memory/const_buf.hpp"

#include "caney/memory/shared_const_buf.hpp"
#include "caney/memory/unique_buf.hpp"

#include <boost/asio/buffer.hpp>

__CANEY_MEMORYV1_BEGIN

/* const_buf */

shared_const_buf const_buf::shared_copy() const {
	return shared_slice(0, m_size);
}

shared_const_buf const_buf::shared_slice(size_t from, size_t size) const {
	from = std::min(from, m_size);
	size = std::min(m_size - from, size);
	return internal_shared_slice(from, size);
}

shared_const_buf const_buf::shared_slice(size_t from) const {
	return shared_slice(from, m_size);
}

unique_buf const_buf::copy() const {
	return unique_buf::copy(*this);
}

shared_const_buf const_buf::internal_shared_slice(size_t from, size_t size) const {
	return shared_const_buf::copy(m_data + from, size);
}

const_buf::operator boost::asio::const_buffer() const {
	return boost::asio::const_buffer(m_data, m_size);
}

/* raw_const_buf */

raw_const_buf::raw_const_buf(boost::asio::const_buffer data)
: raw_const_buf(boost::asio::buffer_cast<unsigned char const*>(data), boost::asio::buffer_size(data)) {}

raw_const_buf::raw_const_buf(boost::asio::mutable_buffer data)
: raw_const_buf(boost::asio::buffer_cast<unsigned char const*>(data), boost::asio::buffer_size(data)) {}

__CANEY_MEMORYV1_END
