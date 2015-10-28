#pragma once

#include <memory>

#include "caney/std/object.hpp"
#include "caney/std/optional.hpp"

#include "generic_chunks.hpp"
#include "internal.hpp"
#include "origin.hpp"

__CANEY_STREAMSV1_BEGIN

class sink_base;

template <typename Chunk>
class sink;

class source_base;

template <typename Chunk>
class source;

/**
 * @brief base class for @ref sink, independent of chunk type
 */
class sink_base : private caney::object {
public:
	/** @brief whether the sink is currently pausing the origin */
	bool is_paused() const;

protected:
	/**
	 * @brief pause the origin. while the origin is paused the sink
	 * will not receive any data (it is kept in the source).
	 */
	void pause();

	/** @brief get origin */
	std::shared_ptr<origin> const& get_origin() const;

	/** @brief called when a new origin is set (by the source this sink is connected to) */
	virtual void on_new_origin(std::shared_ptr<origin> const& new_origin);
	/** @brief called when connected to a source */
	virtual void on_connected_source() {}

private:
	template <typename Chunk>
	friend class sink;

	template <typename Chunk>
	friend class source;

	template <typename Source, typename Sink>
	friend void connect(std::shared_ptr<Source> from, std::shared_ptr<Sink> to);

	void set_new_origin(std::shared_ptr<origin> new_origin);

	void resume();

	bool m_is_paused = false;
	std::shared_ptr<origin> m_origin;
	origin_pause m_origin_pause;
};

/**
 * @brief a sink represents an object that receives @tparam Chunk (s).
 * It gets connected to a source, which emits the chunks.
 */
template <typename Chunk>
class sink : public sink_base {
public:
	using sink_t = sink<Chunk>; //!< sink type
	using source_t = source<Chunk>; //!< source type
	using chunks_t = typename chunk_traits_t<Chunk>::chunks_t; //!< chunk queue type
	using end_t = typename chunk_traits_t<Chunk>::end_t; //!< end stream type

	/** @brief get source this sink is connected to */
	std::shared_ptr<source_t> const& get_source() const {
		return m_source;
	}

protected:
	sink() = default;

	/**
	 * @brief resume origin after it was paused
	 */
	void resume() {
		sink_base::resume();
		// not paused anymore, get pending data
		if (m_source) m_source->send_pending();
	}

	/**
	 * @brief receive data from currently connected source
	 *
	 * receives no data while the sink is paused.
	 */
	virtual void on_receive(chunks_t&& chunks) = 0;
	/**
	 * @brief receive stream end notification from currently connected source
	 *
	 * not called while the sink is paused.
	 */
	virtual void on_end(end_t end) = 0;

	/** @brief disconnect sink from currently connected source */
	void disconnect() {
		if (!m_source) return;
		std::shared_ptr<source_t> old_source = std::move(m_source);
		m_source.reset();
		old_source->m_sink.reset();
		old_source->on_disconnect();

		// if somehow already reconnected don't touch origin
		if (!m_source) set_new_origin(nullptr);
	}

private:
	template <typename FChunk>
	friend class source;

	template <typename Source, typename Sink>
	friend void connect(std::shared_ptr<Source> from, std::shared_ptr<Sink> to);

	std::shared_ptr<source_t> m_source;
};

/**
 * @brief base class for @ref source, independent of chunk type
 */
class source_base : private caney::object {
public:
	/**
	 * @brief get origin responsible for creating data, which takes the
	 * pause/resume notifications
	 */
	std::shared_ptr<origin> get_origin() const;

protected:
	/** @brief called when a sink is connected to the source */
	virtual void on_connected_sink() {}

private:
	template <typename Chunk>
	friend class source;

	template <typename Source, typename Sink>
	friend void connect(std::shared_ptr<Source> from, std::shared_ptr<Sink> to);

	// source might be the origin itself: don't keep itself alive here
	std::weak_ptr<origin> m_origin;
};

/**
 * @brief a source represents an object that emits @tparam Chunk (s).
 * It gets connected to a sink which receives the chunks.
 */
