#include <caney/util/move_arg.hpp>
#include <caney/util/dispatcher.hpp>

#include <iostream>

#include <functional>

#include <boost/asio.hpp>
#include <boost/test/unit_test.hpp>
#include <boost/test/test_case_template.hpp>
#include <boost/mpl/list.hpp>

class copied_exception : std::exception {
};

class nocopy {
public:
	nocopy() = default;

	nocopy(nocopy const&) {
		throw copied_exception();
	}

	nocopy& operator=(nocopy const&) {
		throw copied_exception();
	}

	nocopy(nocopy&& other) {
		other.has_data = false;
	}

	nocopy& operator=(nocopy&& other) {
		has_data = other.has_data;
		other.has_data = false;
		return *this;
	}

	explicit operator bool() const {
		return has_data;
	}

private:
	bool has_data = true;
};

class moveonly {
public:
	moveonly() = default;

	moveonly(moveonly const&) = delete;
	moveonly& operator=(moveonly const&) = delete;

	moveonly(moveonly&& other) {
		other.has_data = false;
	}

	moveonly& operator=(moveonly&& other) {
		has_data = other.has_data;
		other.has_data = false;
		return *this;
	}

	explicit operator bool() const {
		return has_data;
	}

private:
	bool has_data = true;
};

template<typename Arg>
class test_moving_arg {
public:
	void callback_by_value(Arg arg) {
		++callback_counter;
		BOOST_CHECK_MESSAGE(arg, "content delivered");
	}

	void callback_by_const_ref(Arg const& arg) {
		++callback_counter;
		BOOST_CHECK_MESSAGE(arg, "content delivered");
	}

	void callback_by_rref(Arg&& arg) {
		++callback_counter;
		Arg local(std::move(arg));
		BOOST_CHECK_MESSAGE(local, "content delivered");
		BOOST_CHECK_MESSAGE(!arg, "content cleared");
	}

	template<typename Callback>
	void test_direct_call(Callback&& callback) {
		std::function<void(Arg)> bound_callback = std::bind(callback, this, std::placeholders::_1);

		callback_counter = 0;

		BOOST_MESSAGE("callback call 1");
		bound_callback(Arg());
		BOOST_CHECK_EQUAL(callback_counter, 1);

		BOOST_MESSAGE("callback call 2");
		bound_callback(Arg());
		BOOST_CHECK_EQUAL(callback_counter, 2);
	}

	template<typename Callback>
	void test_strand_wrapped_call(Callback&& callback) {
		boost::asio::io_service service;
		boost::asio::strand strand(service);
		std::function<void(Arg)> bound_callback = std::bind(callback, this, std::placeholders::_1);
		std::function<void(Arg)> wrapped_callback = caney::util::wrap_dispatch(strand, bound_callback);
		std::function<void(caney::util::move_arg<Arg>)> wrapped_callback_movearg = strand.wrap(bound_callback);

		service.post([=,&service]() {
			callback_counter = 0;
			BOOST_MESSAGE("callback call 1");
			wrapped_callback(Arg());
			BOOST_MESSAGE("callback call 2");
			wrapped_callback(Arg());
			BOOST_CHECK_EQUAL(callback_counter, 2);
			BOOST_MESSAGE("callback call 3");
			wrapped_callback_movearg(Arg());
			BOOST_MESSAGE("callback call 4");
			wrapped_callback_movearg(Arg());
			BOOST_CHECK_EQUAL(callback_counter, 4);
			service.stop();
		});

		service.run();
	}

private:
	std::size_t callback_counter = 0;
};

BOOST_AUTO_TEST_SUITE(move_arg)

using test_move_arg_types = boost::mpl::list<nocopy, moveonly>;
BOOST_AUTO_TEST_CASE_TEMPLATE(test_move_arg, T, test_move_arg_types) {
	using Test = test_moving_arg<T>;
	Test t;
	t.test_direct_call(&Test::callback_by_value);
	t.test_direct_call(&Test::callback_by_const_ref);
	t.test_direct_call(&Test::callback_by_rref);

	t.test_strand_wrapped_call(&Test::callback_by_value);
	t.test_strand_wrapped_call(&Test::callback_by_const_ref);
	t.test_strand_wrapped_call(&Test::callback_by_rref);
}

BOOST_AUTO_TEST_SUITE_END()
