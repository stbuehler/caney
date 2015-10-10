/** @file */

#pragma once

#include "caney/std/private.hpp"

#include "internal.hpp"

#include <cstring>
#include <memory>

#include <boost/noncopyable.hpp>
#include <boost/smart_ptr/intrusive_ptr.hpp>
/* counter policies: */
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

__CANEY_MEMORYV1_BEGIN

/* forward declarations */
template<typename AllocatorT = std::allocator<void>, typename CounterPolicyT = boost::thread_safe_counter>
class generic_intrusive_buffer;

template<typename AllocatorT, typename CounterPolicyT = boost::thread_safe_counter>
inline boost::intrusive_ptr<generic_intrusive_buffer<AllocatorT, CounterPolicyT>> alloc_intrusive_buffer(AllocatorT const& alloc, std::size_t size);
template<typename AllocatorT, typename CounterPolicyT>
inline void intrusive_ptr_add_ref(const generic_intrusive_buffer<AllocatorT, CounterPolicyT>* p) BOOST_NOEXCEPT;
template<typename AllocatorT, typename CounterPolicyT>
inline void intrusive_ptr_release(const generic_intrusive_buffer<AllocatorT, CounterPolicyT>* p) BOOST_NOEXCEPT;

/** generic intrusive buffer pointer type */
template<typename AllocatorT = std::allocator<void>, typename CounterPolicyT = boost::thread_safe_counter>
using generic_intrusive_buffer_ptr = boost::intrusive_ptr<generic_intrusive_buffer<AllocatorT, CounterPolicyT>>;

/** simple intrusive buffer type */
using intrusive_buffer = generic_intrusive_buffer<>;
/** simple intrusive buffer pointer type */
using intrusive_buffer_ptr = generic_intrusive_buffer_ptr<>;

/**
 * @brief shared mutable managed buffer with intrusive reference counting
 *
 * an intrusive buffer:
 * - keeps track how it was allocated, how big the buffer is and how many pointers there are (using boost::intrusive_ptr)
 * - the meta data and the buffer are allocated as one
 * - uses AllocatorT::rebind_alloc<char> for memory management
 * @tparam AllocatorT allocator to use
 * @tparam CounterPolicyT counter policy to use for intrusive counter
 */
template<typename AllocatorT, typename CounterPolicyT>
class generic_intrusive_buffer final : private AllocatorT, private boost::noncopyable
{
public:
	/** iterator type */
	typedef unsigned char* iterator;
	/** const iterator type */
	typedef unsigned char const* const_iterator;
	/** typedef for contained values (byte) */
	typedef unsigned char value_type;
	/** size type */
	typedef std::size_t size_type;
	/** iterator differencetype */
	typedef std::ptrdiff_t difference_type;

	/**
	 * @brief only public to be accessible by allocator::construct;
	 * use @ref make_intrusive_buffer() or @ref alloc_intrusive_buffer() instead
	 * @internal
	 */
	explicit generic_intrusive_buffer(AllocatorT const& alloc, std::size_t size, caney::private_tag_t)
	: AllocatorT(alloc), m_size(size) {
	}

	/** @brief size of buffer */
	size_type size() const { return m_size; }
	/** @brief whether buffer is empty */
	bool empty() const { return 0 == size(); }
	/** @brief whether buffer is not empty */
	explicit operator bool() const { return !empty(); }

	/** @brief pointer to buffer memory */
	/* the buffer data region starts directly after the meta object */
	unsigned char* data() const { return const_cast<unsigned char*>(reinterpret_cast<unsigned char const*>(this + 1)); }

	/**
	 * @{
	 * @brief standard iterator
	 */
	iterator begin() const { return data(); }
	iterator end() const { return data() + size(); }
	const_iterator cbegin() const { return data(); }
	const_iterator cend() const { return data() + size(); }
	/** @} */

	/** @brief (range-checked) access to buffer element */
	unsigned char& operator[](size_type ndx) const {
		if (ndx >= size()) std::terminate();
		return data()[ndx];
	}

private:
	typedef typename CounterPolicyT::type counter_t;
	mutable counter_t m_ref_counter{0};
	const std::size_t m_size{0};

	typedef generic_intrusive_buffer<AllocatorT, CounterPolicyT> self_t;

	typedef typename std::allocator_traits<AllocatorT>::template rebind_alloc<self_t> allocator_t;
	typedef std::allocator_traits<allocator_t> allocator_traits_t;
	typedef typename std::allocator_traits<AllocatorT>::template rebind_alloc<char> mem_allocator_t;
	typedef std::allocator_traits<mem_allocator_t> mem_allocator_traits_t;

