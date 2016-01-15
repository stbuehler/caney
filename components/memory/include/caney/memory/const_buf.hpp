/** @file */

#pragma once

#include "buffer_prototypes.hpp"
#include "buffer_storage.hpp"

#include <memory>

__CANEY_MEMORYV1_BEGIN

/**
 * @brief represents an unmodifieable contiguous byte buffer
 *
 * this is a "abstract" base class. implementations are:
 * - @ref raw_const_buf
 * - @ref tmp_const_buf
 * - @ref shared_const_buf
 */
class const_buf {
public:
	/**
	 * @{
	 * @brief standard container typedef
	 */
	typedef unsigned char const* iterator;
	typedef unsigned char const* const_iterator;
	typedef unsigned char value_type;
	typedef size_t size_type;
	typedef std::ptrdiff_t difference_type;
	/** @} */

	/**
	 * @brief size of buffer (length in bytes)
	 */
	size_t size() const {
		return m_size;
	}

	/**
	 * @brief whether buffer is empty (i.e. zero length)
	 */
	bool empty() const {
		return 0 == m_size;
	}

	/**
	 * @brief whether buffer is not empty (i.e. nonzero length)
	 */
	explicit operator bool() const {
		return !empty();
	}

	/**
	 * @brief pointer to first byte of data
	 */
	unsigned char const* data() const {
		return m_data;
	}

	/**
	 @{
	 * @brief standard iterator getter
	 */
	const_iterator begin() const {
		return m_data;
	}
	const_iterator end() const {
		return m_data + m_size;
	}
	const_iterator cbegin() const {
		return m_data;
	}
	const_iterator cend() const {
		return m_data + m_size;
	}
	/** @} */

	/**
	 * @brief similar to begin() but returns iterator for `char` access
	 *     instead of byte (unsigned char)
	 */
	char const* char_begin() const {
		return reinterpret_cast<char const*>(m_data);
	}

	/**
	 * @brief similar to end() but returns iterator for `char` access
	 *     instead of byte (unsigned char)
	 */
	char const* char_end() const {
		return reinterpret_cast<char const*>(m_data) + m_size;
	}

	/**
	 * @brief return byte value at position `ndx` (terminates if range
	 *     check fails)
	 * @param ndx index to read byte from
	 */
	unsigned char operator[](size_t ndx) const {
		if (ndx >= m_size) std::terminate();
		return m_data[ndx];
	}

	/**
	 * @brief implicit cast to `boost::asio::const_buffer` which just
	 *     contains a raw pointer to the data and the size
	 */
	operator boost::asio::const_buffer() const;

	/**
	 * @brief create a (possible shared) copy of a slice of the data in
	 *     a `shared_const_buf` which guarantess the buffer is kept
	 *     alive.
	 *
	 * if the buffer is shared no one else is supposed to have write
	 *     access.
	 *
	 * NOTE: a small slice might keep a large buffer alive.
	 *
	 * @param from index to copy data from (gets ranged clipped)
	 * @param size how many bytes to copy (gets ranged clipped)
	 */
	shared_const_buf shared_slice(size_t from, size_t size) const;

	/** @brief alias for `shared_slice(from, size())` */
	shared_const_buf shared_slice(size_t from) const;

	/** @brief alias for `shared_slice(0, size())` */
	shared_const_buf shared_copy() const;

	/**
	 * @brief create a raw reference to a slice of the data. the raw
	 *     reference is only valid as long as the original buffer
	 *     keeping the data alive is.
	 *
	 * @param from index to copy data from (gets ranged clipped)
	 * @param size how many bytes to copy (gets ranged clipped)
	 */
	raw_const_buf raw_slice(size_t from, size_t size) const;

	/** @brief alias for `raw_slice(from, size())` */
	raw_const_buf raw_slice(size_t from) const;

	/** @brief alias for `raw_slice(0, size())` */
	raw_const_buf raw_copy() const;

	/**
	 * @brief create a unique copy of the data. if you only need the copy
	 * of a slice just call @ref copy() on a @ref raw_slice().
	 */
	unique_buf copy() const;

protected:
	/** default construct empty buffer */
	const_buf() = default;

