#pragma once

#include <memory>
#include <string>
#include <type_traits>
#include <vector>

// #include <boost/variant/variant.hpp>

namespace boost {
	namespace asio {
		class const_buffer;
		class mutable_buffer;
	}
}

namespace caney {
	namespace memory {
		inline namespace v1 {
			namespace impl {
				template<typename Storage>
				struct buffer_storage;

				template<typename Char, typename Traits, typename Allocator>
				struct string_buffer_storage {
					typedef std::basic_string<Char, Traits, Allocator> container_t;

					static unsigned char const* data(container_t const& c) {
						return reinterpret_cast<unsigned char const*>(c.data());
					}

					static unsigned char* data(container_t& c) {
						return reinterpret_cast<unsigned char*>(&c[0]);
					}

					static std::size_t size(container_t const& c) {
						return c.size();
					}
				};

				template<typename Traits, typename Allocator>
				struct buffer_storage<std::basic_string<char, Traits, Allocator>> : string_buffer_storage<char, Traits, Allocator> {
				};

				template<typename Traits, typename Allocator>
				struct buffer_storage<std::basic_string<unsigned char, Traits, Allocator>> : string_buffer_storage<unsigned char, Traits, Allocator> {
				};

				template<typename Char, typename Allocator>
				struct vector_buffer_storage {
					typedef std::vector<Char, Allocator> container_t;

					static unsigned char const* data(container_t const& c) {
						return reinterpret_cast<unsigned char const*>(c.data());
					}

					static unsigned char* data(container_t& c) {
						return reinterpret_cast<unsigned char*>(c.data());
					}

					static std::size_t size(container_t const& c) {
						return c.size();
					}
				};

				template<typename Allocator>
				struct buffer_storage<std::vector<char, Allocator>> : vector_buffer_storage<char, Allocator> {
				};

				template<typename Allocator>
				struct buffer_storage<std::vector<unsigned char, Allocator>> : vector_buffer_storage<unsigned char, Allocator> {
				};
			} // namespace impl

			class const_buf;
			class shared_const_buf;
			class raw_const_buf;
			class tmp_const_buf;
			class mutable_buf;
			class raw_mutable_buf;
			class unique_buf;

			class const_buf {
			public:
				typedef unsigned char const* iterator;
				typedef unsigned char const* const_iterator;
				typedef unsigned char value_type;
				typedef std::size_t size_type;
				typedef std::ptrdiff_t difference_type;

				size_type size() const { return m_size; }
				bool empty() const { return 0 == m_size; }
				explicit operator bool() const { return !empty(); }

				unsigned char const* data() const { return m_data; }

				const_iterator begin() const { return m_data; }
				const_iterator end() const { return m_data + m_size; }
				const_iterator cbegin() const { return m_data; }
				const_iterator cend() const { return m_data + m_size; }

				char const* char_begin() const { return reinterpret_cast<char const*>(m_data); }
				char const* char_end() const { return reinterpret_cast<char const*>(m_data) + m_size; }

				unsigned char operator[](size_type ndx) const {
					if (ndx >= m_size) std::terminate();
					return m_data[ndx];
				}

				/* implicit */
				operator boost::asio::const_buffer() const;

				shared_const_buf shared_slice(size_type from, size_type size) const;
				shared_const_buf shared_slice(size_type from) const;
				shared_const_buf shared_copy() const;

				raw_const_buf raw_slice(size_type from, size_type size) const;
				raw_const_buf raw_slice(size_type from) const;
				raw_const_buf raw_copy() const;

				unique_buf copy() const;

			protected:
				const_buf() = default;
				explicit const_buf(unsigned char const* data, size_t size)
				: m_data(data), m_size(size) { }
				explicit const_buf(char const* data, size_t size)
				: const_buf(reinterpret_cast<unsigned char const*>(data), size) { }
				~const_buf() = default; // no need for virtual

				const_buf(const_buf const& other) = delete;
				const_buf& operator=(const_buf const& other) = delete;
				const_buf(const_buf&& other)
				: const_buf(other.m_data, other.m_size) {
					other.raw_reset();
				}
				const_buf& operator=(const_buf&& other) {
					if (this != &other) {
						raw_set(other);
						other.raw_reset();
					}
					return *this;
				}

				void raw_reset() {
					m_data = nullptr;
					m_size = 0;
				}

				void raw_set(unsigned char const* data, size_t size) {
					m_data = data;
					m_size = size;
				}
				void raw_set(char const* data, size_t size) {
					raw_set(reinterpret_cast<unsigned char const*>(data), size);
				}
				void raw_set(const_buf const& other) {
					raw_set(other.m_data, other.m_size);
				}

			private:
				virtual shared_const_buf internal_shared_slice(size_t from, size_t size) const;

				unsigned char const* m_data{nullptr};
				std::size_t m_size{0};
			};

			class raw_const_buf final: public const_buf {
			public:
				explicit raw_const_buf() = default;
				explicit raw_const_buf(unsigned char const* data, std::size_t size)
				: const_buf(data, size) { }
				explicit raw_const_buf(char const* data, std::size_t size)
				: const_buf(data, size) { }

				raw_const_buf(raw_const_buf const& other)
				: const_buf() {
					raw_set(other);
				}
				raw_const_buf& operator=(raw_const_buf const& other) {
					raw_set(other);
					return *this;
				}
				raw_const_buf(raw_const_buf&& other) = default;
				raw_const_buf& operator=(raw_const_buf&& other) = default;
				~raw_const_buf() = default;

				/* implicit */
				template<typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* =nullptr>
				explicit raw_const_buf(Container const& data)
				: const_buf(Storage::data(data), Storage::size(data)) {
				}

				/* implicit */
				explicit raw_const_buf(boost::asio::const_buffer data);

				/* implicit */
				explicit raw_const_buf(boost::asio::mutable_buffer data);

				void reset() {
					raw_reset();
				}
			};

			class shared_const_buf final: public const_buf {
			public:
				// typedef boost::variant<std::shared_ptr<void>> storage_t;
				typedef std::shared_ptr<void> storage_t;

				explicit shared_const_buf() = default;
				shared_const_buf(shared_const_buf const& other)
				: const_buf(), m_storage(other.m_storage) {
					raw_set(other);
				}
				shared_const_buf& operator=(shared_const_buf const& other) {
					m_storage = other.m_storage;
					raw_set(other);
					return *this;
				}
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
				static shared_const_buf copy(char const* data, std::size_t size) {
					return copy(reinterpret_cast<unsigned char const*>(data), size);
				}
				static shared_const_buf copy(const_buf const& buffer) {
					return copy(buffer.data(), buffer.size());
				}

				template<typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* =nullptr>
				static shared_const_buf copy(Container const& data) {
					return copy(Storage::data(data), Storage::size(data));
				}

				static shared_const_buf unsafe_use(storage_t storage, const_buf const& buffer) {
					return shared_const_buf(std::move(storage), buffer);
				}

				storage_t storage() const { return m_storage; }

			private:
				shared_const_buf internal_shared_slice(size_t from, size_t size) const override {
					return unsafe_use(m_storage, raw_slice(from, size));
				}

				explicit shared_const_buf(storage_t storage, const_buf const& buffer)
				: m_storage(storage) {
					raw_set(buffer);
				}

				storage_t m_storage; // pointer value is ignored
			};

			// (potentially sliced) reference to a const_buf, inheriting the original internal_shared_slice() method
			class tmp_const_buf final: public const_buf {
			public:
				explicit tmp_const_buf(const_buf const& backend)
				: m_backend(&backend) {
					raw_set(backend);
				}

				explicit tmp_const_buf(const_buf const& backend, const_buf const& slice)
				: m_backend(&backend) {
					raw_set(slice);
				}

				tmp_const_buf(tmp_const_buf const& other)
				: const_buf(), m_backend(other.m_backend) {
					raw_set(other);
				}
				tmp_const_buf& operator=(tmp_const_buf const& other) {
					m_backend = other.m_backend;
					raw_set(other);
					return *this;
				}
				tmp_const_buf(tmp_const_buf&&) = default;
				tmp_const_buf& operator=(tmp_const_buf&&) = default;

				tmp_const_buf slice(size_type from, size_type size) const {
					return tmp_const_buf(*m_backend, raw_slice(from, size));
				}
				tmp_const_buf slice(size_type from) const {
					return slice(from, size());
				}
				tmp_const_buf copy() const {
					return slice(0, size());
				}

			private:
				virtual shared_const_buf internal_shared_slice(size_t from, size_t size) const {
					size_t inner_from = data() - m_backend->data();
					return m_backend->shared_slice(inner_from + from, size);
				}

				const_buf const* m_backend{nullptr};
			};

			class mutable_buf {
			public:
				typedef unsigned char* iterator;
				typedef unsigned char const* const_iterator;
				typedef unsigned char value_type;
				typedef std::size_t size_type;
				typedef std::ptrdiff_t difference_type;

				size_type size() const { return m_buffer.size(); }
				bool empty() const { return 0 == size(); }
				explicit operator bool() const { return !empty(); }

				unsigned char* data() const { return const_cast<unsigned char*>(m_buffer.data()); }

				iterator begin() const { return data(); }
				iterator end() const { return data() + size(); }
				const_iterator cbegin() const { return data(); }
				const_iterator cend() const { return data() + size(); }

				char* char_begin() const { return reinterpret_cast<char*>(data()); }
				char* char_end() const { return reinterpret_cast<char*>(data()) + size(); }

				unsigned char& operator[](size_type ndx) const {
					if (ndx >= size()) std::terminate();
					return data()[ndx];
				}

				/* implicit */
				operator boost::asio::const_buffer() const;
				/* implicit */
				operator boost::asio::mutable_buffer() const;
				/* implicit */
				operator const_buf const&() const {
					return m_buffer;
				}

				raw_mutable_buf raw_slice(size_type from, size_type size) const;
				raw_mutable_buf raw_slice(size_type from) const;
				raw_mutable_buf raw_copy() const;

				unique_buf copy() const;

			protected:
				mutable_buf() = default;
				explicit mutable_buf(unsigned char* data, size_t size)
				: m_buffer(data, size) { }
				explicit mutable_buf(char* data, size_t size)
				: mutable_buf(reinterpret_cast<unsigned char*>(data), size) { }
				~mutable_buf() = default; // no need for virtual

				mutable_buf(mutable_buf const&) = delete;
				mutable_buf& operator=(mutable_buf const&) = delete;
				mutable_buf(mutable_buf&&) = default;
				mutable_buf& operator=(mutable_buf&&) = default;

				void raw_reset() {
					m_buffer.reset();
				}

				void raw_set(unsigned char* data, size_t size) {
					m_buffer = raw_const_buf(data, size);
				}
				void raw_set(char* data, size_t size) {
					raw_set(reinterpret_cast<unsigned char*>(data), size);
				}
				void raw_set(mutable_buf const& other) {
					raw_set(other.data(), other.size());
				}

			private:
				raw_const_buf m_buffer;
			};

			class raw_mutable_buf final: public mutable_buf {
			public:
				explicit raw_mutable_buf() = default;
				raw_mutable_buf(raw_mutable_buf const& other)
				: mutable_buf() {
					raw_set(other);
				}
				raw_mutable_buf& operator=(raw_mutable_buf const& other) {
					raw_set(other);
					return *this;
				}
				raw_mutable_buf(raw_mutable_buf&& other) = default;
				raw_mutable_buf& operator=(raw_mutable_buf&& other) = default;

				explicit raw_mutable_buf(unsigned char* data, std::size_t size)
				: mutable_buf(data, size) { }

				explicit raw_mutable_buf(char* data, std::size_t size)
				: mutable_buf(data, size) { }

				template<typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* =nullptr>
				explicit raw_mutable_buf(Container& data)
				: raw_mutable_buf(Storage::data(data), Storage::size(data)) {
				}

				explicit raw_mutable_buf(boost::asio::mutable_buffer data);

				void reset() {
					raw_reset();
				}
			};

			class unique_buf final: public mutable_buf {
			public:
				explicit unique_buf() = default;
				unique_buf(unique_buf const& other)
				: mutable_buf(), m_storage(other.m_storage) {
					raw_set(other);
				}
				unique_buf& operator=(unique_buf const& other) {
					m_storage = other.m_storage;
					raw_set(other);
					return *this;
				}
				unique_buf(unique_buf&& other) = default;
				unique_buf& operator=(unique_buf&& other) = default;

				/* implicit */
				template<typename Container, typename Storage = impl::buffer_storage<Container>, typename Storage::container_t* =nullptr>
				unique_buf(Container&& data) {
					std::shared_ptr<Container> storage = std::make_shared<Container>(std::move(data));
					m_storage = storage;
					raw_set(Storage::data(*storage), Storage::size(*storage));
				}

				static unique_buf copy(unsigned char const* data, std::size_t size);
				static unique_buf copy(char const* data, std::size_t size) {
					return copy(reinterpret_cast<unsigned char const*>(data), size);
				}
				static unique_buf copy(const_buf const& buffer) {
					return copy(buffer.data(), buffer.size());
				}

				// splits of <size> bytes at the beginning from the buffer and freezes them
				shared_const_buf freeze(std::size_t size);
				// freeze complete buffer
				shared_const_buf freeze();

				unique_buf slice(size_t from, size_t length)&& {
					unique_buf result{std::move(*this)};
					result.raw_set(result.raw_slice(from, length));
					return result;
				}

				unique_buf slice(size_t from)&& {
					return std::move(*this).slice(from, size());
				}

			private:
				template<std::size_t SIZE>
				friend unique_buf make_unique_buffer();
				friend unique_buf make_unique_buffer(std::size_t size);

				explicit unique_buf(std::shared_ptr<void> storage, unsigned char* data, std::size_t size)
				: mutable_buf(data, size), m_storage(storage) {
				}
				explicit unique_buf(std::shared_ptr<void> storage, char* data, std::size_t size)
				: mutable_buf(data, size), m_storage(storage) {
				}

				std::shared_ptr<void> m_storage;
			};

			/* inline implementations */

			inline shared_const_buf const_buf::shared_slice(size_type from, size_type size) const {
				from = std::min(from, m_size);
				size = std::min(m_size - from, size);
				return internal_shared_slice(from, size);
			}

			inline shared_const_buf const_buf::shared_slice(size_type from) const {
				return shared_slice(from, m_size);
			}

			inline shared_const_buf const_buf::shared_copy() const {
				return shared_slice(0, m_size);
			}

			inline raw_const_buf const_buf::raw_slice(size_type from, size_type size) const {
				from = std::min(from, m_size);
				size = std::min(m_size - from, size);
				return raw_const_buf(m_data + from, size);
			}

			inline raw_const_buf const_buf::raw_slice(size_type from) const {
				return raw_slice(from, m_size);
			}

			inline raw_const_buf const_buf::raw_copy() const {
				return raw_slice(0, m_size);
			}

			inline unique_buf const_buf::copy() const {
				return unique_buf::copy(*this);
			}

			inline shared_const_buf const_buf::internal_shared_slice(size_t from, size_t size) const {
				return shared_const_buf::copy(m_data + from, size);
			}

			inline raw_mutable_buf mutable_buf::raw_slice(size_type from, size_type length) const {
				from = std::min(from, size());
				length = std::min(size() - from, length);
				return raw_mutable_buf(data() + from, length);
			}

			inline raw_mutable_buf mutable_buf::raw_slice(size_type from) const {
				return raw_slice(from, size());
			}

			inline raw_mutable_buf mutable_buf::raw_copy() const {
				return raw_slice(0, size());
			}

			inline unique_buf mutable_buf::copy() const {
				return unique_buf::copy(*this);
			}

			template<std::size_t SIZE>
			unique_buf make_unique_buffer() {
				struct data { unsigned char buffer[SIZE]; };
				std::shared_ptr<data> ptr = std::make_shared<data>();
				return unique_buf(ptr, ptr->buffer, SIZE);
			}

			unique_buf make_unique_buffer(std::size_t size);

			inline raw_const_buf operator"" _cb(const char *str, std::size_t len) {
				return raw_const_buf(str, len);
			}
		}
	} // namespace memory
} // namespace caney
