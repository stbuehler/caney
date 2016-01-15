/** @file */

#pragma once

#include "caney/std/tags.hpp"

#include "internal.hpp"
#include "intrusive_base.hpp"

#include <cstring>
#include <memory>

#include <boost/smart_ptr/intrusive_ptr.hpp>
/* counter policies: */
#include <boost/smart_ptr/intrusive_ref_counter.hpp>

__CANEY_MEMORYV1_BEGIN

namespace impl {
	template <typename Allocator, typename T>
	class allocator_extent : public Allocator {
	private:
		using allocator_t = typename std::allocator_traits<Allocator>::template rebind_alloc<T>;
		using inner_traits = typename std::allocator_traits<allocator_t>;

	public:
		template <typename U>
		struct rebind {
			using other = allocator_extent<typename inner_traits::template rebind_alloc<U>, U>;
		};
		using pointer = typename inner_traits::pointer;
		using size_type = typename inner_traits::size_type;
		using const_void_pointer = typename inner_traits::const_void_pointer;

		allocator_extent(allocator_extent&) = default;
		allocator_extent(allocator_extent const&) = default;
		allocator_extent& operator=(allocator_extent const&) = delete;
		allocator_extent(allocator_extent&&) = default;
		allocator_extent& operator=(allocator_extent&&) = delete;

		explicit allocator_extent(Allocator const& alloc, size_type extent) : Allocator(alloc), m_extent(extent) {}

		template <typename _Alloc, typename _T>
		explicit allocator_extent(allocator_extent<_Alloc, _T> const& alloc) : Allocator(alloc), m_extent(alloc.extent()) {}

		size_type extent() const {
			return m_extent;
		}

		using is_always_equal = std::false_type;

		pointer allocate(size_type n, const_void_pointer hint = nullptr) {
			if (1 == n) {
				typename inner_traits::template rebind_alloc<char> charAlloc(*this);
				auto ptr = inner_traits::template rebind_traits<char>::allocate(charAlloc, sizeof(T) + extent(), hint);
				return reinterpret_cast<pointer>(ptr);
			} else {
				// only single allocations allowed
				std::terminate();
			}
		}

		void deallocate(pointer p, size_type n) {
			if (1 == n) {
				typename inner_traits::template rebind_alloc<char> charAlloc(*this);
				inner_traits::template rebind_traits<char>::deallocate(charAlloc, reinterpret_cast<char*>(p), sizeof(T) + extent());
			} else {
				// only single allocations allowed
				std::terminate();
			}
		}

		friend bool operator==(allocator_extent const& a, allocator_extent const& b) {
			return static_cast<Allocator const&>(a) == static_cast<Allocator const&>(b) && a.extent() == b.extent();
		}
		friend bool operator!=(allocator_extent const& a, allocator_extent const& b) {
			return !(a == b);
		}

	private:
		const size_type m_extent{0};
	};
}


/* forward declarations */
template <typename CounterPolicyT = boost::thread_safe_counter, typename AllocatorT = std::allocator<void>>
class generic_intrusive_buffer;

/** generic intrusive buffer pointer type */
template <typename CounterPolicyT = boost::thread_safe_counter, typename AllocatorT = std::allocator<void>>
using generic_intrusive_buffer_ptr = boost::intrusive_ptr<generic_intrusive_buffer<CounterPolicyT, AllocatorT>>;

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
template <typename CounterPolicyT, typename AllocatorT>
class generic_intrusive_buffer final
	: public intrusive_base<generic_intrusive_buffer<CounterPolicyT, AllocatorT>, CounterPolicyT, impl::allocator_extent<AllocatorT, void>> {
private:
	using self_t = generic_intrusive_buffer<CounterPolicyT, AllocatorT>;
	using base_t = intrusive_base<generic_intrusive_buffer<CounterPolicyT, AllocatorT>, CounterPolicyT, impl::allocator_extent<AllocatorT, void>>;

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
	 * @brief only public to be accessible by `allocate_intrusive`,
	 * use @ref allocate() or @ref create() instead
	 * @internal
	 */
	explicit generic_intrusive_buffer(caney::private_tag_t) {}

	/** @brief size of buffer */
	size_type size() const {
		return this->allocator().extent();
	}
	/** @brief whether buffer is empty */
	bool empty() const {
		return 0 == size();
	}
	/** @brief whether buffer is not empty */
	explicit operator bool() const {
		return !empty();
	}

	/** @brief pointer to buffer memory */
	/* the buffer data region starts directly after the meta object */
	unsigned char* data() const {
		return const_cast<unsigned char*>(reinterpret_cast<unsigned char const*>(this + 1));
	}

	/**
	 * @{
	 * @brief standard iterator
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

	/** @brief (range-checked) access to buffer element */
	unsigned char& operator[](size_type ndx) const {
		if (ndx >= size()) std::terminate();
		return data()[ndx];
	}

	/** type for pointer to buffer */
	using pointer = generic_intrusive_buffer_ptr<CounterPolicyT, AllocatorT>;

	/**
	 * @brief allocate an intrusive buffer with given size
	 * @param alloc allocator to use
	 * @param size size of buffer to allocate
	 */
	static pointer allocate(AllocatorT const& alloc, std::size_t size) {
		impl::allocator_extent<AllocatorT, void> extent_alloc(alloc, size);
		return base_t::allocate(extent_alloc, caney::private_tag);
	}

	/**
	 * @brief allocate an intrusive buffer with given size and initialize with data
	 * @param alloc allocator to use
	 * @param data data of size `size` to copy data from
	 * @param size size of buffer to allocate
	 */
	static pointer allocate(AllocatorT const& alloc, void const* data, std::size_t size) {
		auto buf = allocate(alloc, size);
		if (size > 0) std::memcpy(buf->data(), data, size);
		return buf;
	}

	/**
	 * @brief create an intrusive buffer with given size using default constructed allocator
	 * @param size size of buffer to allocate
	 */
	static pointer create(std::size_t size) {
		return allocate(AllocatorT(), size);
	}

	/**
	 * @brief create an intrusive buffer with given size and initialize with data using default constructed allocator
	 * @param data data of size `size` to copy data from
	 * @param size size of buffer to allocate
	 */
	static pointer create(void const* data, std::size_t size) {
		return allocate(AllocatorT(), data, size);
	}
};

__CANEY_MEMORYV1_END
