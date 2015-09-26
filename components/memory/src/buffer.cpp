#include "caney/memory/buffer.hpp"

#include <cstring>

#include <boost/asio/buffer.hpp>

namespace caney {
	namespace memory {
		inline namespace v1 {
			const_buf::operator boost::asio::const_buffer() const {
				return boost::asio::const_buffer(m_data, m_size);
			}

			raw_const_buf::raw_const_buf(boost::asio::const_buffer data)
			: raw_const_buf(boost::asio::buffer_cast<unsigned char const*>(data), boost::asio::buffer_size(data)) {
			}

			raw_const_buf::raw_const_buf(boost::asio::mutable_buffer data)
			: raw_const_buf(boost::asio::buffer_cast<unsigned char const*>(data), boost::asio::buffer_size(data)) {
			}

			mutable_buf::operator boost::asio::const_buffer() const {
				return boost::asio::const_buffer(data(), size());
			}

			mutable_buf::operator boost::asio::mutable_buffer() const {
				return boost::asio::mutable_buffer(data(), size());
			}

			raw_mutable_buf::raw_mutable_buf(boost::asio::mutable_buffer data)
			: raw_mutable_buf(boost::asio::buffer_cast<unsigned char*>(data), boost::asio::buffer_size(data)) {
			}

			shared_const_buf unique_buf::freeze(std::size_t size) {
				shared_const_buf result = shared_const_buf::unsafe_use(m_storage, raw_slice(0, size));
				raw_set(raw_slice(size));
				return result;
			}

			shared_const_buf unique_buf::freeze() {
				shared_const_buf result = shared_const_buf::unsafe_use(m_storage, raw_copy());
				m_storage.reset();
				raw_reset();
				return result;
			}
		}
	} // namespace memory
} // namespace caney
