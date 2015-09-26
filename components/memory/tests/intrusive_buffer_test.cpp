#include "caney/memory/intrusive_buffer.hpp"
#include "caney/memory/intrusive_buffer_pool.hpp"
#include "caney/memory/buffer.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(intrusive_buffer)

BOOST_AUTO_TEST_CASE(constructing) {
	{
		caney::memory::intrusive_buffer_ptr buf = caney::memory::make_intrusive_buffer(1024);
		unsigned char x = 15;
		for (auto& c: *buf) {
			c = x++;
		}
		auto constSharedBuf = caney::memory::shared_const_buf::unsafe_use(buf);
	}

	{
		caney::memory::intrusive_buffer_pool<> pool(512);
		auto buf = pool.allocate();
	}
}

BOOST_AUTO_TEST_SUITE_END()
