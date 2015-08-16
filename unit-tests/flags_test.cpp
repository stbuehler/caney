#include "caney/std/flags.hpp"

#include <boost/test/unit_test.hpp>

enum class flag1 : size_t {
	bit0,
	bit1,
	bit2,
	bit3,
	bit4,
	bit5,
	bit31 = 31,
	bit32,
	bit33,
	bit34,
	bit35,
	bit36,
	bit37,
	bit38,
	bit63 = 63,
	Last = bit63,
};
using flags1 = caney::flags<flag1>;
CANEY_FLAGS(flags1)

static_assert(32 == flags1::BITS_PER_ELEM, "uint32_t must have 32 bits");
static_assert(2 == flags1::ARRAY_SIZE, "64 bits must fit into 2 uint32_t entries");
static_assert(~uint32_t{0} == flags1::LAST_ENTRY_MASK, "for 64 bits the last entry must use all bits");

enum class flag2 : size_t {
	bit0,
	bit1,
	bit2,
	bit3,
	bit4,
	bit5,
	bit31 = 31,
	bit32,
	bit33,
	bit34,
	bit35,
	bit36,
	bit37,
	bit38,
	bit63 = 63,
	bit64 = 64,
	Last = bit64,
};
using flags2 = caney::flags<flag2>;
CANEY_FLAGS(flags2)

static_assert(32 == flags2::BITS_PER_ELEM, "uint32_t must have 32 bits");
static_assert(3 == flags2::ARRAY_SIZE, "65 bits must use 3 uint32_t entries");
static_assert(uint32_t{1} == flags2::LAST_ENTRY_MASK, "for 65 bits the last entry must use only the first bit");

#define FLAGS1_ARRAY(a0, a1) flags1(flags1::array_t{{a0, a1}})
#define FLAGS2_ARRAY(a0, a1, a2) flags2(flags2::array_t{{a0, a1, a2}})

BOOST_AUTO_TEST_SUITE(flags_test)

BOOST_AUTO_TEST_CASE(test_construction) {
	{
		flags1 f{flag1::bit0 | flag1::bit1 | flag1::bit2 | flag1::bit4 | flag1::bit33 | flag1::bit35 | flag1::bit37};
		BOOST_CHECK_EQUAL(f.underlying_array()[0], 23);
		BOOST_CHECK_EQUAL(f.underlying_array()[1], 42);
	}

	{
		flags1 f{FLAGS1_ARRAY(23, 42)};
		BOOST_CHECK_EQUAL(f.underlying_array()[0], 23);
		BOOST_CHECK_EQUAL(f.underlying_array()[1], 42);
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));
		BOOST_CHECK(f != FLAGS1_ARRAY(22, 42));
		BOOST_CHECK(f != FLAGS1_ARRAY(23, 43));
		BOOST_CHECK(f == (flag1::bit0 | flag1::bit1 | flag1::bit2 | flag1::bit4 | flag1::bit33 | flag1::bit35 | flag1::bit37));
	}

	{
		flags2 f{flag2::bit0 | flag2::bit1 | flag2::bit2 | flag2::bit4 | flag2::bit33 | flag2::bit35 | flag2::bit37 | flag2::bit64};
		BOOST_CHECK_EQUAL(f.underlying_array()[0], 23);
		BOOST_CHECK_EQUAL(f.underlying_array()[1], 42);
		BOOST_CHECK_EQUAL(f.underlying_array()[2], 1);
	}

	{
		flags2 f{FLAGS2_ARRAY(23, 42, 1)};
		BOOST_CHECK_EQUAL(f.underlying_array()[0], 23);
		BOOST_CHECK_EQUAL(f.underlying_array()[1], 42);
		BOOST_CHECK_EQUAL(f.underlying_array()[2], 1);
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
		BOOST_CHECK(f != FLAGS2_ARRAY(22, 42, 1));
		BOOST_CHECK(f != FLAGS2_ARRAY(23, 43, 1));
		BOOST_CHECK(f != FLAGS2_ARRAY(23, 42, 0));
		BOOST_CHECK(f == (flag2::bit0 | flag2::bit1 | flag2::bit2 | flag2::bit4 | flag2::bit33 | flag2::bit35 | flag2::bit37 | flag2::bit64));
	}
}

