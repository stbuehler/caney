#include "caney/std/safe_int.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(safe_int)

BOOST_AUTO_TEST_CASE(test_int8) {
	caney::safe_int<int8_t> a{127};

	// caney::safe_int<unsigned int> a{1}, b{2}, c{3}, d = a + b + c;
}

BOOST_AUTO_TEST_SUITE_END()
