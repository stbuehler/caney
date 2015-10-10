/** @file */

#pragma once

#include "buffer_prototypes.hpp"
#include "buffer_storage.hpp"

#include <memory>

__CANEY_MEMORYV1_BEGIN

class const_buf {
public:
	typedef unsigned char const* iterator;
	typedef unsigned char const* const_iterator;
	typedef unsigned char value_type;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	size_type size() const { return m_size; }
	bool empty() const { return 0 == m_size; }
	explicit operator bool() const { return !empty(); }

	unsigned char const* data() const { return m_data; }

	const_iterator begin() const { return m_data; }
	const_iterator end() const { return m_data + m_size; }
	const_iterator cbegin() const { return m_data; }
	const_iterator cend() const { return m_data + m_size; }

	char const* char_begin() const { return reinterpret_cast<char const*>(m_data); }
	char const* char_end() const { return reinterpret_cast<char const*>(m_data) + m_size; }

	unsigned char operator[](size_type ndx) const {
		if (ndx >= m_size) std::terminate();
		return m_data[ndx];
	}

	/* implicit */
	operator boost::asio::const_buffer() const;

	shared_const_buf shared_slice(size_type from, size_type size) const;
	shared_const_buf shared_slice(size_type from) const;
	shared_const_buf shared_copy() const;

	raw_const_buf raw_slice(size_type from, size_type size) const;
	raw_const_buf raw_slice(size_type from) const;
	raw_const_buf raw_copy() const;

	unique_buf copy() const;

protected:
	const_buf() = default;
	explicit const_buf(unsigned char const* data, size_t size)
	: m_data(data), m_size(size) { }
	explicit const_buf(char const* data, size_t size)
	: const_buf(reinterpret_cast<unsigned char const*>(data), size) { }
	~const_buf() = default; // no need for virtual

	const_buf(const_buf const& other) = delete;
	const_buf& operator=(const_buf const& other) = delete;
	const_buf(const_buf&& other)
	: const_buf(other.m_data, other.m_size) {
		other.raw_reset();
	}
	const_buf& operator=(const_buf&& other) {
		if (this != &other) {
			raw_set(other);
			other.raw_reset();
		}
		return *this;
	}

	void raw_reset() {
		m_data = nullptr;
		m_size = 0;
	}

	void raw_set(unsigned char const* data, size_t size) {
		m_data = data;
		m_size = size;
	}
	void raw_set(char const* data, size_t size) {
		raw_set(reinterpret_cast<unsigned char const*>(data), size);
	}
	void raw_set(const_buf const& other) {
		raw_set(other.m_data, other.m_size);
	}

private:
	virtual shared_const_buf internal_shared_slice(size_t from, size_t size) const;

	unsigned char const* m_data{nullptr};
	std::size_t m_size{0};
};

class raw_const_buf final: public const_buf {
public:
	explicit raw_const_buf() = default;
	explicit raw_const_buf(unsigned char const* data, std::size_t size)
	: const_buf(data, size) { }
	explicit raw_const_buf(char const* data, std::size_t size)
	: const_buf(data, size) { }

	raw_const_buf(raw_const_buf const& other)
	: const_buf() {
		raw_set(other);
	}
	raw_const_buf& operator=(raw_const_buf const& other) {
		raw_set(other);
		return *this;
	}
	raw_const_buf(raw_const_buf&& other) = default;
	raw_const_buf& operator=(raw_const_buf&& other) = default;
	~raw_const_buf() = default;

	/* implicit */
	template<typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* =nullptr>
	explicit raw_const_buf(Container const& data)
	: const_buf(Storage::data(data), Storage::size(data)) {
	}

	/* implicit */
	explicit raw_const_buf(boost::asio::const_buffer data);

	/* implicit */
	explicit raw_const_buf(boost::asio::mutable_buffer data);

	void reset() {
		raw_reset();
	}
};

inline raw_const_buf operator"" _cb(const char *str, std::size_t len) {
	return raw_const_buf(str, len);
}

/* inline method implementations */

inline raw_const_buf const_buf::raw_slice(size_type from, size_type size) const {
	from = std::min(from, m_size);
	size = std::min(m_size - from, size);
	return raw_const_buf(m_data + from, size);
}

inline raw_const_buf const_buf::raw_slice(size_type from) const {
	return raw_slice(from, m_size);
}

inline raw_const_buf const_buf::raw_copy() const {
	return raw_slice(0, m_size);
}

__CANEY_MEMORYV1_END
