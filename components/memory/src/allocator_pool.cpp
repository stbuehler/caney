#include "caney/memory/allocator_pool.hpp"

__CANEY_MEMORYV1_BEGIN

namespace {
	void mem_free(char* obj, std::size_t n) {
		std::allocator<char>().deallocate(obj, n);
	}

	char* mem_alloc(std::size_t n) {
		return std::allocator<char>().allocate(n);
	}
}

allocator_pool::pool::~pool() {
	auto front = m_front.synchronize();
	while (nullptr != *front) {
		chunk_link* elem = *front;
		*front = elem->next;
		mem_free(reinterpret_cast<char*>(elem), m_size);
	}
}

// static
char* allocator_pool::pool::allocate(pool* p, std::size_t n) {
	n = std::max(n, sizeof(chunk_link));

	if (nullptr != p && p->m_size == n) {
		auto front = p->m_front.synchronize();
		if (nullptr != *front) {
			chunk_link* elem = *front;
			*front = elem->next;
			return reinterpret_cast<char*>(elem);
		}
	}
	return mem_alloc(n);
}

// static
void allocator_pool::pool::deallocate(pool* p, char* obj, std::size_t n) {
	n = std::max(n, sizeof(chunk_link));

	if (nullptr != p && p->m_size == n) {
		chunk_link* elem = reinterpret_cast<chunk_link*>(obj);
		auto front = p->m_front.synchronize();
		elem->next = *front;
		*front = elem;
	} else {
		mem_free(obj, n);
	}
}

allocator_pool::allocator_pool(std::size_t size) {
	m_pool = std::make_shared<pool>(size);
}

std::size_t allocator_pool::size() const {
	return m_pool->size();
}

allocator_pool::allocator<void> allocator_pool::alloc() const {
	return allocator<void>(allocator_base(m_pool));
}

__CANEY_MEMORYV1_END
