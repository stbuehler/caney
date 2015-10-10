/** @file */

#pragma once

#include "mutable_buf.hpp"
#include "intrusive_buffer.hpp"

#include <memory>

__CANEY_MEMORYV1_BEGIN

class unique_buf final: public mutable_buf {
public:
	explicit unique_buf() = default;
	unique_buf(unique_buf const& other);
	unique_buf& operator=(unique_buf const& other);
	unique_buf(unique_buf&& other) = default;
	unique_buf& operator=(unique_buf&& other) = default;

	/* implicit */
	template<typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* =nullptr>
	unique_buf(Container&& data) {
		std::shared_ptr<Container> storage = std::make_shared<Container>(std::move(data));
		m_storage = storage;
		raw_set(Storage::data(*storage), Storage::size(*storage));
	}

	static unique_buf allocate(std::size_t size);

	static unique_buf copy(unsigned char const* data, std::size_t size);
	static unique_buf copy(char const* data, std::size_t size);
	static unique_buf copy(const_buf const& buffer);

	// splits of <size> bytes at the beginning from the buffer and freezes them
	shared_const_buf freeze(std::size_t size);
	// freeze complete buffer
	shared_const_buf freeze();

	unique_buf slice(size_t from, size_t length)&&;

	unique_buf slice(size_t from)&&;

private:
	explicit unique_buf(intrusive_buffer_ptr buffer);

	intrusive_buffer_ptr m_storage;
};

__CANEY_MEMORYV1_END
