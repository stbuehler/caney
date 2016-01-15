#include "caney/std/enum_array.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(enum_array)

enum class Indices : uint32_t {
	Index0,
	Index1,
	Index2,
	Index3,
	Last = Index3,
};

using Array = caney::enum_array<Indices, uint32_t>;

BOOST_AUTO_TEST_CASE(constructing) {
	{
		Array const a;
		BOOST_CHECK_EQUAL(a[Indices::Index0], 0);
		BOOST_CHECK_EQUAL(a[Indices::Index1], 0);
		BOOST_CHECK_EQUAL(a[Indices::Index2], 0);
		BOOST_CHECK_EQUAL(a[Indices::Index3], 0);

		BOOST_CHECK_EQUAL(a.at(Indices::Index0), 0);
		BOOST_CHECK_EQUAL(a.at(Indices::Index1), 0);
		BOOST_CHECK_EQUAL(a.at(Indices::Index2), 0);
		BOOST_CHECK_EQUAL(a.at(Indices::Index3), 0);
	}

	{
		Array const a(Array::array_type{{23}});
		BOOST_CHECK_EQUAL(a[Indices::Index0], 23);
		BOOST_CHECK_EQUAL(a[Indices::Index1], 0);
		BOOST_CHECK_EQUAL(a[Indices::Index2], 0);
		BOOST_CHECK_EQUAL(a[Indices::Index3], 0);

		BOOST_CHECK_EQUAL(a.at(Indices::Index0), 23);
		BOOST_CHECK_EQUAL(a.at(Indices::Index1), 0);
		BOOST_CHECK_EQUAL(a.at(Indices::Index2), 0);
		BOOST_CHECK_EQUAL(a.at(Indices::Index3), 0);
	}

	{
		Array const a(23u, 42u, 127u, 65537u);
		BOOST_CHECK_EQUAL(a[Indices::Index0], 23);
		BOOST_CHECK_EQUAL(a[Indices::Index1], 42);
		BOOST_CHECK_EQUAL(a[Indices::Index2], 127);
		BOOST_CHECK_EQUAL(a[Indices::Index3], 65537);

		BOOST_CHECK_EQUAL(a[0], 23);
		BOOST_CHECK_EQUAL(a[1], 42);
		BOOST_CHECK_EQUAL(a[2], 127);
		BOOST_CHECK_EQUAL(a[3], 65537);

		BOOST_CHECK_EQUAL(a.get<Indices::Index0>(), 23);
		BOOST_CHECK_EQUAL(a.get<Indices::Index1>(), 42);
		BOOST_CHECK_EQUAL(a.get<Indices::Index2>(), 127);
		BOOST_CHECK_EQUAL(a.get<Indices::Index3>(), 65537);

		BOOST_CHECK_EQUAL(a.at(Indices::Index0), 23);
		BOOST_CHECK_EQUAL(a.at(Indices::Index1), 42);
		BOOST_CHECK_EQUAL(a.at(Indices::Index2), 127);
		BOOST_CHECK_EQUAL(a.at(Indices::Index3), 65537);

		BOOST_CHECK_EQUAL(a.at(0), 23);
		BOOST_CHECK_EQUAL(a.at(1), 42);
		BOOST_CHECK_EQUAL(a.at(2), 127);
		BOOST_CHECK_EQUAL(a.at(3), 65537);
	}
}

BOOST_AUTO_TEST_CASE(iterators) {
	Array a(23u, 42u, 127u, 65537u);
	BOOST_REQUIRE(a.begin() + 4 == a.end());

	BOOST_CHECK_EQUAL(*(a.begin() + 0), 23);
	BOOST_CHECK_EQUAL(*(a.begin() + 1), 42);
	BOOST_CHECK_EQUAL(*(a.begin() + 2), 127);
	BOOST_CHECK_EQUAL(*(a.begin() + 3), 65537);

	BOOST_CHECK_EQUAL(*(a.end() - 4), 23);
	BOOST_CHECK_EQUAL(*(a.end() - 3), 42);
	BOOST_CHECK_EQUAL(*(a.end() - 2), 127);
	BOOST_CHECK_EQUAL(*(a.end() - 1), 65537);

	BOOST_CHECK_EQUAL(*(a.rbegin() + 3), 23);
	BOOST_CHECK_EQUAL(*(a.rbegin() + 2), 42);
	BOOST_CHECK_EQUAL(*(a.rbegin() + 1), 127);
	BOOST_CHECK_EQUAL(*(a.rbegin() + 0), 65537);

	BOOST_CHECK_EQUAL(*(a.rend() - 1), 23);
	BOOST_CHECK_EQUAL(*(a.rend() - 2), 42);
	BOOST_CHECK_EQUAL(*(a.rend() - 3), 127);
	BOOST_CHECK_EQUAL(*(a.rend() - 4), 65537);

	BOOST_CHECK_EQUAL(a.front(), 23);
	BOOST_CHECK_EQUAL(a.back(), 65537);

	auto& x = a.array();
	(void) x;

	auto&& y = std::move(a).array();
	(void) y;
}