BOOST_AUTO_TEST_CASE(test_isset) {
	{
		flags1 f{FLAGS1_ARRAY(23, 42)};
		BOOST_CHECK(f & flag1::bit0);
		BOOST_CHECK(f & flag1::bit1);
		BOOST_CHECK(f & flag1::bit2);
		BOOST_CHECK(f & flag1::bit4);
		BOOST_CHECK(f & flag1::bit33);
		BOOST_CHECK(f & flag1::bit35);
		BOOST_CHECK(f & flag1::bit37);

		BOOST_CHECK(!(f & flag1::bit3));
		BOOST_CHECK(!(f & flag1::bit5));
		BOOST_CHECK(!(f & flag1::bit31));
		BOOST_CHECK(!(f & flag1::bit32));
		BOOST_CHECK(!(f & flag1::bit34));
		BOOST_CHECK(!(f & flag1::bit36));
		BOOST_CHECK(!(f & flag1::bit38));
		BOOST_CHECK(!(f & flag1::bit63));

		BOOST_CHECK(flag1::bit0 & f);
		BOOST_CHECK(flag1::bit1 & f);
		BOOST_CHECK(flag1::bit2 & f);
		BOOST_CHECK(flag1::bit4 & f);
		BOOST_CHECK(flag1::bit33 & f);
		BOOST_CHECK(flag1::bit35 & f);
		BOOST_CHECK(flag1::bit37 & f);

		BOOST_CHECK(!(flag1::bit3 & f));
		BOOST_CHECK(!(flag1::bit5 & f));
		BOOST_CHECK(!(flag1::bit31 & f));
		BOOST_CHECK(!(flag1::bit32 & f));
		BOOST_CHECK(!(flag1::bit34 & f));
		BOOST_CHECK(!(flag1::bit36 & f));
		BOOST_CHECK(!(flag1::bit38 & f));
		BOOST_CHECK(!(flag1::bit63 & f));
	}

	{
		flags2 f{FLAGS2_ARRAY(23, 42, 1)};
		BOOST_CHECK(f & flag2::bit0);
		BOOST_CHECK(f & flag2::bit1);
		BOOST_CHECK(f & flag2::bit2);
		BOOST_CHECK(f & flag2::bit4);
		BOOST_CHECK(f & flag2::bit33);
		BOOST_CHECK(f & flag2::bit35);
		BOOST_CHECK(f & flag2::bit37);
		BOOST_CHECK(f & flag2::bit64);

		BOOST_CHECK(!(f & flag2::bit3));
		BOOST_CHECK(!(f & flag2::bit5));
		BOOST_CHECK(!(f & flag2::bit31));
		BOOST_CHECK(!(f & flag2::bit32));
		BOOST_CHECK(!(f & flag2::bit34));
		BOOST_CHECK(!(f & flag2::bit36));
		BOOST_CHECK(!(f & flag2::bit38));
		BOOST_CHECK(!(f & flag2::bit63));

		BOOST_CHECK(flag2::bit0 & f);
		BOOST_CHECK(flag2::bit1 & f);
		BOOST_CHECK(flag2::bit2 & f);
		BOOST_CHECK(flag2::bit4 & f);
		BOOST_CHECK(flag2::bit33 & f);
		BOOST_CHECK(flag2::bit35 & f);
		BOOST_CHECK(flag2::bit37 & f);
		BOOST_CHECK(flag2::bit64 & f);

		BOOST_CHECK(!(flag2::bit3 & f));
		BOOST_CHECK(!(flag2::bit5 & f));
		BOOST_CHECK(!(flag2::bit31 & f));
		BOOST_CHECK(!(flag2::bit32 & f));
		BOOST_CHECK(!(flag2::bit34 & f));
		BOOST_CHECK(!(flag2::bit36 & f));
		BOOST_CHECK(!(flag2::bit38 & f));
		BOOST_CHECK(!(flag2::bit63 & f));
	}
}

