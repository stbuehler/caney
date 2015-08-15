#include "caney/std/private.hpp"

namespace caney {
	inline namespace stdv1 {
		private_tag_t make_private_tag() {
			return private_tag_t();
		}

		private_tag_t private_tag = make_private_tag();
	}
}
