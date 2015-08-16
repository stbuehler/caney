#pragma once

#include <memory>

#include <boost/optional.hpp>

#include "caney/std/object.hpp"

#include "origin.hpp"
#include "generic_chunks.hpp"

namespace caney {
	namespace streams {
		inline namespace v1 {
			class sink_base;

			template<typename Chunk>
			class sink;

			class source_base;

			template<typename Chunk>
			class source;

			class sink_base : private caney::object {
			public:
				bool is_paused() const;

			protected:
				void pause();

				std::shared_ptr<origin> const& get_origin() const;

				virtual void on_new_origin(std::shared_ptr<origin> const& new_origin);
				virtual void on_connected_source() { }

			private:
				template<typename Chunk>
				friend class sink;

				template<typename Chunk>
				friend class source;

				template<typename Source, typename Sink>
				friend void connect(std::shared_ptr<Source> from, std::shared_ptr<Sink> to);

				void set_new_origin(std::shared_ptr<origin> new_origin);

				void resume();

				bool m_is_paused = false;
				std::shared_ptr<origin> m_origin;
				origin_pause m_origin_pause;
			};

			template<typename Chunk>
			class sink : public sink_base {
			public:
				using sink_t = sink<Chunk>;
				using source_t = source<Chunk>;
				using chunks_t = typename chunk_traits_t<Chunk>::chunks_t;
				using end_t = typename chunk_traits_t<Chunk>::end_t;

				std::shared_ptr<source_t> const& get_source() const {
					return m_source;
				}

			protected:
				sink() = default;

				void resume() {
					std::shared_ptr<source_t> source = m_source;
					// m_source might get reset, need local copy
					if (source) source->send_pending();
					sink_base::resume();
				}

				// sinks might also get data when they are paused
				virtual void on_receive(chunks_t&& chunks) = 0;
				virtual void on_end(end_t end) = 0;

				void disconnect() {
					if (!m_source) return;
					std::shared_ptr<source_t> old_source = std::move(m_source);
					m_source.reset();
					old_source->m_sink.reset();

					set_new_origin(nullptr);

					old_source->on_disconnect();
				}

			private:
				template<typename FChunk>
				friend class source;

				template<typename Source, typename Sink>
				friend void connect(std::shared_ptr<Source> from, std::shared_ptr<Sink> to);

				std::shared_ptr<source_t> m_source;
			};

			class source_base : private caney::object {
			public:
				std::weak_ptr<origin> const& get_origin() const;

			protected:
				virtual void on_connected_sink() { }

			private:
				template<typename Chunk>
				friend class source;

				template<typename Source, typename Sink>
				friend void connect(std::shared_ptr<Source> from, std::shared_ptr<Sink> to);

				// source might be the origin itself: don't keep itself alive here
				std::weak_ptr<origin> m_origin;
			};

			template<typename Chunk>
			class source : public source_base {
			public:
				using sink_t = sink<Chunk>;
				using source_t = source<Chunk>;
				using chunks_t = typename chunk_traits_t<Chunk>::chunks_t;
				using end_t = typename chunk_traits_t<Chunk>::end_t;

				std::shared_ptr<sink_t> const& get_sink() const {
					return m_sink;
				}

			protected:
				source() = default;
				source(std::shared_ptr<sink_t> sink);

				void set_origin(std::shared_ptr<origin> new_origin) {
					std::shared_ptr<origin> old_origin = m_origin.lock();
					if (old_origin == new_origin) return;
					m_origin = new_origin;
					if (m_sink) m_sink->set_new_origin(std::move(new_origin));
				}

				// returns whether send() / send_end() actually send data or just
				// queue them
				bool can_send() const {
					return m_sink && !m_out_pending && !m_sink->is_paused();
				}

				void send(chunks_t&& chunks) {
					if (can_send()) {
						m_sink->on_receive(std::move(chunks));
					} else {
						m_out_pending = true;
						if (m_out_end.is_initialized()) std::terminate();
						chunk_traits_t<Chunk>::append(m_out_queue, std::move(chunks));
					}
				}

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
						if (!m_out_end.is_initialized()) m_out_end = std::move(end);
					}
				}

				virtual void on_disconnect() = 0;

