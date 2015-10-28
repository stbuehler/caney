#pragma once

#include "caney/memory/buffer.hpp"
#include "caney/std/optional.hpp"

#include "file_size.hpp"
#include "internal.hpp"

#include <system_error>

__CANEY_STREAMSV1_BEGIN

class file_handle;

// TODO: this probably requires some log concept. use boost logging?

/**
 * @brief represents a chunk of a file on disk
 */
class file_chunk {
public:
	explicit file_chunk() = default;
	/** @brief construct @ref file_chunk from handle, offset and length */
	explicit file_chunk(std::shared_ptr<file_handle> handle, file_size offset, file_size length);

	/** @brief size of chunk in bytes */
	file_size bytes() const {
		return m_length;
	}

	/** @brief split chunk; return first "bytes" in new @ref file_chunk, remainder is kept in current chunk */
	file_chunk split(file_size bytes);
	/** @brief like split, but does not return anything; i.e. removes first "bytes" of data */
	void remove(file_size bytes);

	/** @brief direct buffer access is not available */
	caney::nullopt_t get_const_buffer() const {
		return caney::nullopt;
	}

	/** @brief get offset of start of the current chunk in file */
	file_size get_offset() const {
		return m_offset;
	}

	/** @brief get file handle */
	std::shared_ptr<file_handle> const& get_handle() const {
		return m_handle;
	}

	/** @brief (possibly asynchronous???) read some data from the chunk. TBD. */
	void read(std::size_t max_size, std::function<void(std::error_code ec, std::shared_ptr<memory::unique_buf> buffer)> callback);

private:
	std::shared_ptr<file_handle> m_handle;
	file_size m_offset;
	file_size m_length; // relative to offset
};

__CANEY_STREAMSV1_END
