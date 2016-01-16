#pragma once

#include "caney/std/tags.hpp"

#include "file_size.hpp"
#include "internal.hpp"

#include <memory>
#include <sys/stat.h>
#include <system_error>

#include <boost/noncopyable.hpp>

__CANEY_STREAMSV1_BEGIN

/** @brief manages a file descriptor (either representing an open file or "-1") */
class unix_file_descriptor final : private boost::noncopyable {
public:
	/** @brief construct without a file (=> -1) */
	explicit unix_file_descriptor() = default;

	/** @brief construct from file descriptor */
	explicit unix_file_descriptor(int native) : m_native(native) {}

	/** @brief move construct */
	unix_file_descriptor(unix_file_descriptor&& other) : m_native(other.release()) {}

	/** @brief move assign */
	unix_file_descriptor& operator=(unix_file_descriptor&& other);

	~unix_file_descriptor();

	/** @brief file descriptor of open file or -1 */
	int native() const {
		return m_native;
	}

	/** @brief close file */
	void close();

	/** @brief release ownership and return file descriptor of open file or -1 */
	int release();

	/** @brief whether it represents an open file */
	explicit operator bool() const {
		return -1 != m_native;
	}

private:
	int m_native{-1};
};

/** @brief manages a regular (possibly temporary) opened file with name and stat */
class unix_file_handle : public std::enable_shared_from_this<unix_file_handle>, private boost::noncopyable {
private:
	//! @cond INTERNAL
	struct info : private boost::noncopyable {
		explicit info(std::string const& filename, struct ::stat const& st, bool temporary);
		~info();

		std::string const m_filename;
		struct ::stat const m_st;
		bool const m_temporary{false};
	};
	//! @endcond INTERNAL

public:
	/** @brief file descriptor type */
	using file_descriptor_t = unix_file_descriptor;

	//! @cond INTERNAL
	explicit unix_file_handle(private_tag_t, file_descriptor_t&& fd, std::string const& filename, struct ::stat const& st, bool temporary);
	explicit unix_file_handle(private_tag_t, file_descriptor_t&& fd, std::shared_ptr<info> const& parentInfo);
	//! @endcond INTERNAL

	/** @brief file descriptor */
	file_descriptor_t const& file_descriptor() const {
		return m_fd;
	}

	/** @brief filename of managed file */
	std::string const& filename() const {
		return m_info->m_filename;
	}

	/** @brief result of fstat after file was opened (might be outdated) */
	struct stat const& stat() const {
		return m_info->m_st;
	}

	/**
	 * @brief whether file is temporary and will be deleted when all file handles
	 * referencing it are gone.
	 */
	bool is_temporary() const {
		return m_info->m_temporary;
	}

	/** @brief create a new file handle with an independent @ref file_descriptor() */
	std::shared_ptr<unix_file_handle> duplicate(std::error_code& ec) const;

	static std::shared_ptr<unix_file_handle> open_file(std::string const& filename, std::error_code& ec);

	static std::shared_ptr<unix_file_handle> open_temporary_file(std::string const& filename, std::error_code& ec);

private:
	file_descriptor_t const m_fd;
	std::shared_ptr<info> const m_info;
};

__CANEY_STREAMSV1_END
