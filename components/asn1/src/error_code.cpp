#include "caney/asn1/error_codes.h"

namespace caney {
	namespace asn1 {
		inline namespace v1 {
			error_category& error_category_instance() {
				static error_category instance;
				return instance;
			}

			std::error_code make_error_code(error ev) {
				return std::error_code(static_cast<int>(ev), error_category_instance());
			}
		}
	}
}
