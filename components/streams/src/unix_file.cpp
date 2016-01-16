#include "caney/streams/unix_file.hpp"

#include "caney/std/error_code.hpp"

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
unix_file_handle::info::info(std::__cxx11::string const& filename, struct ::stat const& st, bool temporary)
: m_filename(filename), m_st(st), m_temporary(temporary) {}

unix_file_handle::info::~info() {
	if (m_temporary) {
		::unlink(m_filename.c_str());
	}
}

unix_file_handle::unix_file_handle(
	private_tag_t,
	unix_file_handle::file_descriptor_t&& fd,
	const std::__cxx11::string& filename,
	const struct ::stat& st,
	bool temporary)
: m_fd(std::move(fd)), m_info(std::make_shared<info>(filename, st, temporary)) {}

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

__CANEY_STREAMSV1_END
