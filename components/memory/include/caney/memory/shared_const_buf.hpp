/** @file */

#pragma once

#include "const_buf.hpp"
#include "intrusive_buffer.hpp"

#include <memory>

#include <boost/variant/variant.hpp>

__CANEY_MEMORYV1_BEGIN

/**
 * @brief implementation of @ref const_buf which keeps memory alive.
 *
 * This should be used to store buffer long term (i.e. on the heap).
 */
class shared_const_buf final: public const_buf {
public:
	/** @brief various underlying types that can keep a buffer alive */
	typedef boost::variant<std::shared_ptr<void>, intrusive_buffer_ptr> storage_t;

	/** @brief default construct empty buffer */
	explicit shared_const_buf() = default;

	/** @brief copy constructor */
	shared_const_buf(shared_const_buf const&) = default;

	/** @brief copy assignment */
	shared_const_buf& operator=(shared_const_buf const&) = default;

	/** @brief move constructor */
	shared_const_buf(shared_const_buf&&) = default;

	/** @brief move assignment */
	shared_const_buf& operator=(shared_const_buf&&) = default;

	/** @brief default destructor */
	~shared_const_buf() = default;

	/** @brief move data from container into buffer */
	template<typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* =nullptr>
	explicit shared_const_buf(Container&& data) {
		std::shared_ptr<Container> storage = std::make_shared<Container>(std::move(data));
		raw_set(Storage::data(*storage), Storage::size(*storage));
		m_storage = std::move(storage);
	}

	/** @brief create new buffer from given data (copies the data) */
	static shared_const_buf copy(unsigned char const* data, std::size_t size);

	/** @brief create new buffer from given data (copies the data) */
	static shared_const_buf copy(char const* data, std::size_t size);

	/** @brief create new buffer from another buffer (copies the data) */
	static shared_const_buf copy(const_buf const& buffer);

	/** @brief create new buffer from data in a container (copies the data) */
	template<typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* =nullptr>
	static shared_const_buf copy(Container const& data) {
		return copy(Storage::data(data), Storage::size(data));
	}

	/**
	 * @brief create new buffer from raw pointers in `buffer` claiming
	 *     `storage` keeps it alive. don't modify `buffer` afterwards,
	 *     it is supposed to be constant.
	 */
	static shared_const_buf unsafe_use(storage_t storage, const_buf const& buffer);

	/**
	 * @brief create new buffer from @ref intrusive_buffer. don't modify
	 *     `buffer` afterwards, it is supposed to be constant.
	 */
	static shared_const_buf unsafe_use(intrusive_buffer_ptr buffer);

private:
	shared_const_buf internal_shared_slice(size_t from, size_t size) const override;

	explicit shared_const_buf(storage_t storage, const_buf const& buffer);

	storage_t m_storage; // pointer value is ignored
};

__CANEY_MEMORYV1_END
