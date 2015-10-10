#include "caney/memory/fixed_intrusive_ptr.hpp"

#include <boost/test/unit_test.hpp>

namespace {
	class simple_object : public caney::memory::fixed_intrusive_ctr<simple_object> {
	private:
		std::string m_test;
	};

	class simple_base_object : public caney::memory::fixed_intrusive_ctr<simple_base_object> {
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

BOOST_AUTO_TEST_SUITE(fixed_intrusive_ptr)

BOOST_AUTO_TEST_CASE(constructing) {
	{
		simple_object::pointer ptr = caney::memory::make_fixed_intrusive<simple_object>();
		caney::memory::fixed_intrusive_ptr<simple_object const> constptr = ptr;
	}

	{
		simple_object::pointer ptr = simple_object::pointer::create();
		ptr = ptr.create();
	}
}

BOOST_AUTO_TEST_CASE(deriving) {
	caney::memory::fixed_intrusive_ptr<simple_derived_object> ptr;
	ptr = ptr.create();

	simple_base_object::pointer base_ptr = ptr;
	caney::memory::fixed_intrusive_ptr<simple_base_object const> constptr = ptr;
}

BOOST_AUTO_TEST_SUITE_END()
