#include "caney/std/wqueue.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(wqueue)

BOOST_AUTO_TEST_CASE(test) {
	caney::wqueue<uint32_t> queue;
	queue.emplace(1);
	uint32_t elem = queue.wait();
	BOOST_CHECK_EQUAL(elem, 1);
}

BOOST_AUTO_TEST_SUITE_END()
