#include "caney/std/error_code.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(error_code)

BOOST_AUTO_TEST_CASE(test) {
	errno = ENOSYS;

	std::error_code ec = caney::errno_error_code();

	BOOST_CHECK(ec.category() == std::generic_category());
	BOOST_CHECK_EQUAL(ec.value(), ENOSYS);
}

BOOST_AUTO_TEST_SUITE_END()
