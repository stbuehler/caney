#include "caney/std/enum_helper.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(enum_helper)

using namespace caney;

enum class enum1 {
	value0 = 0,
	value255 = 255,
};

enum class enum2 : int8_t {
	valuen128 = -128,
	value0 = 0,
	value127 = 127,
};

enum class enum3 : uint8_t {
	value0 = 0,
	value255 = 255,
};

static_assert(std::is_same<decltype(from_enum(enum2::value0)), int8_t>::value, "from_enum return value matches underlying type");
static_assert(std::is_same<decltype(from_enum(enum3::value0)), uint8_t>::value, "from_enum return value matches underlying type");

BOOST_AUTO_TEST_CASE(test1) {
	BOOST_CHECK_EQUAL(from_enum(enum1::value0), 0);
	BOOST_CHECK_EQUAL(from_enum(enum1::value255), 255);

	BOOST_CHECK_EQUAL(from_enum(to_enum<enum1>(0)), 0);
	BOOST_CHECK_EQUAL(from_enum(to_enum<enum1>(255)), 255);
}

BOOST_AUTO_TEST_CASE(test2) {
	BOOST_CHECK_EQUAL(from_enum(enum2::valuen128), -128);
	BOOST_CHECK_EQUAL(from_enum(enum2::value0), 0);
	BOOST_CHECK_EQUAL(from_enum(enum2::value127), 127);

	BOOST_CHECK_EQUAL(from_enum(to_enum<enum2>(-128)), -128);
	BOOST_CHECK_EQUAL(from_enum(to_enum<enum2>(0)), 0);
	BOOST_CHECK_EQUAL(from_enum(to_enum<enum2>(127)), 127);

	BOOST_CHECK_EQUAL(from_enum(enum2{to_enum(int8_t{-128})}), -128);
	BOOST_CHECK_EQUAL(from_enum(enum2{to_enum(int8_t{0})}), 0);
	BOOST_CHECK_EQUAL(from_enum(enum2{to_enum(int8_t{127})}), 127);
}

BOOST_AUTO_TEST_CASE(test3) {
	BOOST_CHECK_EQUAL(from_enum(enum3::value0), 0);
	BOOST_CHECK_EQUAL(from_enum(enum3::value255), 255);

	BOOST_CHECK_EQUAL(from_enum(to_enum<enum3>(0)), 0);
	BOOST_CHECK_EQUAL(from_enum(to_enum<enum3>(255)), 255);

	BOOST_CHECK_EQUAL(from_enum(enum3{to_enum(uint8_t{0})}), 0);
	BOOST_CHECK_EQUAL(from_enum(enum3{to_enum(uint8_t{255})}), 255);
}

BOOST_AUTO_TEST_SUITE_END()
