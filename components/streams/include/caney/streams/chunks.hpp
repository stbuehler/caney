#pragma once

#include "caney/memory/buffer.hpp"

#include "file_chunk.hpp"
#include "generic_chunks.hpp"
#include "internal.hpp"

#include <list>

#include <boost/variant.hpp>

__CANEY_STREAMSV1_BEGIN

/**
 * @brief represents a chunk of memory, using @ref memory::shared_const_buf as storage
 *
 * it is wrapped to prepare is being used as value in @ref chunk
 */
class memory_chunk {
public:
	/** @brief construct memory_chunk */
	explicit memory_chunk(memory::shared_const_buf&& buffer);

	/** @brief size of chunk in bytes */
	file_size bytes() const;

	/** @brief split chunk; return first "bytes" in new @ref memory_chunk, remainder is kept in current chunk */
	memory_chunk split(file_size bytes);
	/** @brief like split, but does not return anything; i.e. removes first "bytes" of data */
	void remove(file_size bytes);

	/** @brief access underlying storage */
	boost::asio::const_buffer get_const_buffer() const;

private:
	memory::shared_const_buf m_buffer;
};

/**
 * @brief stores a chunk and provides convenient access to it.
 *
 * right now only @ref memory_chunk is supported.
 * TODO: support @ref file_chunk
 */
class chunk {
public:
	/** @brief underlying value type storing the actual chunk */
	using value_t = boost::variant<memory_chunk>;

	/** @brief construct a @ref chunk from a @ref memory_chunk */
	explicit chunk(memory_chunk&& chunk);

	/** @brief construct a @ref chunk using @ref memory_chunk from @ref memory::shared_const_buf */
	explicit chunk(memory::shared_const_buf&& buffer);

	/** @brief size of chunk in bytes */
	file_size bytes() const;

	/** @brief split chunk; return first "bytes" in new @ref chunk, remainder is kept in current chunk */
	chunk split(file_size bytes);
	/** @brief like split, but does not return anything; i.e. removes first "bytes" of data */
	void remove(file_size bytes);

	/**
	 * @brief access underlying storage in memory if possible. chunk types not supporting this method will return @ref nullopt
	 *
	 * chunk types supporting this method must return an empty buffer instead of @ref nullopt if they are empty.
	 */
	caney::optional<boost::asio::const_buffer> get_const_buffer() const;

	/** @brief get underlying chunk value */
	value_t const& get() const {
		return m_value;
	}

private:
	value_t m_value;
};

/**
 * @brief A queue of @ref chunk.
 * Also keeps track of how may bytes there are in the queue.
 */
class chunk_queue {
public:
	/** @brief container to manage list of chunks */
	using container_t = std::list<chunk>;

	chunk_queue() = default;
	/** @brief construct container containing one chunk */
	chunk_queue(chunk&& c);

	/** @brief append a chunk at the end of the queue */
	void append(chunk&& c);
	/** @brief move all chunks from another chunkqueue and append them to the end of the queue */
	void append(chunk_queue&& other);
	/** @brief whether there are no chunks (doesn't care whether the chunks itself are empty or not) */
	bool empty() const;
	/** @brief drop all chunks */
	void clear();

	/** @brief read-only view of the underlying queue */
	container_t const& queue() const;

	/** @brief split chunk_queue; return first "bytes" in new @ref chunk_queue, remainder is kept in current chunk_queue */
	chunk_queue split(file_size bytes);
	/** @brief like split, but does not return anything; i.e. removes first "bytes" of data */
	void remove(file_size bytes);

private:
	file_size m_bytes{0};
	container_t m_queue;
};

/** declare @ref chunk_queue as *THE* queue type for @ref chunk */
template <>
struct chunk_traits_t<chunk> {
	/** @brief queue type */
	using chunks_t = chunk_queue;

	/** @brief move @param chunks to another queue @param to */
	static void append(chunks_t& to, chunks_t&& chunks) {
		to.append(std::move(chunks));
	}

	/** @brief whether there are no chunks (doesn't care whether the chunks itself are empty or not) */
	static bool empty(chunks_t const& chunks) {
		return chunks.empty();
	}

	/** @brief drop all chunks */
	static void clear(chunks_t& chunks) {
		return chunks.clear();
	}

	/** @brief reasons a stream could end for */
	using end_t = StreamEnd;
};

__CANEY_STREAMSV1_END
