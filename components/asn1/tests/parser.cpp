#include "caney/asn1/parser.h"

namespace {
	namespace p = caney::asn1::parser;
	namespace asn1 = caney::asn1;

	struct PA_DATA {
		int32_t padata_type;
		caney::memory::shared_const_buf padata_value;
	};

	template<typename PA_DATA_T = PA_DATA>
	struct parse_PA_DATA {
		using value_type = PA_DATA_T;
		static constexpr asn1::PC pc{asn1::PC::Constructed};

		static std::error_code parse(asn1::Encoding encoding, caney::memory::tmp_const_buf& buf, value_type& value) {
			if (auto ec = p::tagged_explicit<p::integer<int32_t>, asn1::tag_context_marker<1>>::parse(encoding, buf, value.padata_type)) return ec;
			if (auto ec = p::inner<asn1::tag_context_marker<2>>::parse(encoding, buf, value.padata_value)) return ec;
			buf = caney::memory::tmp_const_buf(); // ignore remaining
			return std::error_code();
		}
	};

	struct KDC_REQ {
		uint8_t pvno;
		uint8_t msg_type;
		std::vector<PA_DATA> padata;
	};

	template<typename KDC_REQ>
	struct parse_KDC_REQ {

	};
} // anonymous namespace
