#pragma once

#include "caney/memory/buffer.hpp"

#include "file_size.hpp"

#include <system_error>

#include <boost/optional.hpp>

namespace caney {
	namespace streams {
		inline namespace v1 {
			// TODO: this probably requires some log concept. use boost logging?
			class file_handle;

			class file_chunk {
			public:
				explicit file_chunk() = default;
				explicit file_chunk(std::shared_ptr<file_handle> handle, file_size offset, file_size length);

				file_size bytes() const {
					return m_length;
				}

				file_chunk split(file_size bytes);

				void remove(file_size bytes);

				// direct buffer access is not available
				boost::none_t get_const_buffer() const {
					return boost::none;
				}

				file_size get_offset() const {
					return m_offset;
				}

				std::shared_ptr<file_handle> const& get_handle() const {
					return m_handle;
				}

				void read(std::size_t max_size, std::function<void(std::error_code ec, std::shared_ptr<memory::unique_buf> buffer)> callback);

			private:
				std::shared_ptr<file_handle> m_handle;
				file_size m_offset;
				file_size m_length; // relative to offset
			};
		}
	} // namespace streams
} // namespace caney
