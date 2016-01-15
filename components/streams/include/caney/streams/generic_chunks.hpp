#pragma once

#include <list>

namespace caney {
	namespace streams {
		inline namespace v1 {
			enum class StreamEnd {
				EndOfStream,
				Aborted,
			};

			template <typename Chunk>
			struct chunk_traits_t {
				using chunks_t = std::list<Chunk>;

				static void append(chunks_t& to, chunks_t&& chunks) {
					to.splice(to.end(), std::move(chunks));
				}

				static bool empty(chunks_t const& chunks) {
					return chunks.empty();
				}

				static void clear(chunks_t& chunks) {
					chunks.clear();
				}

				using end_t = StreamEnd;
			};
		}
	} // namespace streams
} // namespace caney
