#include "caney/streams/file_chunk.hpp"

__CANEY_STREAMSV1_BEGIN

file_chunk::file_chunk(std::shared_ptr<file_handle> handle, file_size offset, file_size length)
: m_handle(handle), m_offset(offset), m_length(length) {}

file_chunk file_chunk::split(file_size bytes) {
	if (bytes > m_length) std::terminate();
	file_chunk result(m_handle, m_offset, bytes);
	m_offset += bytes;
	m_length -= bytes;
	return result;
}

void file_chunk::remove(file_size bytes) {
	if (bytes > m_length) std::terminate();
	m_offset += bytes;
	m_length -= bytes;
}

void file_chunk::read(std::size_t max_size, std::function<void(std::error_code ec, std::shared_ptr<memory::unique_buf> buffer)> callback) {
	if (file_size{max_size} > m_length) max_size = boost::numeric_cast<std::size_t>(m_length.get());
	if (0 == max_size) {
		// callback({}, memory::make_unique_buffer<0>());
	}
}

__CANEY_STREAMSV1_END
