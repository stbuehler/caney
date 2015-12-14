#include "caney/std/bitset.hpp"

#include <boost/test/unit_test.hpp>

template class caney::bitset<128, uint64_t>;

BOOST_AUTO_TEST_SUITE(bitset)

BOOST_AUTO_TEST_CASE(constructing) {
	{
		//constexpr caney::impl::bit_mask<const uint64_t, caney::bitset_endianness::little_endian> m{0};
		// constexpr uint64_t elem{0};
		// constexpr caney::bit_const_reference<decltype(m)> r{elem, m};

		size_t count = 0;
		constexpr caney::bitset<128, const uint64_t> x{};
		//constexpr auto beg = x.begin();
		for (auto b: x) {
			++count;
			BOOST_CHECK_EQUAL(b, false);
		}
		BOOST_CHECK_EQUAL(128, count);
	}

	{
		size_t count = 0;
		caney::bitset<128, uint64_t> x;
		for (auto b: x) {
			++count;
			BOOST_CHECK_EQUAL(b, false);
		}
		BOOST_CHECK_EQUAL(128, count);
	}

	{
		size_t count = 0;
		caney::bitset<128, uint64_t> x;
		x.flip_all();
		for (auto b: x) {
			++count;
			BOOST_CHECK_EQUAL(b, true);
		}
		BOOST_CHECK_EQUAL(128, count);
	}
}

BOOST_AUTO_TEST_SUITE_END()
