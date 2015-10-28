#pragma once

#include "internal.hpp"

#include <list>

__CANEY_STREAMSV1_BEGIN

/** @brief reasons a stream could end for */
enum class StreamEnd {
	EndOfStream, //! regular end of stream
	Aborted, //! aborted (user, IO error, ...)
};

/** @brief declare ''std::list<Chunk>'' as the default queue type for any @tparam Chunk */
template <typename Chunk>
struct chunk_traits_t {
	/** @brief generic queue type */
	using chunks_t = std::list<Chunk>;

	/** @brief move @param chunks to another queue @param to */
	static void append(chunks_t& to, chunks_t&& chunks) {
		to.splice(to.end(), std::move(chunks));
	}

	/** @brief whether there are no chunks (doesn't care whether the chunks itself are empty or not) */
	static bool empty(chunks_t const& chunks) {
		return chunks.empty();
	}

	/** @brief drop all chunks */
	static void clear(chunks_t& chunks) {
		chunks.clear();
	}

	/** @brief reasons a stream could end for */
	using end_t = StreamEnd;
};

__CANEY_STREAMSV1_END
