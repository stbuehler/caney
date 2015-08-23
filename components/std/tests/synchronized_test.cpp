#include "caney/std/synchronized.hpp"

#include <boost/test/unit_test.hpp>

BOOST_AUTO_TEST_SUITE(synchronized)

BOOST_AUTO_TEST_CASE(synchronized_no_assign) {
	caney::synchronized<uint32_t> sync_value{1};
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 1);

	BOOST_CHECK_EQUAL(sync_value.synchronize().get(), 1);
	BOOST_CHECK_EQUAL(*sync_value.synchronize().operator->(), 1);
	BOOST_CHECK_EQUAL(sync_value.shared_synchronize().get(), 1);
	BOOST_CHECK_EQUAL(*sync_value.shared_synchronize().operator->(), 1);

	sync_value.synchronize().get() = 9;
	sync_value.shared_synchronize([](uint32_t const& value) {
		BOOST_CHECK_EQUAL(value, 9);
	});
	*sync_value.synchronize() = 10;
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 10);

	sync_value.synchronize([](uint32_t& value) {
		BOOST_CHECK_EQUAL(value, 10);
		value = 11;
	});
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 11);
	sync_value.synchronize().get() = 12;
	sync_value.synchronize([](uint32_t const& value) {
		BOOST_CHECK_EQUAL(value, 12);
	});
	*sync_value.synchronize() = 13;
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 13);

	caney::synchronized<uint32_t> copy_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*copy_sync_value.synchronize(), 13);

	caney::shared_synchronized<uint32_t> shared_copy_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*shared_copy_sync_value.synchronize(), 13);

	caney::synchronized<uint64_t> big_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*big_sync_value.synchronize(), 13);

	caney::shared_synchronized<uint64_t> shared_big_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*shared_big_sync_value.synchronize(), 13);
}

BOOST_AUTO_TEST_CASE(synchronized_assign) {
	caney::synchronized<uint32_t, true> sync_value{1};
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 1);

	BOOST_CHECK_EQUAL(sync_value.synchronize().get(), 1);
	BOOST_CHECK_EQUAL(*sync_value.synchronize().operator->(), 1);

	sync_value.synchronize([](uint32_t& value) {
		BOOST_CHECK_EQUAL(value, 1);
		value = 8;
	});
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 8);
	sync_value.synchronize().get() = 9;
	sync_value.synchronize([](uint32_t const& value) {
		BOOST_CHECK_EQUAL(value, 9);
	});
	*sync_value.synchronize() = 10;
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 10);

	sync_value.synchronize([](uint32_t& value) {
		BOOST_CHECK_EQUAL(value, 10);
		value = 11;
	});
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 11);
	sync_value.synchronize().get() = 12;
	sync_value.synchronize([](uint32_t const& value) {
		BOOST_CHECK_EQUAL(value, 12);
	});
	*sync_value.synchronize() = 13;
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 13);

	caney::synchronized<uint32_t> copy_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*copy_sync_value.synchronize(), 13);
	*copy_sync_value.synchronize() = 14;
	sync_value = copy_sync_value;
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 14);

	caney::shared_synchronized<uint32_t> shared_copy_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*shared_copy_sync_value.synchronize(), 14);
	*shared_copy_sync_value.synchronize() = 15;
	sync_value = shared_copy_sync_value;
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 15);

	caney::synchronized<uint64_t, true> big_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*big_sync_value.synchronize(), 15);
	*sync_value.synchronize() = 16;
	big_sync_value = sync_value;
	BOOST_CHECK_EQUAL(*big_sync_value.synchronize(), 16);

	caney::shared_synchronized<uint64_t, true> shared_big_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*shared_big_sync_value.synchronize(), 16);
	*sync_value.synchronize() = 17;
	shared_big_sync_value = sync_value;
	BOOST_CHECK_EQUAL(*shared_big_sync_value.synchronize(), 17);
}

BOOST_AUTO_TEST_CASE(shared_synchronized_no_assign) {
	caney::shared_synchronized<uint32_t> sync_value{1};
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 1);

	BOOST_CHECK_EQUAL(sync_value.synchronize().get(), 1);
	BOOST_CHECK_EQUAL(*sync_value.synchronize().operator->(), 1);
	BOOST_CHECK_EQUAL(sync_value.shared_synchronize().get(), 1);
	BOOST_CHECK_EQUAL(*sync_value.shared_synchronize().operator->(), 1);

	sync_value.synchronize().get() = 9;
	sync_value.shared_synchronize([](uint32_t const& value) {
		BOOST_CHECK_EQUAL(value, 9);
	});
	*sync_value.synchronize() = 10;
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 10);

	sync_value.synchronize([](uint32_t& value) {
		BOOST_CHECK_EQUAL(value, 10);
		value = 11;
	});
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 11);
	sync_value.synchronize().get() = 12;
	sync_value.synchronize([](uint32_t const& value) {
		BOOST_CHECK_EQUAL(value, 12);
	});
	*sync_value.synchronize() = 13;
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 13);

	caney::synchronized<uint32_t> copy_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*copy_sync_value.synchronize(), 13);

	caney::shared_synchronized<uint32_t> shared_copy_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*shared_copy_sync_value.synchronize(), 13);

	caney::synchronized<uint64_t> big_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*big_sync_value.synchronize(), 13);

	caney::shared_synchronized<uint64_t> shared_big_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*shared_big_sync_value.synchronize(), 13);
}

BOOST_AUTO_TEST_CASE(shared_synchronized_assign) {
	caney::shared_synchronized<uint32_t, true> sync_value{1};
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 1);

	BOOST_CHECK_EQUAL(sync_value.synchronize().get(), 1);
	BOOST_CHECK_EQUAL(*sync_value.synchronize().operator->(), 1);

	sync_value.synchronize([](uint32_t& value) {
		BOOST_CHECK_EQUAL(value, 1);
		value = 8;
	});
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 8);
	sync_value.synchronize().get() = 9;
	sync_value.synchronize([](uint32_t const& value) {
		BOOST_CHECK_EQUAL(value, 9);
	});
	*sync_value.synchronize() = 10;
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 10);

	sync_value.synchronize([](uint32_t& value) {
		BOOST_CHECK_EQUAL(value, 10);
		value = 11;
	});
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 11);
	sync_value.synchronize().get() = 12;
	sync_value.synchronize([](uint32_t const& value) {
		BOOST_CHECK_EQUAL(value, 12);
	});
	*sync_value.synchronize() = 13;
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 13);

	caney::synchronized<uint32_t> copy_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*copy_sync_value.synchronize(), 13);
	*copy_sync_value.synchronize() = 14;
	sync_value = copy_sync_value;
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 14);

	caney::shared_synchronized<uint32_t> shared_copy_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*shared_copy_sync_value.synchronize(), 14);
	*shared_copy_sync_value.synchronize() = 15;
	sync_value = shared_copy_sync_value;
	BOOST_CHECK_EQUAL(*sync_value.synchronize(), 15);

	caney::synchronized<uint64_t, true> big_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*big_sync_value.synchronize(), 15);
	*sync_value.synchronize() = 16;
	big_sync_value = sync_value;
	BOOST_CHECK_EQUAL(*big_sync_value.synchronize(), 16);

	caney::shared_synchronized<uint64_t, true> shared_big_sync_value{sync_value};
	BOOST_CHECK_EQUAL(*shared_big_sync_value.synchronize(), 16);
	*sync_value.synchronize() = 17;
	shared_big_sync_value = sync_value;
	BOOST_CHECK_EQUAL(*shared_big_sync_value.synchronize(), 17);
}

BOOST_AUTO_TEST_SUITE_END()
