/** @file */

#pragma once

#include "const_buf.hpp"
#include "intrusive_buffer.hpp"

#include <memory>

#include <boost/variant/variant.hpp>

__CANEY_MEMORYV1_BEGIN

class shared_const_buf final: public const_buf {
public:
	typedef boost::variant<std::shared_ptr<void>, intrusive_buffer_ptr> storage_t;

	explicit shared_const_buf() = default;
	shared_const_buf(shared_const_buf const& other);
	shared_const_buf& operator=(shared_const_buf const& other);
	shared_const_buf(shared_const_buf&&) = default;
	shared_const_buf& operator=(shared_const_buf&&) = default;
	~shared_const_buf() = default;

	template<typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* =nullptr>
	explicit shared_const_buf(Container&& data) {
		std::shared_ptr<Container> storage = std::make_shared<Container>(std::move(data));
		raw_set(Storage::data(*storage), Storage::size(*storage));
		m_storage = std::move(storage);
	}

	static shared_const_buf copy(unsigned char const* data, std::size_t size);
	static shared_const_buf copy(char const* data, std::size_t size);
	static shared_const_buf copy(const_buf const& buffer);

	template<typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* =nullptr>
	static shared_const_buf copy(Container const& data) {
		return copy(Storage::data(data), Storage::size(data));
	}

	static shared_const_buf unsafe_use(storage_t storage, const_buf const& buffer);

	static shared_const_buf unsafe_use(intrusive_buffer_ptr buffer);

	storage_t storage() const;

private:
	shared_const_buf internal_shared_slice(size_t from, size_t size) const override;

	explicit shared_const_buf(storage_t storage, const_buf const& buffer);

	storage_t m_storage; // pointer value is ignored
};

__CANEY_MEMORYV1_END
