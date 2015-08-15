#include <caney/bencode.hpp>

#include <boost/test/unit_test.hpp>


BOOST_AUTO_TEST_SUITE(bencode_test)

BOOST_AUTO_TEST_CASE(test_big_number) {
	caney::memory::shared_const_buf source(std::string("i-11498749138591659872394815462934713428e"));
	caney::memory::shared_const_buf buf{source};
	boost::optional<caney::bencode::big_number> i = caney::bencode::parse<caney::bencode::big_number>(buf);
	BOOST_REQUIRE(i.is_initialized());
	BOOST_CHECK(buf.empty());
	BOOST_CHECK_EQUAL(i->raw().data(), source.data() + 1);
	BOOST_CHECK_EQUAL(reinterpret_cast<uintptr_t>(i->raw().data()), reinterpret_cast<uintptr_t>(source.data()) + 1);
	BOOST_CHECK_EQUAL(i->raw().size() + 2, source.size());
}

BOOST_AUTO_TEST_CASE(test_int8_t) {
	caney::memory::shared_const_buf buf(std::string("i-128e"));
	boost::optional<int8_t> i = caney::bencode::parse<int8_t>(buf);
	BOOST_REQUIRE(i.is_initialized());
	BOOST_CHECK(buf.empty());
	BOOST_CHECK_EQUAL(i.get(), -128);
}

BOOST_AUTO_TEST_CASE(test_int_vector) {
	caney::memory::shared_const_buf buf(std::string("li0ei-1ei1ei-128ei127ee"));
	boost::optional<std::vector<int8_t>> l = caney::bencode::parse<std::vector<int8_t>>(buf);
	BOOST_REQUIRE(l.is_initialized());
	BOOST_CHECK(buf.empty());
	BOOST_REQUIRE_EQUAL(l->size(), 5);
	BOOST_CHECK_EQUAL(l->at(0), 0);
	BOOST_CHECK_EQUAL(l->at(1), -1);
	BOOST_CHECK_EQUAL(l->at(2), 1);
	BOOST_CHECK_EQUAL(l->at(3), -128);
	BOOST_CHECK_EQUAL(l->at(4), 127);
}

BOOST_AUTO_TEST_SUITE_END()
