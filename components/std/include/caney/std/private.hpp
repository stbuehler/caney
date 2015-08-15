#pragma once

namespace caney {
	inline namespace stdv1 {
		// mark constructors that should be protected/private but can't (for
		// various reasons: make_shared, ...) with an argument of type
		// private_tag_t. If anyone calls it explicity it is clear who is to
		// blame.
		class private_tag_t {
		private:
			friend private_tag_t make_private_tag();
			private_tag_t() = default;
		};

		extern private_tag_t private_tag;
	}
} // namespace caney
