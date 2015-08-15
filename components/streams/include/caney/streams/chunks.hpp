#pragma once

#include "caney/memory/buffer.hpp"

#include "generic_chunks.hpp"
#include "file_chunk.hpp"

#include <list>

#include <boost/variant.hpp>

namespace caney {
	namespace streams {
		inline namespace v1 {
			class memory_chunk {
			public:
				explicit memory_chunk(memory::shared_const_buf&& buffer);

				file_size bytes() const;

				// return first "bytes" data, forward self by "bytes"
				memory_chunk split(file_size bytes);
				// drop first "bytes" data
				void remove(file_size bytes);

				boost::asio::const_buffer get_const_buffer() const;

			private:
				memory::shared_const_buf m_buffer;
			};

			class chunk {
			public:
				using value_t = boost::variant<memory_chunk>;

				explicit chunk(memory_chunk&& chunk);

				explicit chunk(memory::shared_const_buf&& buffer);

				file_size bytes() const;

				chunk split(file_size bytes);
				void remove(file_size bytes);

				// not all chunk types support this method and will return boost::none
				// if the method is supported it must always return a (possibly empty) const_buffer
				boost::optional<boost::asio::const_buffer> get_const_buffer() const;

				value_t const& get() const {
					return m_value;
				}

			private:
				value_t m_value;
			};

			class chunk_queue {
			public:
				using container_t = std::list<chunk>;

				chunk_queue() = default;
				chunk_queue(chunk&& c);

				void append(chunk&& c);
				void append(chunk_queue&& other);
				bool empty() const;
				void clear();

				container_t const& queue();

				chunk_queue split(file_size bytes);
				void remove(file_size bytes);

			private:
				file_size m_bytes{0};
				container_t m_queue;
			};

			template<>
			struct chunk_traits_t<chunk> {
				using chunks_t = chunk_queue;

				static void append(chunks_t& to, chunks_t&& chunks) {
					to.append(std::move(chunks));
				}

				static bool empty(chunks_t const& chunks) {
					return chunks.empty();
				}

				static void clear(chunks_t& chunks) {
					return chunks.clear();
				}

				using end_t = StreamEnd;
			};
		}
	} // namespace streams
} // namespace caney
