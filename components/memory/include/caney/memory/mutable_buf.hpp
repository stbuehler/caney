/** @file */

#pragma once

#include "const_buf.hpp"

__CANEY_MEMORYV1_BEGIN

class mutable_buf {
public:
	typedef unsigned char* iterator;
	typedef unsigned char const* const_iterator;
	typedef unsigned char value_type;
	typedef std::size_t size_type;
	typedef std::ptrdiff_t difference_type;

	size_type size() const { return m_buffer.size(); }
	bool empty() const { return 0 == size(); }
	explicit operator bool() const { return !empty(); }

	unsigned char* data() const { return const_cast<unsigned char*>(m_buffer.data()); }

	iterator begin() const { return data(); }
	iterator end() const { return data() + size(); }
	const_iterator cbegin() const { return data(); }
	const_iterator cend() const { return data() + size(); }

	char* char_begin() const { return reinterpret_cast<char*>(data()); }
	char* char_end() const { return reinterpret_cast<char*>(data()) + size(); }

	unsigned char& operator[](size_type ndx) const {
		if (ndx >= size()) std::terminate();
		return data()[ndx];
	}

	/* implicit */
	operator boost::asio::const_buffer() const;
	/* implicit */
	operator boost::asio::mutable_buffer() const;
	/* implicit */
	operator const_buf const&() const {
		return m_buffer;
	}

	raw_mutable_buf raw_slice(size_type from, size_type size) const;
	raw_mutable_buf raw_slice(size_type from) const;
	raw_mutable_buf raw_copy() const;

	unique_buf copy() const;

protected:
	mutable_buf() = default;
	explicit mutable_buf(unsigned char* data, size_t size)
	: m_buffer(data, size) { }
	explicit mutable_buf(char* data, size_t size)
	: mutable_buf(reinterpret_cast<unsigned char*>(data), size) { }
	~mutable_buf() = default; // no need for virtual

	mutable_buf(mutable_buf const&) = delete;
	mutable_buf& operator=(mutable_buf const&) = delete;
	mutable_buf(mutable_buf&&) = default;
	mutable_buf& operator=(mutable_buf&&) = default;

	void raw_reset() {
		m_buffer.reset();
	}

	void raw_set(unsigned char* data, size_t size) {
		m_buffer = raw_const_buf(data, size);
	}
	void raw_set(char* data, size_t size) {
		raw_set(reinterpret_cast<unsigned char*>(data), size);
	}
	void raw_set(mutable_buf const& other) {
		raw_set(other.data(), other.size());
	}

private:
	raw_const_buf m_buffer;
};

class raw_mutable_buf final: public mutable_buf {
public:
	explicit raw_mutable_buf() = default;
	raw_mutable_buf(raw_mutable_buf const& other)
	: mutable_buf() {
		raw_set(other);
	}
	raw_mutable_buf& operator=(raw_mutable_buf const& other) {
		raw_set(other);
		return *this;
	}
	raw_mutable_buf(raw_mutable_buf&& other) = default;
	raw_mutable_buf& operator=(raw_mutable_buf&& other) = default;

	explicit raw_mutable_buf(unsigned char* data, std::size_t size)
	: mutable_buf(data, size) { }

	explicit raw_mutable_buf(char* data, std::size_t size)
	: mutable_buf(data, size) { }

	template<typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* =nullptr>
	explicit raw_mutable_buf(Container& data)
	: raw_mutable_buf(Storage::data(data), Storage::size(data)) {
	}

	explicit raw_mutable_buf(boost::asio::mutable_buffer data);

	void reset() {
		raw_reset();
	}
};

/* inline method implementations */

inline raw_mutable_buf mutable_buf::raw_slice(size_type from, size_type length) const {
	from = std::min(from, size());
	length = std::min(size() - from, length);
	return raw_mutable_buf(data() + from, length);
}

inline raw_mutable_buf mutable_buf::raw_slice(size_type from) const {
	return raw_slice(from, size());
}

inline raw_mutable_buf mutable_buf::raw_copy() const {
	return raw_slice(0, size());
}

__CANEY_MEMORYV1_END
