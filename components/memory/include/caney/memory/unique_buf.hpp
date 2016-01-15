/** @file */

#pragma once

#include "intrusive_buffer.hpp"
#include "mutable_buf.hpp"

#include <memory>

__CANEY_MEMORYV1_BEGIN

/**
 * @brief implementation of @ref mutable_buf which keeps memory alive.
 */
class unique_buf final : public mutable_buf {
public:
	/** @brief construct empty buffer */
	explicit unique_buf() = default;

	/**
	 * @brief not copyable
	 * @internal
	 */
	unique_buf(unique_buf const&) = delete;
	/**
	 * @brief not copyable
	 * @internal
	 */
	unique_buf& operator=(unique_buf const&) = delete;

	/** move constructor */
	unique_buf(unique_buf&& other);

	/** move assignment */
	unique_buf& operator=(unique_buf&& other);

	/**
	 * move container into buffer
	 */
	template <typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* = nullptr>
	unique_buf(Container&& data) {
		std::shared_ptr<Container> storage = std::make_shared<Container>(std::move(data));
		m_storage = storage;
		raw_set(Storage::data(*storage), Storage::size(*storage));
	}

	/**
	 * @brief allocate new buffer
	 */
	static unique_buf allocate(std::size_t size);

	/**
	 * @brief create new buffer and copy given data to it
	 */
	static unique_buf copy(unsigned char const* data, std::size_t size);

	/**
	 * @brief create new buffer and copy given data to it
	 */
	static unique_buf copy(char const* data, std::size_t size);

	/**
	 * @brief create new buffer and copy given buffer into it
	 */
	static unique_buf copy(const_buf const& buffer);

	/**
	 * @brief splits of `size` bytes at the beginning; remove the
	 *     beginning from the buffer and return it as "frozen"
	 */
	shared_const_buf freeze(std::size_t size);

	/**
	 * @brief freeze complete buffer and return it; the unique buffer
	 *     will be empty afterwards.
	 */
	shared_const_buf freeze();

	/**
	 * @brief create slice from a temporary buffer
	 * @param from index to copy data from (gets ranged clipped)
	 * @param size how many bytes to copy (gets ranged clipped)
	 */
	unique_buf slice(size_t from, size_t size) &&;

	/**
	 * @brief create slice from a temporary buffer, alias to
	 *     `slice(from, size())`
	 */
	unique_buf slice(size_t from) &&;

private:
	explicit unique_buf(intrusive_buffer_ptr buffer);

	intrusive_buffer_ptr m_storage;
};

__CANEY_MEMORYV1_END
