/** @file */

#pragma once

#include "const_buf.hpp"

__CANEY_MEMORYV1_BEGIN

/**
 * @brief represents an modifieable contiguous byte buffer
 *
 * this is a "abstract" base class. implementations are:
 * - @ref raw_mutable_buf
 * - @ref unique_buf
 */
class mutable_buf {
public:
	/**
	 * @{
	 * @brief standard container typedef
	 */
	typedef unsigned char* iterator;
	typedef unsigned char const* const_iterator;
	typedef unsigned char value_type;
	typedef size_t size_type;
	typedef std::ptrdiff_t difference_type;
	/** @} */

	/**
	 * @brief size of buffer (length in bytes)
	 */

	size_t size() const {
		return m_buffer.size();
	}

	/**
	 * @brief whether buffer is empty (i.e. zero length)
	 */
	bool empty() const {
		return 0 == size();
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
	unsigned char* data() const {
		return const_cast<unsigned char*>(m_buffer.data());
	}

	/**
	 @{
	 * @brief standard iterator getter
	 */
	iterator begin() const {
		return data();
	}
	iterator end() const {
		return data() + size();
	}
	const_iterator cbegin() const {
		return data();
	}
	const_iterator cend() const {
		return data() + size();
	}
	/** @} */

	/**
	 * @brief similar to begin() but returns iterator for `char` access
	 *     instead of byte (unsigned char)
	 */
	char* char_begin() const {
		return reinterpret_cast<char*>(data());
	}

	/**
	 * @brief similar to end() but returns iterator for `char` access
	 *     instead of byte (unsigned char)
	 */
	char* char_end() const {
		return reinterpret_cast<char*>(data()) + size();
	}

	/**
	 * @brief return byte value at position `ndx` (terminates if range
	 *     check fails)
	 * @param ndx index to read byte from
	 */
	unsigned char& operator[](size_t ndx) const {
		if (ndx >= size()) std::terminate();
		return data()[ndx];
	}

	/**
	 * @brief implicit cast to `boost::asio::const_buffer` which just
	 *     contains a raw pointer to the data and the size
	 */
	operator boost::asio::const_buffer() const;

	/**
	 * @brief implicit cast to `boost::asio::mutable_buffer` which just
	 *     contains a raw pointer to the data and the size
	 */
	operator boost::asio::mutable_buffer() const;

	/**
	 * @brief have `mutable_buf` act as `const_buf`
	 */
	operator const_buf const&() const {
		return m_buffer;
	}

	/**
	 * @brief create a raw reference to a slice of the data. the raw
	 *     reference is only valid as long as the original buffer
	 *     keeping the data alive is.
	 *
	 * @param from index to copy data from (gets ranged clipped)
	 * @param size how many bytes to copy (gets ranged clipped)
	 */
	raw_mutable_buf raw_slice(size_t from, size_t size) const;

	/** @brief alias for `raw_slice(from, size())` */
	raw_mutable_buf raw_slice(size_t from) const;

	/** @brief alias for `raw_slice(0, size())` */
	raw_mutable_buf raw_copy() const;

	/**
	 * @brief create a unique copy of the data. if you only need the copy
	 * of a slice just call @ref copy() on a @ref raw_slice().
	 */
	unique_buf copy() const;

protected:
	/** default construct empty buffer */
	mutable_buf() = default;

	/** initialize with raw data */
	explicit mutable_buf(unsigned char* data, size_t size) : m_buffer(data, size) {}

	/** initialize with raw data */
	explicit mutable_buf(char* data, size_t size) : mutable_buf(reinterpret_cast<unsigned char*>(data), size) {}

	/**
	 * destructor is not virtual as this class is supposed to be
	 * abstract, and should never be destructed directly.
	 */
	~mutable_buf() {
		raw_reset();
	}

	/** hide copy constructor, but make it available to derived classes */
	mutable_buf(mutable_buf const&) = default;

	/** hide copy assignment, but make it available to derived classes */
	mutable_buf& operator=(mutable_buf const&) = default;

	/** hide move constructor, but make it available to derived classes */
	mutable_buf(mutable_buf&&) = default;

	/** hide move assignment, but make it available to derived classes */
	mutable_buf& operator=(mutable_buf&&) = default;

	/** reset raw data */
	void raw_reset() {
		m_buffer.reset();
	}

	/** set raw data */
	void raw_set(unsigned char* data, size_t size) {
		m_buffer = raw_const_buf(data, size);
	}

	/** set raw data */
	void raw_set(char* data, size_t size) {
		raw_set(reinterpret_cast<unsigned char*>(data), size);
	}

private:
	raw_const_buf m_buffer;
};

/**
 * @brief implementation of @ref mutable_buf which does not keep
 *     referenced data alive - in other words, just "raw" pointers.
 */
class raw_mutable_buf final : public mutable_buf {
public:
	/** @brief initialize empty buffer */
	explicit raw_mutable_buf() = default;

	/**
	 * @brief initialize from raw pointers - make sure you keep the data
	 *     around
	 */
	explicit raw_mutable_buf(unsigned char* data, std::size_t size) : mutable_buf(data, size) {}

	/**
	 * @brief initialize from raw pointers - make sure you keep the data
	 *     around
	 */
	explicit raw_mutable_buf(char* data, std::size_t size) : mutable_buf(data, size) {}

	/** @brief default copy constructor */
	raw_mutable_buf(raw_mutable_buf const&) = default;
	/** @brief default copy assignment */
	raw_mutable_buf& operator=(raw_mutable_buf const&) = default;
	/** @brief default move constructor */
	raw_mutable_buf(raw_mutable_buf&& other) = default;
	/** @brief default move assignment */
	raw_mutable_buf& operator=(raw_mutable_buf&& other) = default;
	/** @brief default destructor */
	~raw_mutable_buf() = default;

	/**
	 * @brief initialize with data contained in some container - make
	 *     sure you keep the container around
	 */
	template <typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* = nullptr>
	explicit raw_mutable_buf(Container& data) : raw_mutable_buf(Storage::data(data), Storage::size(data)) {}

	/**
	 * @brief implicit conversion from `boost::asio::mutable_buffer`
	 */
	raw_mutable_buf(boost::asio::mutable_buffer data);

	/**
	 * @brief reset to empty buffer
	 */
	void reset() {
		raw_reset();
	}
};

/* inline method implementations */

inline raw_mutable_buf mutable_buf::raw_slice(size_t from, size_t length) const {
	from = std::min(from, size());
	length = std::min(size() - from, length);
	return raw_mutable_buf(data() + from, length);
}

inline raw_mutable_buf mutable_buf::raw_slice(size_t from) const {
	return raw_slice(from, size());
}

inline raw_mutable_buf mutable_buf::raw_copy() const {
	return raw_slice(0, size());
}

__CANEY_MEMORYV1_END