BOOST_AUTO_TEST_CASE(test_flip) {
	{
		flags1 f{FLAGS1_ARRAY(23, 42)};
		f.flip_all();
		BOOST_CHECK(f == FLAGS1_ARRAY(~23u, ~42u));
		f = ~f;
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));
	}

	{
		flags1 f{FLAGS1_ARRAY(23, 42)};
		f ^= flag1::bit0;
		BOOST_CHECK(!(f & flag1::bit0));
		BOOST_CHECK(f == FLAGS1_ARRAY(22, 42));
		f[flag1::bit0] = !f[flag1::bit0];
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));
	}

	{
		flags1 f{FLAGS1_ARRAY(23, 42)};
		f ^= flag1::bit32;
		BOOST_CHECK(f & flag1::bit32);
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 43));
		f[flag1::bit32] = !f[flag1::bit32];
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));
	}

	{
		flags1 f{FLAGS1_ARRAY(23, 42)};
		f ^= flag1::bit63;
		BOOST_CHECK(f & flag1::bit63);
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42 | 0x80000000u));
		f[flag1::bit63] = !f[flag1::bit63];
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));
	}

	{
		flags2 f{FLAGS2_ARRAY(23, 42, 1)};
		f.flip_all();
		BOOST_CHECK(f == FLAGS2_ARRAY(~23u, ~42u, 0));
		f = ~f;
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
	}

	{
		flags2 f{FLAGS2_ARRAY(23, 42, 1)};
		f ^= flag2::bit0;
		BOOST_CHECK(!(f & flag2::bit0));
		BOOST_CHECK(f == FLAGS2_ARRAY(22, 42, 1));
		f[flag2::bit0] = !f[flag2::bit0];
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
	}

	{
		flags2 f{FLAGS2_ARRAY(23, 42, 1)};
		f ^= flag2::bit32;
		BOOST_CHECK(f & flag2::bit32);
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 43, 1));
		f[flag2::bit32] = !f[flag2::bit32];
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
	}

	{
		flags2 f{FLAGS2_ARRAY(23, 42, 1)};
		f ^= flag2::bit63;
		BOOST_CHECK(f & flag2::bit63);
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42 | 0x80000000u, 1));
		f[flag2::bit63] = !f[flag2::bit63];
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
	}

	{
		flags2 f{FLAGS2_ARRAY(23, 42, 1)};
		f ^= flag2::bit64;
		BOOST_CHECK(!(f & flag2::bit64));
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 0));
		f[flag2::bit64] = !f[flag2::bit64];
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
	}
}

