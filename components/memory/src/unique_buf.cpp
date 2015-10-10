#include "caney/memory/unique_buf.hpp"

#include "caney/memory/shared_const_buf.hpp"

__CANEY_MEMORYV1_BEGIN

unique_buf::unique_buf(unique_buf const& other)
: mutable_buf(), m_storage(other.m_storage) {
	raw_set(other);
}

unique_buf& unique_buf::operator=(unique_buf const& other) {
	m_storage = other.m_storage;
	raw_set(other);
	return *this;
}

/* static */
unique_buf unique_buf::allocate(std::size_t size) {
	if (0 == size) return unique_buf();
	return unique_buf(make_intrusive_buffer(size));
}

/* static */
unique_buf unique_buf::copy(unsigned char const* data, std::size_t size) {
	if (0 == size) return unique_buf();
	return unique_buf(make_intrusive_buffer(data, size));
}

/* static */
unique_buf unique_buf::copy(char const* data, std::size_t size) {
	return copy(reinterpret_cast<unsigned char const*>(data), size);
}

/* static */
unique_buf unique_buf::copy(const_buf const& buffer) {
	return copy(buffer.data(), buffer.size());
}

shared_const_buf unique_buf::freeze(std::size_t size) {
	shared_const_buf result = shared_const_buf::unsafe_use(m_storage, raw_slice(0, size));
	raw_set(raw_slice(size));
	return result;
}

shared_const_buf unique_buf::freeze() {
	shared_const_buf result = shared_const_buf::unsafe_use(m_storage, raw_copy());
	m_storage.reset();
	raw_reset();
	return result;
}

unique_buf unique_buf::slice(size_t from, size_t length)&& {
	unique_buf result{std::move(*this)};
	result.raw_set(result.raw_slice(from, length));
	return result;
}

unique_buf unique_buf::slice(size_t from)&& {
	return std::move(*this).slice(from, size());
}

unique_buf::unique_buf(intrusive_buffer_ptr buffer)
: mutable_buf(buffer ? buffer->data() : nullptr, buffer ? buffer->size() : 0)
, m_storage(std::move(buffer)) {
}

__CANEY_MEMORYV1_END
