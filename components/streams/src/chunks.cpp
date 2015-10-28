#include "caney/streams/chunks.hpp"

#include "caney/streams/streams.hpp"

#include <boost/asio/buffer.hpp>

__CANEY_STREAMSV1_BEGIN

memory_chunk::memory_chunk(memory::shared_const_buf&& buffer) : m_buffer(std::move(buffer)) {}

file_size memory_chunk::bytes() const {
	return file_size{m_buffer.size()};
}

memory_chunk memory_chunk::split(file_size bytes) {
	memory_chunk result{m_buffer.shared_slice(0, boost::numeric_cast<std::size_t>(bytes.get()))};
	remove(bytes);
	return result;
}

void memory_chunk::remove(file_size bytes) {
	m_buffer = m_buffer.shared_slice(boost::numeric_cast<std::size_t>(bytes.get()));
}

boost::asio::const_buffer memory_chunk::get_const_buffer() const {
	return m_buffer;
}

namespace {
	struct chunk_split_visitor : public boost::static_visitor<chunk> {
		file_size m_bytes;
		chunk_split_visitor(file_size bytes) : m_bytes(bytes) {}

		template <typename T>
		chunk operator()(T& content) const {
			return chunk(content.split(m_bytes));
		}
	};

	struct chunk_remove_visitor : public boost::static_visitor<void> {
		file_size m_bytes;
		chunk_remove_visitor(file_size bytes) : m_bytes(bytes) {}

		template <typename T>
		void operator()(T& content) const {
			content.remove(m_bytes);
		}
	};

	struct chunk_get_const_buffer : public boost::static_visitor<caney::optional<boost::asio::const_buffer>> {
		template <typename T>
		caney::optional<boost::asio::const_buffer> operator()(T& content) const {
			return content.get_const_buffer();
		}
	};
}

chunk::chunk(memory_chunk&& chunk) : m_value(std::move(chunk)) {}

chunk::chunk(memory::shared_const_buf&& buffer) : chunk(memory_chunk(std::move(buffer))) {}

file_size chunk::bytes() const {
	return file_size(boost::get<memory_chunk>(m_value).bytes());
}

chunk chunk::split(file_size bytes) {
	return boost::apply_visitor(chunk_split_visitor(bytes), m_value);
}

void chunk::remove(file_size bytes) {
	boost::apply_visitor(chunk_remove_visitor(bytes), m_value);
}

caney::optional<boost::asio::const_buffer> chunk::get_const_buffer() const {
	return boost::apply_visitor(chunk_get_const_buffer(), m_value);
}

chunk_queue::chunk_queue(chunk&& c) {
	m_queue.emplace_back(std::move(c));
}

void chunk_queue::append(chunk&& c) {
	m_queue.emplace_back(std::move(c));
}

void chunk_queue::append(chunk_queue&& other) {
	m_queue.splice(m_queue.end(), std::move(other.m_queue));
}

bool chunk_queue::empty() const {
	return m_queue.empty();
}

void chunk_queue::clear() {
	m_queue.clear();
}

std::list<chunk> const& chunk_queue::queue() const {
	return m_queue;
}

chunk_queue chunk_queue::split(file_size bytes) {
	chunk_queue result;
	while (bytes > file_size{0}) {
		if (m_queue.empty()) std::terminate();
		if (m_queue.front().bytes() >= bytes) {
			bytes -= m_queue.front().bytes();
			// move chunk from m_queue to result.m_queue
			result.m_queue.splice(result.m_queue.end(), m_queue, m_queue.begin());
		} else {
			result.append(m_queue.front().split(bytes));
			return result;
		}
	}
	return result;
}

void chunk_queue::remove(file_size bytes) {
	while (bytes > file_size{0}) {
		if (m_queue.empty()) std::terminate();
		if (m_queue.front().bytes() >= bytes) {
			bytes -= m_queue.front().bytes();
			m_queue.pop_front();
		} else {
			m_queue.front().remove(bytes);
			return;
		}
	}
}

void foo1() {
	using Chunk = std::vector<uint8_t>;
	using filter_t = filter<Chunk>;

	std::shared_ptr<filter_t> filter = std::make_shared<filter_t>();
	connect(filter, filter);
}

void foo2() {
	using Chunk = chunk;
	using filter_t = filter<Chunk>;

	std::shared_ptr<filter_t> filter = std::make_shared<filter_t>();
	connect(filter, filter);
}

__CANEY_STREAMSV1_END
