#include "caney/std/optional.hpp"

#include <string>

#include <boost/test/unit_test.hpp>

__CANEY_STDV1_BEGIN
/* make sure all functions are "valid", not just the ones we use below: */
template class caney::optional<int>;
template class caney::optional<std::string>;
__CANEY_STDV1_END

static_assert(std::is_trivially_destructible<caney::optional<int>>::value, "");

BOOST_AUTO_TEST_SUITE(optional)

BOOST_AUTO_TEST_CASE(test_optional_int) {
	{
		constexpr caney::optional<int> t;
		constexpr bool empty = !t;
		static_assert(empty, "t must be empty");
		static_assert(t == caney::nullopt, "t must be empty");
	}

	{
		constexpr caney::optional<int> t{1};
		constexpr bool empty = !t;
		static_assert(!empty, "t must not be empty");
		constexpr int value = t.value();
		static_assert(value == 1, "t must contain '1'");
	}

	{
		constexpr caney::optional<int> t = 1;
		constexpr bool empty = !t;
		static_assert(!empty, "t must not be empty");
		constexpr int value = t.value();
		static_assert(value == 1, "t must contain '1'");
	}

	{
		caney::optional<int> t;
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
}

BOOST_AUTO_TEST_CASE(test_optional_string) {
	caney::optional<std::string> t;
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