BOOST_AUTO_TEST_CASE(test_set) {
	{
		// these operations shouldn't actually change anything
		flags1 f{FLAGS1_ARRAY(23, 42)};
		f |= flag1::bit0;
		f |= flag1::bit1;
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));
		f |= flag1::bit2;
		f |= flag1::bit4;
		f |= flag1::bit33;
		f |= flag1::bit35;
		f |= flag1::bit37;
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));

		f = f | flag1::bit0;
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));

		f.set(flag1::bit0);
		f.set(flag1::bit1);
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));
		f.set(flag1::bit2);
		f.set(flag1::bit4);
		f.set(flag1::bit33);
		f.set(flag1::bit35);
		f.set(flag1::bit37);
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));

		f &= ~flag1::bit3;
		f &= ~flag1::bit5;
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));
		f &= ~flag1::bit31;
		f &= ~flag1::bit32;
		f &= ~flag1::bit34;
		f &= ~flag1::bit36;
		f &= ~flag1::bit38;
		f &= ~flag1::bit63;
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));

		f = f & ~flag1::bit63;
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));

		f.reset(flag1::bit3);
		f.reset(flag1::bit5);
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));
		f.reset(flag1::bit31);
		f.reset(flag1::bit32);
		f.reset(flag1::bit34);
		f.reset(flag1::bit36);
		f.reset(flag1::bit38);
		f.reset(flag1::bit63);
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));

		f[flag1::bit0] = true;
		f[flag1::bit3] = false;
		f[flag1::bit63] = false;
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));
	}

	{
		flags1 f{FLAGS1_ARRAY(23, 42)};
		f.reset(flag1::bit0);
		BOOST_CHECK(!(f & flag1::bit0));
		BOOST_CHECK(f == FLAGS1_ARRAY(22, 42));
		f[flag1::bit0] = true;
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));
	}

	{
		flags1 f{FLAGS1_ARRAY(23, 42)};
		f.set(flag1::bit32);
		BOOST_CHECK(f & flag1::bit32);
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 43));
		f[flag1::bit32] = false;
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));
	}

	{
		flags1 f{FLAGS1_ARRAY(23, 42)};
		f.set(flag1::bit63);
		BOOST_CHECK(f & flag1::bit63);
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42 | 0x80000000u));
		f[flag1::bit63] = false;
		BOOST_CHECK(f == FLAGS1_ARRAY(23, 42));
	}

	{
		// these operations shouldn't actually change anything
		flags2 f{FLAGS2_ARRAY(23, 42, 1)};
		f |= flag2::bit0;
		f |= flag2::bit1;
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
		f |= flag2::bit2;
		f |= flag2::bit4;
		f |= flag2::bit33;
		f |= flag2::bit35;
		f |= flag2::bit37;
		f |= flag2::bit64;
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));

		f = f | flag2::bit0;
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));

		f.set(flag2::bit0);
		f.set(flag2::bit1);
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
		f.set(flag2::bit2);
		f.set(flag2::bit4);
		f.set(flag2::bit33);
		f.set(flag2::bit35);
		f.set(flag2::bit37);
		f.set(flag2::bit64);
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));

		f &= ~flag2::bit3;
		f &= ~flag2::bit5;
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
		f &= ~flag2::bit31;
		f &= ~flag2::bit32;
		f &= ~flag2::bit34;
		f &= ~flag2::bit36;
		f &= ~flag2::bit38;
		f &= ~flag2::bit63;
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));

		f = f & ~flag2::bit63;
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));

		f.reset(flag2::bit3);
		f.reset(flag2::bit5);
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
		f.reset(flag2::bit31);
		f.reset(flag2::bit32);
		f.reset(flag2::bit34);
		f.reset(flag2::bit36);
		f.reset(flag2::bit38);
		f.reset(flag2::bit63);
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));

		f[flag2::bit0] = true;
		f[flag2::bit3] = false;
		f[flag2::bit63] = false;
		f[flag2::bit64] = true;
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
	}

	{
		flags2 f{FLAGS2_ARRAY(23, 42, 1)};
		f.reset(flag2::bit0);
		BOOST_CHECK(!(f & flag2::bit0));
		BOOST_CHECK(f == FLAGS2_ARRAY(22, 42, 1));
		f[flag2::bit0] = true;
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
	}

	{
		flags2 f{FLAGS2_ARRAY(23, 42, 1)};
		f.set(flag2::bit32);
		BOOST_CHECK(f & flag2::bit32);
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 43, 1));
		f[flag2::bit32] = false;
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
	}

	{
		flags2 f{FLAGS2_ARRAY(23, 42, 1)};
		f.set(flag2::bit63);
		BOOST_CHECK(f & flag2::bit63);
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42 | 0x80000000u, 1));
		f[flag2::bit63] = false;
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
	}

	{
		flags2 f{FLAGS2_ARRAY(23, 42, 1)};
		f.reset(flag2::bit64);
		BOOST_CHECK(!(f & flag2::bit64));
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 0));
		f[flag2::bit64] = true;
		BOOST_CHECK(f == FLAGS2_ARRAY(23, 42, 1));
	}
}

BOOST_AUTO_TEST_CASE(test_clear) {
	{
		flags1 f{FLAGS1_ARRAY(23, 42)};
		BOOST_CHECK(f.any());
		BOOST_CHECK(!f.all());
		BOOST_CHECK(!f.none());
		f.clear();
		BOOST_CHECK(f == FLAGS1_ARRAY(0, 0));
		BOOST_CHECK(!f.any());
		BOOST_CHECK(!f.all());
		BOOST_CHECK(f.none());
		f.flip_all();
		BOOST_CHECK(f == FLAGS1_ARRAY(~0u, ~0u));
		BOOST_CHECK(f.any());
		BOOST_CHECK(f.all());
		BOOST_CHECK(!f.none());
		f.clear();
		BOOST_CHECK(f == FLAGS1_ARRAY(0, 0));
	}

	{
		flags2 f{FLAGS2_ARRAY(23, 42, 1)};
		BOOST_CHECK(f.any());
		BOOST_CHECK(!f.all());
		BOOST_CHECK(!f.none());
		f.clear();
		BOOST_CHECK(f == FLAGS2_ARRAY(0, 0, 0));
		BOOST_CHECK(!f.any());
		BOOST_CHECK(!f.all());
		BOOST_CHECK(f.none());
		f.flip_all();
		BOOST_CHECK(f == FLAGS2_ARRAY(~0u, ~0u, 1));
		BOOST_CHECK(f.any());
		BOOST_CHECK(f.all());
		BOOST_CHECK(!f.none());
		f.clear();
		BOOST_CHECK(f == FLAGS2_ARRAY(0, 0, 0));
	}
}

BOOST_AUTO_TEST_SUITE_END()