				// don't send anything while we have stuff pending.
				// connect() will try to send pending data, and resets the flag.
				bool m_out_pending = false;
				chunks_t m_out_queue;
				boost::optional<end_t> m_out_end;

			private:
				template<typename FChunk>
				friend class sink;

				template<typename Source, typename Sink>
				friend void connect(std::shared_ptr<Source> from, std::shared_ptr<Sink> to);

				void send_pending() {
					std::shared_ptr<sink_t> sink = m_sink;
					// send pending data to sink while sink == m_sink && !sink->is_paused
					// reset m_out_pending if finished successfully
					if (m_out_pending) {
						while (!sink->is_paused() && !chunk_traits_t<Chunk>::empty(m_out_queue)) {
							chunks_t chunks = std::move(m_out_queue);
							chunk_traits_t<Chunk>::clear(m_out_queue);
							m_out_pending = m_out_end.is_initialized();

							// can_send() might still return false, send directly
							sink->on_receive(std::move(chunks));
							if (sink != m_sink) return;
						}

						if (!sink->is_paused() && m_out_end.is_initialized()) {
							end_t end = std::move(m_out_end.get());
							m_out_end = boost::none;

							m_out_pending = false;
							send_end(std::move(end));
						}
					}
				}

				std::shared_ptr<sink_t> m_sink;
			};

			template<typename ChunkIn, typename ChunkOut>
			class transform : public sink<ChunkIn>, public source<ChunkOut> {
			public:
				using sink_t = sink<ChunkIn>;
				using source_t = source<ChunkOut>;
				using chunks_in_t = typename sink_t::chunks_t;
				using chunks_out_t = typename source_t::chunks_t;

			protected:
				void on_new_origin(std::shared_ptr<origin> const& new_origin) override {
					source_t::set_origin(new_origin);
				}

				void on_disconnect() override {
					sink_t::disconnect();
				}

				void on_connected_source() override {
					if (!source_t::get_sink()) {
						sink_t::pause();
					}
				}

				void on_connected_sink() override {
					sink_t::resume();
				}
			};

			template<typename Chunk>
			class filter : public transform<Chunk, Chunk> {
			public:
				using transform_t = transform<Chunk, Chunk>;
				using sink_t = sink<Chunk>;
				using source_t = source<Chunk>;
				using chunks_t = typename chunk_traits_t<Chunk>::chunks_t;
				using end_t = typename chunk_traits_t<Chunk>::end_t;

			protected:
				void on_end(end_t end) override {
					source_t::send_end(end);
				}

				void on_receive(chunks_t&& chunks) override {
					source_t::send(std::move(chunks));
				}
			};

			template<typename Source>
			struct source_traits {
			private:
				template<typename Chunk>
				static Chunk helper_func(source<Chunk> const&);

			public:
				using chunk_t = decltype(helper_func(std::declval<Source>()));
				using source_t = source<chunk_t>;
			};

			template<typename Sink>
			struct sink_traits {
			private:
				template<typename Chunk>
				static Chunk helper_func(sink<Chunk> const&);

			public:
				using chunk_t = decltype(helper_func(std::declval<Sink>()));
				using sink_t = sink<chunk_t>;
			};

			template<typename Source, typename Sink>
			void connect(std::shared_ptr<Source> from, std::shared_ptr<Sink> to) {
				using from_traits = source_traits<Source>;
				using to_traits = sink_traits<Sink>;
				using source_t = typename from_traits::source_t;
				using sink_t = typename to_traits::sink_t;

				static_assert(std::is_same<typename from_traits::chunk_t, typename to_traits::chunk_t>::value, "chunk types have to match");
				std::shared_ptr<source_t> from_base{from};
				std::shared_ptr<sink_t> to_base{to};

				if (!from_base || !to_base || from_base->m_sink || to_base->m_source) {
					std::terminate();
				}

				from_base->m_sink = to_base;
				to_base->m_source = from_base;

				to_base->set_new_origin(from_base->get_origin().lock());
				if (to_base != from_base->m_sink) return;

				// first to_base has a chance to call pause() before from_base might call resume()
				to_base->on_connected_source();
				if (to_base != from_base->m_sink) return;

				from_base->on_connected_sink();
				if (to_base != from_base->m_sink) return;

				from_base->send_pending();
			}
		}
	} // namespace streams
} // namespace caney
