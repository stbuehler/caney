#include "caney/memory/intrusive_base.hpp"

#include <boost/test/unit_test.hpp>

static_assert(std::is_same<caney::memory::impl::intrusive_traits_detect<int>::base, void>::value, "base detection not working");

namespace {
	class simple_object : public caney::memory::intrusive_base<simple_object> {
	private:
		std::string m_test;
	};

	using simple_object_traits = caney::memory::impl::intrusive_traits<simple_object>;

	class simple_base_object : public caney::memory::intrusive_base<simple_base_object> {
	public:
		virtual ~simple_base_object() = default;

	private:
		std::string m_test;
	};

	class simple_derived_object : public simple_base_object {
	private:
		std::string m_test_derived;
	};
} // anonymous namespace

BOOST_AUTO_TEST_SUITE(intrusive_base_test)

BOOST_AUTO_TEST_CASE(constructing) {
	boost::intrusive_ptr<simple_object> ptr = caney::memory::make_intrusive<simple_object>();
	ptr = simple_object::create();
	ptr = simple_object::allocate(std::allocator<void>());
	boost::intrusive_ptr<simple_object const> const_ptr = ptr;
}

BOOST_AUTO_TEST_CASE(deriving) {
	boost::intrusive_ptr<simple_derived_object> ptr = caney::memory::make_intrusive<simple_derived_object>();
	boost::intrusive_ptr<simple_base_object> base_ptr = ptr;
	ptr = simple_derived_object::create<simple_derived_object>();
	ptr = simple_derived_object::allocate<simple_derived_object>(std::allocator<void>());
}

BOOST_AUTO_TEST_SUITE_END()
