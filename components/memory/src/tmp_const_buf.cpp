#include "caney/memory/tmp_const_buf.hpp"

#include "caney/memory/shared_const_buf.hpp"

__CANEY_MEMORYV1_BEGIN

shared_const_buf tmp_const_buf::internal_shared_slice(size_t from, size_t size) const {
	if (!m_backend) return shared_const_buf();
	size_t inner_from = data() - m_backend->data();
	return m_backend->shared_slice(inner_from + from, size);
}

__CANEY_MEMORYV1_END