	friend boost::intrusive_ptr<generic_intrusive_buffer<AllocatorT, CounterPolicyT>> alloc_intrusive_buffer<AllocatorT, CounterPolicyT>(AllocatorT const& alloc, std::size_t size);
	friend void intrusive_ptr_add_ref<AllocatorT, CounterPolicyT>(const generic_intrusive_buffer<AllocatorT, CounterPolicyT>* p) BOOST_NOEXCEPT;
	friend void intrusive_ptr_release<AllocatorT, CounterPolicyT>(const generic_intrusive_buffer<AllocatorT, CounterPolicyT>* p) BOOST_NOEXCEPT;
};

/**
 * @brief allocate an intrusive buffer with given size
 * @param alloc allocator to use
 * @param size size of buffer to allocate
 */
template<typename AllocatorT, typename CounterPolicyT>
inline boost::intrusive_ptr<generic_intrusive_buffer<AllocatorT, CounterPolicyT>> alloc_intrusive_buffer(AllocatorT const& alloc, std::size_t size) {
	typedef generic_intrusive_buffer<AllocatorT, CounterPolicyT> buffer_t;
	typename buffer_t::allocator_t buffer_alloc(alloc);
	typename buffer_t::mem_allocator_t mem_alloc(alloc);

	typename buffer_t::mem_allocator_traits_t::pointer raw_ptr = buffer_t::mem_allocator_traits_t::allocate(mem_alloc, sizeof(buffer_t) + size);
	typename buffer_t::allocator_traits_t::pointer ptr = reinterpret_cast<typename buffer_t::allocator_traits_t::pointer>(raw_ptr);
	try {
		buffer_t::allocator_traits_t::construct(buffer_alloc, ptr, alloc, size, caney::private_tag);
	} catch (...) {
		buffer_t::mem_allocator_traits_t::deallocate(mem_alloc, raw_ptr, sizeof(buffer_t) + size);
		throw;
	}
	return boost::intrusive_ptr<buffer_t>(ptr);
}

/**
 * @brief allocate an intrusive buffer with given size and initialize with data
 * @param alloc allocator to use
 * @param data data of size `size` to copy data from
 * @param size size of buffer to allocate
 */
template<typename AllocatorT, typename CounterPolicyT = boost::thread_safe_counter>
inline boost::intrusive_ptr<generic_intrusive_buffer<AllocatorT, CounterPolicyT>> alloc_intrusive_buffer(AllocatorT const& alloc, void const* data, std::size_t size) {
	auto buf = alloc_intrusive_buffer<AllocatorT, CounterPolicyT>(alloc, size);
	if (size > 0) std::memcpy(buf->data(), data, size);
	return buf;
}

/**
 * @brief allocate an intrusive buffer with given size using std::allocator
 * @param size size of buffer to allocate
 */
template<typename CounterPolicyT = boost::thread_safe_counter>
inline boost::intrusive_ptr<generic_intrusive_buffer<std::allocator<void>, CounterPolicyT>> make_intrusive_buffer(std::size_t size) {
	return alloc_intrusive_buffer<std::allocator<void>, CounterPolicyT>(std::allocator<void>(), size);
}

/**
 * @brief allocate an intrusive buffer with given size and initialize with data using std::allocator
 * @param data data of size `size` to copy data from
 * @param size size of buffer to allocate
 */
template<typename CounterPolicyT = boost::thread_safe_counter>
inline boost::intrusive_ptr<generic_intrusive_buffer<std::allocator<void>, CounterPolicyT>> make_intrusive_buffer(void const* data, std::size_t size) {
	return alloc_intrusive_buffer<std::allocator<void>, CounterPolicyT>(std::allocator<void>(), data, size);
}

/**
 * @brief counter manipulation, needed for boost::intrusive_ptr
 * @internal
 */
template<typename AllocatorT, typename CounterPolicyT>
inline void intrusive_ptr_add_ref(const generic_intrusive_buffer<AllocatorT, CounterPolicyT>* p) BOOST_NOEXCEPT {
	CounterPolicyT::increment(p->m_ref_counter);
}

/**
 * @brief counter manipulation, needed for boost::intrusive_ptr
 * @internal
 */
template<typename AllocatorT, typename CounterPolicyT>
inline void intrusive_ptr_release(const generic_intrusive_buffer<AllocatorT, CounterPolicyT>* p) BOOST_NOEXCEPT {
	try {
		if (CounterPolicyT::decrement(p->m_ref_counter) == 0) {
			typedef generic_intrusive_buffer<AllocatorT, CounterPolicyT> buffer_t;
			std::size_t size = p->m_size;
			typename buffer_t::allocator_t buffer_alloc(static_cast<AllocatorT const&>(*p));
			typename buffer_t::mem_allocator_t mem_alloc(static_cast<AllocatorT const&>(*p));
			buffer_t::allocator_traits_t::destroy(buffer_alloc, p);
			buffer_t::mem_allocator_traits_t::deallocate(mem_alloc, reinterpret_cast<char*>(const_cast<buffer_t*>(p)), sizeof(buffer_t) + size);
		}
	} catch (...) {
		std::terminate();
	}
}

__CANEY_MEMORYV1_END