template <typename Chunk>
class source : public source_base {
public:
	using sink_t = sink<Chunk>; //!< sink type
	using source_t = source<Chunk>; //!< source type
	using chunks_t = typename chunk_traits_t<Chunk>::chunks_t; //!< chunk queue type
	using end_t = typename chunk_traits_t<Chunk>::end_t; //!< end stream type

	/** @brief get sink this source is connected to */
	std::shared_ptr<sink_t> const& get_sink() const {
		return m_sink;
	}

protected:
	source() = default;

	/** @brief set new origin, propagate new origin to connected sink */
	void set_origin(std::shared_ptr<origin> new_origin) {
		std::shared_ptr<origin> old_origin = m_origin.lock();
		if (old_origin == new_origin) return;
		m_origin = new_origin;
		if (m_sink) m_sink->set_new_origin(std::move(new_origin));
	}

	/** @brief returns whether send() / send_end() actually can send data or should just queue it */
	bool can_send() const {
		return m_sink && !m_out_pending && !m_sink->is_paused();
	}

	/** @brief send chunks if possible or store them in the pending queue */
	void send(chunks_t&& chunks) {
		if (can_send()) {
			m_sink->on_receive(std::move(chunks));
		} else {
			m_out_pending = true;
			if (m_out_end) std::terminate();
			chunk_traits_t<Chunk>::append(m_out_queue, std::move(chunks));
		}
	}

	/** @brief send end of stream notification if possible or store it in the pending queue */
	void send_end(end_t end) {
		if (can_send()) {
			std::shared_ptr<sink_t> old_sink = m_sink;
			m_sink->m_source.reset();
			m_sink.reset();
			old_sink->on_end(end);
			if (old_sink->m_source) return;
			old_sink->set_new_origin(nullptr);
		} else {
			m_out_pending = true;
			/* ignore if we already have an end */
			if (!m_out_end) m_out_end = std::move(end);
		}
	}

	/** @brief called when disconnected from sink */
	virtual void on_disconnect() = 0;

private:
	template <typename FChunk>
	friend class sink;

	template <typename Source, typename Sink>
	friend void connect(std::shared_ptr<Source> from, std::shared_ptr<Sink> to);

	void send_pending() {
		std::shared_ptr<sink_t> sink = m_sink;
		// send pending data to sink while sink == m_sink && !sink->is_paused
		// reset m_out_pending if finished successfully
		if (m_out_pending) {
			while (!sink->is_paused() && !chunk_traits_t<Chunk>::empty(m_out_queue)) {
				chunks_t chunks = std::move(m_out_queue);
				chunk_traits_t<Chunk>::clear(m_out_queue);
				m_out_pending = bool(m_out_end);

				// can_send() might still return false, send directly
				sink->on_receive(std::move(chunks));
				if (sink != m_sink) return;
			}

			if (!sink->is_paused() && m_out_end) {
				end_t end = std::move(m_out_end.value());
				m_out_end = caney::nullopt;

				m_out_pending = false;

				std::shared_ptr<sink_t> old_sink = m_sink;
				m_sink->m_source.reset();
				m_sink.reset();
				old_sink->on_end(end);
				if (old_sink->m_source) return;
				old_sink->set_new_origin(nullptr);

				send_end(std::move(end));
			}
		}
	}

	std::shared_ptr<sink_t> m_sink;

	// don't send anything while we have stuff pending.
	// connect() will try to send pending data, and resets the flag.
	bool m_out_pending = false;
	chunks_t m_out_queue;
	caney::optional<end_t> m_out_end;
};

/**
 * @brief transform data from @tparam ChunkIn chunks to @tparam ChunkOut chunks.
 *
 * - when source gets disconnected from connected sink it also disconnects the own sink.
 * - pauses sink (when a source is connected) while there is no connected sink
 * - propagates origin
 *
 * you need to implement ``sink<ChunkIn>::on_end()`` and ``sink<ChunkIn>::on_receive``
 */
