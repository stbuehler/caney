#pragma once

#include "caney/memory/buffer.hpp"
#include "caney/std/tags.hpp"

#include "chunks.hpp"
#include "internal.hpp"
#include "streams.hpp"

#include <list>

#include <boost/asio.hpp>

__CANEY_STREAMSV1_BEGIN

/**
 * @brief An endpoint for some stream-oriented boost protocol.
 *
 * Provides a sink (to send data over the socket), a source (to receive data), an origin (to pause receiving data).
 *
 * Uses @ref chunk as chunk type.
 *
 * @tparam the (byte stream) protocol this endpoint is for. ``boost::asio::ip::tcp`` or ``boost::asio::local::stream_protocol``
 */
template <typename Protocol, typename StreamSocketService = boost::asio::stream_socket_service<Protocol>>
class asio_endpoint : public sink<chunk>,
					  public source<chunk>,
					  public origin,
					  public std::enable_shared_from_this<asio_endpoint<Protocol, StreamSocketService>> {
public:
	/** @brief the endpoints sink type */
	using sink_t = sink<chunk>;
	/** @brief the endpoints source type */
	using source_t = source<chunk>;
	/** @brief the endpoints chunkqueue type */
	using chunks_t = chunk_traits_t<chunk>::chunks_t;
	/** @brief the endpoints stream end type */
	using end_t = chunk_traits_t<chunk>::end_t;

	/** @brief share strands with this type */
	using shared_strand_t = std::shared_ptr<boost::asio::strand>;
	/** @brief boost socket type */
	using socket_t = boost::asio::basic_stream_socket<Protocol, StreamSocketService>;

	/** @brief shared_from_this */
	using std::enable_shared_from_this<asio_endpoint<Protocol, StreamSocketService>>::shared_from_this;

	/** @brief create instance */
	static std::shared_ptr<asio_endpoint> create(shared_strand_t strand, socket_t&& sock) {
		std::shared_ptr<asio_endpoint> self = std::make_shared<asio_endpoint>(private_tag, std::move(strand), std::move(sock));
		self->set_origin(self);
		return self;
	}

	//! @nowarn
	/** @internal @brief private constructor */
	asio_endpoint(private_tag_t, shared_strand_t strand, socket_t&& sock) : m_strand(std::move(strand)), m_socket(std::move(sock)) {}
	//! @endnowarn

private:
	// source events
	void on_pause() override {
		// could only cancel all socket operations, not the read alone
	}

	void start_read() {
		if (origin::is_paused() || m_is_reading || m_got_fin || !m_socket.is_open()) return;
		m_is_reading = true;

		if (m_read_buffer.size() < 1024) { m_read_buffer = memory::unique_buf::allocate(16 * 1024); }

		std::weak_ptr<asio_endpoint> weak_self{shared_from_this()};

		m_socket.async_read_some(
			boost::asio::mutable_buffers_1(m_read_buffer),
			m_strand->wrap([weak_self, this](const boost::system::error_code& error, std::size_t bytes_transferred) {
				std::shared_ptr<asio_endpoint> self = weak_self.lock();
				if (!self) return;
				m_is_reading = false;
				if (error == boost::asio::error::eof) {
					m_got_fin = true;
					source_t::send_end(StreamEnd::EndOfStream);
				} else if (error) {
					boost::system::error_code ec;
					m_socket.close(ec);
					source_t::send_end(StreamEnd::Aborted);
					sink_t::disconnect();
				} else if (bytes_transferred > 0) {
					source_t::send(chunk(m_read_buffer.freeze(bytes_transferred)));
					on_resume();
				}
			}));
	}

	void on_resume() override {
		start_read();
	}

	void on_disconnect() override {
		boost::system::error_code ec;
		m_socket.close(ec);
		source_t::send_end(StreamEnd::Aborted);
		sink_t::disconnect();
	}

	void on_connected_sink() override {
		if (!m_socket.is_open()) {
			source_t::send_end(StreamEnd::Aborted);
		} else if (m_got_fin) {
			source_t::send_end(StreamEnd::EndOfStream);
		} else {
			on_resume();
		}
	}

	// sink events
	void on_connected_source() override {
		if (!m_socket.is_open()) { disconnect(); }
	}

	void on_end(end_t end) override {
		boost::system::error_code ec;

		switch (end) {
		case StreamEnd::EndOfStream:
			m_socket.shutdown(Protocol::socket::shutdown_send, ec);
			if (ec) {
				m_socket.close();
				source_t::send_end(StreamEnd::Aborted);
			}
			break;
		case StreamEnd::Aborted:
			m_socket.close(ec);
			source_t::send_end(StreamEnd::Aborted);
			break;
		}
	}

	void start_write() {
		if (m_is_writing || m_write_queue.empty() || !m_socket.is_open()) return;
		m_is_writing = true;

		std::vector<boost::asio::const_buffer> data;
		for (chunk const& c : m_write_queue.queue()) {
			caney::optional<boost::asio::const_buffer> c_buffer = c.get_const_buffer();
			if (c_buffer) {
				data.emplace_back(c_buffer.get());
			} else {
				break;
			}
		}

		if (data.empty()) {
			// can't handle file chunks yet
			std::terminate();
		}

		std::weak_ptr<asio_endpoint> weak_self{shared_from_this()};

		m_socket.async_write_some(data, m_strand->wrap([weak_self, this](const boost::system::error_code& error, std::size_t bytes_transferred) {
			std::shared_ptr<asio_endpoint> self = weak_self.lock();
			if (!self) return;
			m_is_writing = false;
			if (error) {
				boost::system::error_code ec;
				m_socket.close(ec);
				source_t::send_end(StreamEnd::Aborted);
				sink_t::disconnect();
			} else {
				m_write_queue.remove(file_size{bytes_transferred});
				start_write();
			}
		}));
	}

	void on_receive(chunks_t&& chunks) override {
		m_write_queue.append(std::move(chunks));
		start_write();
	}

	memory::unique_buf m_read_buffer;
	bool m_is_reading = false, m_got_fin = false;

	chunks_t m_write_queue;
	bool m_is_writing = false;

	shared_strand_t m_strand;
	socket_t m_socket;
};

template class asio_endpoint<boost::asio::ip::tcp>;

__CANEY_STREAMSV1_END