BOOST_AUTO_TEST_CASE(const_iterators) {
	Array const a(23u, 42u, 127u, 65537u);
	BOOST_REQUIRE(a.begin() + 4 == a.end());

	BOOST_CHECK_EQUAL(*(a.begin() + 0), 23);
	BOOST_CHECK_EQUAL(*(a.begin() + 1), 42);
	BOOST_CHECK_EQUAL(*(a.begin() + 2), 127);
	BOOST_CHECK_EQUAL(*(a.begin() + 3), 65537);

	BOOST_CHECK_EQUAL(*(a.end() - 4), 23);
	BOOST_CHECK_EQUAL(*(a.end() - 3), 42);
	BOOST_CHECK_EQUAL(*(a.end() - 2), 127);
	BOOST_CHECK_EQUAL(*(a.end() - 1), 65537);

	BOOST_CHECK_EQUAL(*(a.rbegin() + 3), 23);
	BOOST_CHECK_EQUAL(*(a.rbegin() + 2), 42);
	BOOST_CHECK_EQUAL(*(a.rbegin() + 1), 127);
	BOOST_CHECK_EQUAL(*(a.rbegin() + 0), 65537);

	BOOST_CHECK_EQUAL(*(a.rend() - 1), 23);
	BOOST_CHECK_EQUAL(*(a.rend() - 2), 42);
	BOOST_CHECK_EQUAL(*(a.rend() - 3), 127);
	BOOST_CHECK_EQUAL(*(a.rend() - 4), 65537);

	BOOST_CHECK_EQUAL(a.front(), 23);
	BOOST_CHECK_EQUAL(a.back(), 65537);

	auto const& x = a.array();
	(void) x;
}

BOOST_AUTO_TEST_CASE(modifying) {
	Array a{23u, 42u, 127u, 65537u};
	Array orig{a};
	BOOST_REQUIRE(a.begin() + 4 == a.end());
	BOOST_CHECK(a == orig);

	{
		uint32_t tmp = a[Indices::Index0];
		uint32_t counter = 256;

		a[Indices::Index0] = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index0], counter);
		BOOST_CHECK(a != orig);

		a[0] = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index0], counter);
		BOOST_CHECK(a != orig);

		a.at(Indices::Index0) = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index0], counter);
		BOOST_CHECK(a != orig);

		a.at(0) = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index0], counter);
		BOOST_CHECK(a != orig);

		*(a.begin() + 0) = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index0], counter);
		BOOST_CHECK(a != orig);

		*(a.end() - 4) = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index0], counter);
		BOOST_CHECK(a != orig);

		*(a.rend() - 1) = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index0], counter);
		BOOST_CHECK(a != orig);

		*(a.rbegin() + 3) = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index0], counter);
		BOOST_CHECK(a != orig);

		a.front() = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index0], counter);
		BOOST_CHECK(a != orig);

		a.at(0) = tmp;
		BOOST_CHECK(a == orig);
	}

	{
		uint32_t tmp = a[Indices::Index3];
		uint32_t counter = 256;

		a[Indices::Index3] = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index3], counter);
		BOOST_CHECK(a != orig);

		a[3] = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index3], counter);
		BOOST_CHECK(a != orig);

		a.at(Indices::Index3) = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index3], counter);
		BOOST_CHECK(a != orig);

		a.at(3) = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index3], counter);
		BOOST_CHECK(a != orig);

		*(a.begin() + 3) = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index3], counter);
		BOOST_CHECK(a != orig);

		*(a.end() - 1) = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index3], counter);
		BOOST_CHECK(a != orig);

		*(a.rend() - 4) = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index3], counter);
		BOOST_CHECK(a != orig);

		*(a.rbegin() + 0) = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index3], counter);
		BOOST_CHECK(a != orig);

		a.back() = ++counter;
		BOOST_CHECK_EQUAL(a[Indices::Index3], counter);
		BOOST_CHECK(a != orig);

		a.at(3) = tmp;
		BOOST_CHECK(a == orig);
	}
}

BOOST_AUTO_TEST_SUITE_END()
