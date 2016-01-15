/** @file */

#pragma once

#include "const_buf.hpp"

__CANEY_MEMORYV1_BEGIN

/**
 * @brief implementation of @ref const_buf which keeps a (potentially
 *     slice) reference to another const_buf (which must be kept valid),
 *     inheriting its `internal_shared_slice` method.
 *
 * This is very handy in parsers when you want to pass on sliced
 * buffers.
 */
class tmp_const_buf final : public const_buf {
public:
	/**
	 * @brief empty buffer, not keeping a reference to another buffer
	 */
	explicit tmp_const_buf() = default;

	/**
	 * @brief reference full range of another buffer
	 */
	explicit tmp_const_buf(const_buf const& backend) : const_buf(backend), m_backend(&backend) {}

	/**
	 * @brief reference given `slice` of another buffer
	 * @pre `slice` must be a slice of the `backend`
	 */
	explicit tmp_const_buf(const_buf const& backend, const_buf const& slice) : const_buf(slice), m_backend(&backend) {}

	/** @brief copy constructor */
	tmp_const_buf(tmp_const_buf const&) = default;
	/** @brief copy assignment */
	tmp_const_buf& operator=(tmp_const_buf const& other) = default;

	/** @brief move constructor (cleans up original) */
	tmp_const_buf(tmp_const_buf&& other) : const_buf(other), m_backend(other.m_backend) {
		other.reset();
	}

	/** @brief move assignment (cleans up original) */
	tmp_const_buf& operator=(tmp_const_buf&& other) {
		m_backend = other.m_backend;
		const_buf::operator=(other);
		other.reset();
		return *this;
	}

	/**
	 * @brief reset to empty buffer
	 */
	void reset() {
		raw_reset();
		m_backend = nullptr;
	}

	/**
	 * @brief create a sub slice from current buffer
	 * @param from index to slice from (gets ranged clipped)
	 * @param size length to slice (gets ranged clipped)
	 */
	tmp_const_buf slice(size_type from, size_type size) const {
		return tmp_const_buf(*m_backend, raw_slice(from, size));
	}

	/**
	 * @brief alias for `slice(from, size())`
	 */
	tmp_const_buf slice(size_type from) const {
		return slice(from, size());
	}

	/**
	 * @brief alias for `slice(0, size())`
	 */
	tmp_const_buf copy() const {
		return slice(0, size());
	}

private:
	shared_const_buf internal_shared_slice(size_t from, size_t size) const override;

	const_buf const* m_backend{nullptr};
};

__CANEY_MEMORYV1_END
