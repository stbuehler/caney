#include "caney/memory/buffer.hpp"

#include <cstring>

#include <boost/asio/buffer.hpp>

namespace caney {
	namespace memory {
		inline namespace v1 {
			namespace {
				std::shared_ptr<unsigned char> allocate_storage(std::size_t size) {
					if (0 == size) return nullptr;
					return std::shared_ptr<unsigned char>(new unsigned char[size], [](unsigned char* ptr) { delete[] ptr; });
				}
			} // anonymous namespace

			const_buf::operator boost::asio::const_buffer() const {
				return boost::asio::const_buffer(m_data, m_size);
			}

			raw_const_buf::raw_const_buf(boost::asio::const_buffer data)
			: raw_const_buf(boost::asio::buffer_cast<unsigned char const*>(data), boost::asio::buffer_size(data)) {
			}

			raw_const_buf::raw_const_buf(boost::asio::mutable_buffer data)
			: raw_const_buf(boost::asio::buffer_cast<unsigned char const*>(data), boost::asio::buffer_size(data)) {
			}

			// static
			shared_const_buf shared_const_buf::copy(unsigned char const* data, std::size_t size) {
				if (size > 0) {
					std::shared_ptr<unsigned char> storage = allocate_storage(size);
					std::memcpy(storage.get(), data, size);
					return shared_const_buf::unsafe_use(storage, raw_const_buf(storage.get(), size));
				} else {
					return shared_const_buf();
				}
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

			// static
			unique_buf unique_buf::copy(unsigned char const* data, std::size_t size) {
				if (size > 0) {
					unique_buf result = make_unique_buffer(size);
					std::memcpy(result.data(), data, size);
					return result;
				} else {
					return unique_buf();
				}
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

			unique_buf make_unique_buffer(std::size_t size) {
				std::shared_ptr<unsigned char> storage = allocate_storage(size);
				return unique_buf(storage, storage.get(), size);
			}
		}
	}
}
