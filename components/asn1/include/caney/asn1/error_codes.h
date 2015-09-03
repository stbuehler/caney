#pragma once

#include <system_error>

namespace caney {
	namespace asn1 {
		inline namespace v1 {
			enum class error {
				end_of_file,
				unused_content,
				unexpected_tag,
				tag_number_overflow,
				length_overflow,
				integer_overflow,
				der_not_canonical,
				invalid_length,
				unexpected_indefinite_length,
			};

			class error_category : public std::error_category {
			public:
				virtual const char* name() const noexcept override;
				virtual std::string message(int ev) const override;
			};
			error_category& error_category_instance();

			std::error_code make_error_code(error ev);
		}
	}
}

namespace std {
	template<> struct is_error_code_enum<caney::asn1::error> : public true_type {
	};
}
