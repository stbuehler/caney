#include "caney/memory/intrusive_ptr.hpp"

#include <boost/test/unit_test.hpp>

namespace {
	class simple_object : public caney::memory::intrusive_ctr<> {
	private:
		std::string m_test;
	};

	class simple_base_object : public caney::memory::intrusive_ctr<> {
	private:
		std::string m_test;
	};

	class simple_derived_object : public simple_base_object {
	public:
		virtual ~simple_derived_object() = default;

	private:
		std::string m_test_derived;
	};

	class simple_derived_dynamic_object : public simple_derived_object {
	private:
		std::string m_test_derived_dynamic;
	};
} // anonymous namespace

BOOST_AUTO_TEST_SUITE(intrusive_ptr)

BOOST_AUTO_TEST_CASE(constructing) {
	{
		caney::memory::intrusive_ptr<simple_object> ptr = caney::memory::make_intrusive<simple_object>();
		caney::memory::intrusive_ptr<simple_object const> const_ptr = ptr;
	}

	{
		caney::memory::intrusive_ptr<simple_object> ptr = caney::memory::intrusive_ptr<simple_object>::create();
		ptr = ptr.create();
	}
}

BOOST_AUTO_TEST_CASE(deriving) {
	{
		caney::memory::intrusive_ptr<simple_derived_object> ptr;
		ptr = ptr.create();

		caney::memory::intrusive_ptr<simple_base_object> base_ptr = ptr;
		caney::memory::intrusive_ptr<simple_base_object const> const_base_ptr = ptr;

		BOOST_CHECK(caney::memory::static_intrusive_ptr_cast<simple_derived_object>(base_ptr) == ptr);
		BOOST_CHECK(caney::memory::dynamic_intrusive_ptr_cast<simple_derived_dynamic_object>(ptr) == nullptr);
		BOOST_CHECK(caney::memory::const_intrusive_ptr_cast<simple_base_object>(const_base_ptr) == base_ptr);
	}

	{
		caney::memory::intrusive_ptr<simple_derived_dynamic_object> ptr;
		ptr = ptr.create();
		caney::memory::intrusive_ptr<simple_derived_object> base_ptr = ptr;

		BOOST_CHECK(caney::memory::static_intrusive_ptr_cast<simple_derived_dynamic_object>(base_ptr) == ptr);
		BOOST_CHECK(caney::memory::dynamic_intrusive_ptr_cast<simple_derived_dynamic_object>(base_ptr) == ptr);
	}
}

BOOST_AUTO_TEST_SUITE_END()
