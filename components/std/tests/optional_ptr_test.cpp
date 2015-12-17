#include "caney/std/optional_ptr.hpp"

#include <string>

#include <boost/test/unit_test.hpp>

__CANEY_STDV1_BEGIN
/* make sure all functions are "valid", not just the ones we use below: */
template class caney::optional_ptr<int>;
template class caney::optional_ptr<std::string>;
__CANEY_STDV1_END

BOOST_AUTO_TEST_SUITE(optional_ptr)

BOOST_AUTO_TEST_CASE(test_optional_ptr_int) {
	caney::optional_ptr<int> t;
	BOOST_CHECK(!t);
	t = 1;
	BOOST_CHECK_EQUAL(t.value(), 1);
	t = caney::nullopt;
	BOOST_CHECK(!t);
	t = 2;
	BOOST_CHECK_EQUAL(t.value(), 2);
	t = caney::nullopt;
	BOOST_CHECK(!t);
}

BOOST_AUTO_TEST_CASE(test_optional_ptr_string) {
	caney::optional_ptr<std::string> t;
	BOOST_CHECK(!t);
	t = std::string("abc");
	BOOST_CHECK_EQUAL(t.value(), "abc");
	BOOST_CHECK_EQUAL(*t, "abc");
	BOOST_CHECK_EQUAL(t->size(), 3);
	t = caney::nullopt;
	BOOST_CHECK(!t);
	t = std::string("xyz");
	BOOST_CHECK_EQUAL(t.value(), "xyz");
	t = caney::nullopt;
	BOOST_CHECK(!t);
}

BOOST_AUTO_TEST_SUITE_END()
