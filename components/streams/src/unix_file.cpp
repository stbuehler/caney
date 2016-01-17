#include "caney/streams/unix_file.hpp"

#include "caney/std/error_code.hpp"
#include "caney/std/optional.hpp"

#include <cassert>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

__CANEY_STREAMSV1_BEGIN

unix_file_descriptor& unix_file_descriptor::operator=(unix_file_descriptor&& other) {
	if (this != &other) {
		close();
		m_native = other.release();
	}
	return *this;
}

unix_file_descriptor::~unix_file_descriptor() {
	close();
}

void unix_file_descriptor::close() {
	if (*this) {
		::close(m_native);
		m_native = -1;
	}
}

int unix_file_descriptor::release() {
	int fd = m_native;
	m_native = -1;
	return fd;
}

//! @cond INTERNAL
unix_file_handle::info::info(std::string&& filename, struct ::stat const& st, bool temporary)
: m_filename(std::move(filename)), m_st(st), m_temporary(temporary) {}

unix_file_handle::info::~info() {
	if (m_temporary) { ::unlink(m_filename.c_str()); }
}

unix_file_handle::unix_file_handle(
	private_tag_t,
	unix_file_handle::file_descriptor_t&& fd,
	std::string&& filename,
	const struct ::stat& st,
	bool temporary)
    : m_fd(std::move(fd)), m_info(std::make_shared<info>(std::move(filename), st, temporary)) {}

unix_file_handle::unix_file_handle(private_tag_t, unix_file_handle::file_descriptor_t&& fd, std::shared_ptr<unix_file_handle::info> const& parentInfo)
: m_fd(std::move(fd)), m_info(parentInfo) {}
//! @endcond INTERNAL

std::shared_ptr<unix_file_handle> unix_file_handle::duplicate(std::error_code& ec) const {
	file_descriptor_t new_fd{::dup(m_fd.native())};
	if (!new_fd) {
		ec = errno_error_code();
		return nullptr;
	}

	return std::make_shared<unix_file_handle>(private_tag, std::move(new_fd), m_info);
}

namespace {
#if !defined(O_NOFOLLOW)
#error O_NOFOLLOW is required for safe symlinks
#endif

	const int open_flags = O_RDONLY
#if defined(O_BINARY)
		| O_BINARY
#endif
#if defined(O_NONBLOCK)
		| O_NONBLOCK /* don't block on opening named files */
#endif
#if defined(O_NOCTTY)
		| O_NOCTTY /* don't allow overtaking controlling terminal */
#endif
		;
	const int open_dirflags = O_DIRECTORY
#if defined(O_SEARCH)
		| O_SEARCH
#endif
		;

	unix_file_descriptor open_file_nofollow(std::string filename, std::error_code& ec) {
		unix_file_descriptor parent;
		char* current;

		if ('/' == filename[0]) {
			parent = unix_file_descriptor(::open("/", open_dirflags));
			current = &filename[0] + 1;
		} else {
			parent = unix_file_descriptor(::open(".", open_dirflags));
			current = &filename[0];
		}
		if (!parent) {
			/* this shouldn't happen */
			ec = errno_error_code();
			return unix_file_descriptor();
		}

		char* next;
		while (nullptr != (next = ::strchr(current, '/'))) {
			*next = '\0';

			if (0 == ::strcmp(current, "..")) {
				/* path traversal not allowed */
				ec = errno_error_code(EACCES);
				return unix_file_descriptor();
			}
			if ('\0' == current[0] || 0 == ::strcmp(current, ".")) {
				/* '' or '.': skip */
				current = next + 1;
				continue;
			}

			unix_file_descriptor fd{::openat(parent.native(), current, open_dirflags | O_NOFOLLOW)};
			if (!fd) {
				ec = errno_error_code();
				return unix_file_descriptor();
			}

			parent = std::move(fd);

			current = next + 1;
		}

		if ('\0' == current[0] || 0 == strcmp(current, ".") || 0 == strcmp(current, "..")) {
			ec = errno_error_code(EACCES);
			return unix_file_descriptor();
		}

		unix_file_descriptor fd{::openat(parent.native(), current, open_flags | O_NOFOLLOW)};
		if (!fd) {
			ec = errno_error_code();
			return unix_file_descriptor();
		}

		return fd;
	}

	unix_file_descriptor open_file_follow(std::string const& filename, std::error_code& ec) {
		const int flags = O_RDONLY
#if defined(O_BINARY)
			| O_BINARY
#endif
#if defined(O_NONBLOCK)
			| O_NONBLOCK /* don't block on opening named files */
#endif
#if defined(O_NOCTTY)
			| O_NOCTTY /* don't allow overtaking controlling terminal */
#endif
			;
		unix_file_descriptor fd{::open(filename.c_str(), flags)};
		if (!fd) {
			ec = errno_error_code();
			return unix_file_descriptor();
		}
		return fd;
	}

	std::shared_ptr<unix_file_handle> open_file_impl(std::string filename, symlink_policy policy, bool temporary, std::error_code& ec) {
		unix_file_descriptor fd;
		switch (policy) {
		case symlink_policy::no_follow:
			fd = open_file_nofollow(filename, ec);
			break;
		case symlink_policy::follow:
			fd = open_file_follow(filename, ec);
			break;
		}
		if (ec) return nullptr;
		assert(fd);
		if (!fd) {
			ec = errno_error_code(EINVAL);
			return nullptr;
		}

		struct ::stat st;
		if (0 != ::fstat(fd.native(), &st)) {
			ec = errno_error_code();
			return nullptr;
		}

		TODO

		return std::make_shared<unix_file_handle>(private_tag, std::move(fd), filename, st, temporary);
	}
}

std::shared_ptr<unix_file_handle> unix_file_handle::open_file(std::string const& filename, symlink_policy policy, std::error_code& ec) {
	return open_file_impl(filename, policy, /* temporary: */ false, ec);
}

std::shared_ptr<unix_file_handle> unix_file_handle::open_temporary_file(std::string const& filename, symlink_policy policy, std::error_code& ec) {
	return open_file_impl(filename, policy, /* temporary: */ true, ec);
}

__CANEY_STREAMSV1_END
