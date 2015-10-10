#include "caney/memory/shared_const_buf.hpp"

__CANEY_MEMORYV1_BEGIN

shared_const_buf::shared_const_buf(shared_const_buf const& other)
: const_buf(), m_storage(other.m_storage) {
	raw_set(other);
}

shared_const_buf& shared_const_buf::operator=(shared_const_buf const& other) {
	m_storage = other.m_storage;
	raw_set(other);
	return *this;
}

/* static */
shared_const_buf shared_const_buf::copy(unsigned char const* data, std::size_t size) {
	if (0 == size) return shared_const_buf();
	return unsafe_use(intrusive_buffer::create(data, size));
}

/* static */
shared_const_buf shared_const_buf::copy(char const* data, std::size_t size) {
	return copy(reinterpret_cast<unsigned char const*>(data), size);
}

/* static */
shared_const_buf shared_const_buf::copy(const_buf const& buffer) {
	return copy(buffer.data(), buffer.size());
}

/* static */
shared_const_buf shared_const_buf::unsafe_use(storage_t storage, const_buf const& buffer) {
	return shared_const_buf(std::move(storage), buffer);
}

/* static */
shared_const_buf shared_const_buf::unsafe_use(intrusive_buffer_ptr buffer) {
	if (!buffer) return shared_const_buf();
	// extract raw range before move
	raw_const_buf raw(buffer->data(), buffer->size());
	return shared_const_buf(std::move(buffer), raw);
}

shared_const_buf::storage_t shared_const_buf::storage() const {
	return m_storage;
}

shared_const_buf shared_const_buf::internal_shared_slice(size_t from, size_t size) const {
	return unsafe_use(m_storage, raw_slice(from, size));
}

shared_const_buf::shared_const_buf(storage_t storage, const_buf const& buffer)
: m_storage(std::move(storage)) {
	raw_set(buffer);
}

__CANEY_MEMORYV1_END