	/** initialize with raw data */
	explicit const_buf(unsigned char const* data, size_t size) : m_data(data), m_size(size) {}

	/** initialize with raw data */
	explicit const_buf(char const* data, size_t size) : const_buf(reinterpret_cast<unsigned char const*>(data), size) {}

	/**
	 * destructor is not virtual as this class is supposed to be
	 * abstract, and should never be destructed directly.
	 */
	~const_buf() {
		raw_reset();
	}

	/** hide copy constructor, but make it available to derived classes */
	const_buf(const_buf const&) = default;

	/** hide copy assignment, but make it available to derived classes */
	const_buf& operator=(const_buf const&) = default;

	/** move constructor (cleans up the original) */
	const_buf(const_buf&& other) : const_buf(other.m_data, other.m_size) {
		other.raw_reset();
	}

	/** move assignment (cleans up the original) */
	const_buf& operator=(const_buf&& other) {
		if (this != &other) {
			operator=(other);
			other.raw_reset();
		}
		return *this;
	}

	/** reset raw data */
	void raw_reset() {
		m_data = nullptr;
		m_size = 0;
	}

	/** set raw data */
	void raw_set(unsigned char const* data, size_t size) {
		m_data = data;
		m_size = size;
	}

	/** set raw data */
	void raw_set(char const* data, size_t size) {
		raw_set(reinterpret_cast<unsigned char const*>(data), size);
	}

private:
	/**
	 * @brief this is the entry point to created possible shared slices
	 */
	virtual shared_const_buf internal_shared_slice(size_t from, size_t size) const;

	unsigned char const* m_data{nullptr};
	std::size_t m_size{0};
};

/**
 * @brief implementation of @ref const_buf which does not keep
 *     referenced data alive - in other words, just "raw" pointers.
 */
class raw_const_buf final : public const_buf {
public:
	/** @brief initialize empty buffer */
	explicit raw_const_buf() = default;

	/**
	 * @brief initialize from raw pointers - make sure you keep the data
	 *     around
	 */
	explicit raw_const_buf(unsigned char const* data, std::size_t size) : const_buf(data, size) {}

	/**
	 * @brief initialize from raw pointers - make sure you keep the data
	 *     around
	 */
	explicit raw_const_buf(char const* data, std::size_t size) : const_buf(data, size) {}

	/** @brief default copy constructor */
	raw_const_buf(raw_const_buf const&) = default;
	/** @brief default copy assignment */
	raw_const_buf& operator=(raw_const_buf const&) = default;
	/** @brief default move constructor */
	raw_const_buf(raw_const_buf&& other) = default;
	/** @brief default move assignment */
	raw_const_buf& operator=(raw_const_buf&& other) = default;
	/** @brief default destructor */
	~raw_const_buf() = default;

	/**
	 * @brief initialize with data contained in some container - make
	 *     sure you keep the container around
	 */
	template <typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* = nullptr>
	explicit raw_const_buf(Container const& data) : const_buf(Storage::data(data), Storage::size(data)) {}

	/**
	 * @brief implicit conversion from `boost::asio::const_buffer`
	 */
	raw_const_buf(boost::asio::const_buffer data);

	/**
	 * @brief implicit conversion from `boost::asio::mutable_buffer`
	 */
	raw_const_buf(boost::asio::mutable_buffer data);

	/**
	 * @brief reset to empty buffer
	 */
	void reset() {
		raw_reset();
	}
};

/**
 * @brief create @ref raw_const_buf from string literal
 */
inline raw_const_buf operator"" _cb(const char* str, std::size_t len) {
	return raw_const_buf(str, len);
}

/* inline method implementations */

inline raw_const_buf const_buf::raw_slice(size_t from, size_t size) const {
	from = std::min(from, m_size);
	size = std::min(m_size - from, size);
	return raw_const_buf(m_data + from, size);
}

inline raw_const_buf const_buf::raw_slice(size_t from) const {
	return raw_slice(from, m_size);
}

inline raw_const_buf const_buf::raw_copy() const {
	return raw_slice(0, m_size);
}

__CANEY_MEMORYV1_END
