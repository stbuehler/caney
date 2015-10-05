#include "caney/std/smart_ptr_traits_boost.hpp"
#include "caney/std/weak_fn.hpp"

#include <iostream>

#include <boost/smart_ptr/make_shared.hpp>

namespace {
	void test_call_std(std::shared_ptr<int> ref, int inc) {
		*ref += inc;
	}

	void test_call_boost(boost::shared_ptr<int> ref, int inc) {
		*ref += inc;
	}

	class foo {
	public:
		explicit foo(int init)
		: m_value(init) {
		}

		void bar1(int inc) {
			m_value += inc;
		}

		void bar2() const {
			std::cout << "bar2\n";
		}

		int m_value;
	};
} // anonymous namespace

void test_weakfn_std() {
	auto l_test = [](std::shared_ptr<int> ref, int inc) { test_call_std(ref, inc); };

	auto pi = std::make_shared<int>(10);
	auto cb = caney::impl::make_shared_fn<std::shared_ptr>(test_call_std);
	cb(pi, 5);
	std::cout << *pi << "\n";

	auto pfoo = std::make_shared<foo>(20);
	caney::impl::make_shared_fn<std::shared_ptr>(&foo::bar1)(pfoo, 7);
	std::cout << pfoo->m_value << "\n";

	auto pconstfoo = std::shared_ptr<foo const>(pfoo);
	caney::impl::make_shared_fn<std::shared_ptr>(&foo::bar2)(pconstfoo);

	using x = decltype(caney::impl::make_shared_fn<std::shared_ptr>(&foo::bar2));
	using y = x::arguments_t;
	y _y;
	(void)_y;

	caney::weak_fn(&test_call_std)(std::weak_ptr<int>{pi}, 5);
	caney::weak_fn(l_test)(std::weak_ptr<int>{pi}, 5);
	caney::weak_fn(&foo::bar1)(std::weak_ptr<foo>{pfoo}, 7);
	caney::weak_fn(&foo::bar2)(std::weak_ptr<foo>{pfoo});
	caney::weak_fn(&foo::bar2)(std::weak_ptr<foo const>{pconstfoo});

	caney::impl::make_shared_fn_from_ptr(&test_call_std, pi);
	caney::weak_fn(&test_call_std, pi)(5);
	caney::weak_fn(&foo::bar1, pfoo)(7);
	caney::weak_fn(&foo::bar2, pconstfoo)();
}

void test_weakfn_boost() {
	auto l_test = [](boost::shared_ptr<int> ref, int inc) { test_call_boost(ref, inc); };

	auto pi = boost::make_shared<int>(10);
	auto cb = caney::impl::make_shared_fn<boost::shared_ptr>(test_call_boost);
	cb(pi, 5);
	std::cout << *pi << "\n";

	auto pfoo = boost::make_shared<foo>(20);
	caney::impl::make_shared_fn<boost::shared_ptr>(&foo::bar1)(pfoo, 7);
	std::cout << pfoo->m_value << "\n";

	auto pconstfoo = boost::shared_ptr<foo const>(pfoo);
	caney::impl::make_shared_fn<boost::shared_ptr>(&foo::bar2)(pconstfoo);

	using x = decltype(caney::impl::make_shared_fn<boost::shared_ptr>(&foo::bar2));
	using y = x::arguments_t;
	y _y;
	(void)_y;

	caney::weak_fn(&test_call_boost)(boost::weak_ptr<int>{pi}, 5);
	caney::weak_fn(l_test)(boost::weak_ptr<int>{pi}, 5);
	caney::weak_fn<boost::shared_ptr>(&foo::bar1)(boost::weak_ptr<foo>{pfoo}, 7);
	caney::weak_fn<boost::shared_ptr>(&foo::bar2)(boost::weak_ptr<foo>{pfoo});
	caney::weak_fn<boost::shared_ptr>(&foo::bar2)(boost::weak_ptr<foo const>{pconstfoo});

	caney::impl::make_shared_fn_from_ptr(&test_call_boost, pi);
	caney::weak_fn(&test_call_boost, pi)(5);
	caney::weak_fn(&foo::bar1, pfoo)(7);
	caney::weak_fn(&foo::bar2, pconstfoo)();
}