template <typename ChunkIn, typename ChunkOut>
class transform : public sink<ChunkIn>, public source<ChunkOut> {
public:
	using sink_t = sink<ChunkIn>; //!< own sink type
	using source_t = source<ChunkOut>; //!< own source type
	using chunks_in_t = typename sink_t::chunks_t; //!< chunk queue type for incoming chunks
	using chunks_out_t = typename source_t::chunks_t; //!< chunk queue type for outgoing chunks

protected:
	/** @brief propagate origin by default */
	void on_new_origin(std::shared_ptr<origin> const& new_origin) override {
		source_t::set_origin(new_origin);
	}

	/** @brief disconnect sink when source gets disconnected by default */
	void on_disconnect() override {
		sink_t::disconnect();
	}

	/** @brief pause new connected source when the own source is not connected (i.e. nowhere the data could go yet) */
	void on_connected_source() override {
		if (!source_t::get_sink()) { sink_t::pause(); }
	}

	/** @brief when the source gets a sink we can start processing incoming data */
	void on_connected_sink() override {
		sink_t::resume();
	}
};

/**
 * @brief a filter is a @ref transform which doesn't change the chunk type,
 * and simply forwards data and stream end notifications by default.
 */
template <typename Chunk>
class filter : public transform<Chunk, Chunk> {
public:
	using transform_t = transform<Chunk, Chunk>; //!< base transform type
	using sink_t = sink<Chunk>; //!< sink type
	using source_t = source<Chunk>; //!< source type
	using chunks_t = typename chunk_traits_t<Chunk>::chunks_t; //!< chunk queue type
	using end_t = typename chunk_traits_t<Chunk>::end_t; //!< end stream type

protected:
	/** @brief forward data by default */
	void on_receive(chunks_t&& chunks) override {
		source_t::send(std::move(chunks));
	}

	/** @brief forward stream end notifications by default */
	void on_end(end_t end) override {
		source_t::send_end(end);
	}
};

/** @brief determine chunk type of source by finding @ref source base class */
template <typename Source>
struct source_traits {
private:
	template <typename Chunk>
	static Chunk helper_func(source<Chunk> const&);

public:
	/** @brief chunk type */
	using chunk_t = decltype(helper_func(std::declval<Source>()));
	/** @brief base source type */
	using source_t = source<chunk_t>;
};

/** @brief determine chunk type of sink by finding @ref sink base class */
template <typename Sink>
struct sink_traits {
private:
	template <typename Chunk>
	static Chunk helper_func(sink<Chunk> const&);

public:
	/** @brief chunk type */
	using chunk_t = decltype(helper_func(std::declval<Sink>()));
	/** @brief base sink type */
	using sink_t = sink<chunk_t>;
};

/**
 * @brief connect a source and a sink
 *
 * also notify sink of origin of source, and send pending data from the source to the sink
 */
template <typename Source, typename Sink>
void connect(std::shared_ptr<Source> from, std::shared_ptr<Sink> to) {
	using from_traits = source_traits<Source>;
	using to_traits = sink_traits<Sink>;
	using source_t = typename from_traits::source_t;
	using sink_t = typename to_traits::sink_t;

	static_assert(std::is_same<typename from_traits::chunk_t, typename to_traits::chunk_t>::value, "chunk types have to match");
	std::shared_ptr<source_t> from_base{from};
	std::shared_ptr<sink_t> to_base{to};

	if (!from_base || !to_base || from_base->m_sink || to_base->m_source) { std::terminate(); }

	from_base->m_sink = to_base;
	to_base->m_source = from_base;

	to_base->set_new_origin(from_base->get_origin());
	if (to_base != from_base->m_sink) return;

	// first to_base has a chance to call pause() before from_base might call resume()
	to_base->on_connected_source();
	if (to_base != from_base->m_sink) return;

	from_base->on_connected_sink();
	if (to_base != from_base->m_sink) return;

	from_base->send_pending();
}

__CANEY_STREAMSV1_END
