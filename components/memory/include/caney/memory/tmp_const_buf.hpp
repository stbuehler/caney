/** @file */

#pragma once

#include "const_buf.hpp"

__CANEY_MEMORYV1_BEGIN

// (potentially sliced) reference to a const_buf, inheriting the original internal_shared_slice() method
class tmp_const_buf final: public const_buf {
public:
	explicit tmp_const_buf() = default;

	explicit tmp_const_buf(const_buf const& backend)
	: m_backend(&backend) {
		raw_set(backend);
	}

	explicit tmp_const_buf(const_buf const& backend, const_buf const& slice)
	: m_backend(&backend) {
		raw_set(slice);
	}

	tmp_const_buf(tmp_const_buf const& other)
	: const_buf(), m_backend(other.m_backend) {
		raw_set(other);
	}
	tmp_const_buf& operator=(tmp_const_buf const& other) {
		m_backend = other.m_backend;
		raw_set(other);
		return *this;
	}
	tmp_const_buf(tmp_const_buf&& other)
	: const_buf(), m_backend(other.m_backend) {
		raw_set(other);
		other.reset();
	}
	tmp_const_buf& operator=(tmp_const_buf&& other) {
		m_backend = other.m_backend;
		raw_set(other);
		other.reset();
		return *this;
	}

	void reset() {
		raw_reset();
		m_backend = nullptr;
	}

	tmp_const_buf slice(size_type from, size_type size) const {
		return tmp_const_buf(*m_backend, raw_slice(from, size));
	}
	tmp_const_buf slice(size_type from) const {
		return slice(from, size());
	}
	tmp_const_buf copy() const {
		return slice(0, size());
	}

private:
	shared_const_buf internal_shared_slice(size_t from, size_t size) const override;

	const_buf const* m_backend{nullptr};
};

__CANEY_MEMORYV1_END
