#pragma once

#include "error_codes.h"

#include "caney/memory/buffer.hpp"

#include <system_error>
#include <limits>

namespace caney {
	namespace asn1 {
		inline namespace v1 {
			enum class Encoding {
				DER,
			};

			enum class Class : uint8_t {
				Universal,
				Application,
				ContextSpecific,
				Private,
			};

			enum class PC : uint8_t {
				Primitive,
				Constructed,
			};

			enum class UniversalPrimitiveTag : uint8_t {
				EndOfContent     = 0x00,
				Boolean          = 0x01,
				Integer          = 0x02,
				BitString        = 0x03,
				OctetString      = 0x04,
				Null             = 0x05,
				ObjectIdentifier = 0x06,
				ObjectDescriptor = 0x07,
				Real             = 0x09,
				Enumerated       = 0x0a,
				Utf8String       = 0x0c,
				RelativeOID      = 0x0d,
				NumericString    = 0x12,
				PrintableString  = 0x13,
				T61String        = 0x14,
				VideotexString   = 0x15,
				IA5String        = 0x16,
				UTCTime          = 0x17,
				GeneralizedTime  = 0x18,
				GraphicString    = 0x19,
				VisibleString    = 0x1a,
				GeneralString    = 0x1b,
				UniversalString  = 0x1c,
				CharacterString  = 0x1d,
				BmpString        = 0x1e,
			};

			enum class UniversalConstructedTag : uint8_t {
				BitString        = 0x03,
				OctetString      = 0x04,
				ObjectDescriptor = 0x07,
				External         = 0x08,
				EmbeddedPDV      = 0x0b,
				Utf8String       = 0x0c,
				Sequence         = 0x10,
				Set              = 0x11,
				NumericString    = 0x12,
				PrintableString  = 0x13,
				T61String        = 0x14,
				VideotexString   = 0x15,
				IA5String        = 0x16,
				UTCTime          = 0x17,
				GeneralizedTime  = 0x18,
				GraphicString    = 0x19,
				VisibleString    = 0x1a,
				GeneralString    = 0x1b,
				UniversalString  = 0x1c,
				CharacterString  = 0x1d,
				BmpString        = 0x1e,
			};

			struct Tag {
				Class m_class{Class::Universal};
				PC m_pc{PC::Primitive};
				uint64_t m_number{0};

				constexpr explicit Tag() = default;

				constexpr explicit Tag(Class clas, PC pc, uint64_t number)
					: m_class(clas), m_pc(pc), m_number(number) {
				}

				constexpr explicit Tag(UniversalPrimitiveTag ptag)
					: m_class(Class::Universal), m_pc(PC::Primitive), m_number(static_cast<uint64_t>(ptag)) {
				}

				constexpr explicit Tag(UniversalConstructedTag ctag)
					: m_class(Class::Universal), m_pc(PC::Constructed), m_number(static_cast<uint64_t>(ctag)) {
				}

				static constexpr Tag context(uint64_t number) {
					return Tag(Class::ContextSpecific, PC::Constructed, number);
				}

				friend bool operator==(Tag const& a, Tag const& b) {
					return a.m_class == b.m_class && a.m_pc == b.m_pc && a.m_number == b.m_number;
				}
				friend bool operator!=(Tag const& a, Tag const& b) {
					return !(a == b);
				}
			};

			template<Class TagClass, PC TagPC, uint64_t TagNumber>
			struct tag_marker {
				static constexpr Tag tag{TagClass, TagPC, TagNumber};
			};
			template<UniversalPrimitiveTag TagNumber>
			struct tag_primitive_marker {
				static constexpr Tag tag{Class::Universal, PC::Primitive, static_cast<uint64_t>(TagNumber)};
			};
			template<UniversalConstructedTag TagNumber>
			struct tag_constructed_marker {
				static constexpr Tag tag{Class::Universal, PC::Constructed, static_cast<uint64_t>(TagNumber)};
			};
			template<uint64_t TagNumber>
			struct tag_context_marker {
				static constexpr Tag tag{Class::ContextSpecific, PC::Constructed, TagNumber};
			};

			namespace parser {
				std::error_code parse_tag(Encoding encoding, memory::tmp_const_buf& buf, Tag& outtag, memory::tmp_const_buf& outbuf);
				std::error_code parse_expected_tag(Encoding encoding, memory::tmp_const_buf& buf, Tag const& tag, memory::tmp_const_buf& outbuf);

				template<typename InnerParser, Class TagClass, uint64_t TagNumber>
				struct tagged_implicit {
					using inner_type = typename InnerParser::inner_type;
					using value_type = typename inner_type::value_type;
					static constexpr PC pc{inner_type::pc};

					static std::error_code parse(Encoding encoding, memory::tmp_const_buf& buf, value_type& value) {
						memory::tmp_const_buf innerBuf;
						if (auto ec = parse_expected_tag(encoding, buf, Tag(TagClass, pc, TagNumber), innerBuf)) return ec;
						if (auto ec = inner_type::parse(encoding, innerBuf, value)) return ec;
						if (!innerBuf.empty()) return error::unused_content;
						return std::error_code();
					}
				};

				template<typename InnerParser, typename TagMarker>
				struct tagged_explicit {
					using value_type = typename InnerParser::value_type;
					using inner_type = InnerParser;
					static constexpr PC pc{PC::Constructed};

					static std::error_code parse(Encoding encoding, memory::tmp_const_buf& buf, value_type& value) {
						memory::tmp_const_buf innerBuf;
						if (auto ec = parse_expected_tag(encoding, buf, TagMarker::tag, innerBuf)) return ec;
						if (auto ec = InnerParser::parse(encoding, innerBuf, value)) return ec;
						if (!innerBuf.empty()) return error::unused_content;
						return std::error_code();
					}
				};

				struct raw_uintmax {
					using value_type = std::uintmax_t;
					static constexpr PC pc{PC::Primitive};
					static std::error_code parse(Encoding encoding, memory::tmp_const_buf& buf, value_type& value);
				};

				struct raw_intmax {
					using value_type = std::intmax_t;
					static constexpr PC pc{PC::Primitive};
					static std::error_code parse(Encoding encoding, memory::tmp_const_buf& buf, value_type& value);
				};

				template<typename Int, class Enable = void>
				struct raw_integer;

				template<typename Int>
				struct raw_integer<Int, std::enable_if_t<std::is_integral<Int>::value && !std::is_signed<Int>::value>> {
					using value_type = Int;
					static constexpr PC pc{PC::Primitive};
					static std::error_code parse(Encoding encoding, memory::tmp_const_buf& buf, value_type& value) {
						uintmax_t innerValue;
						std::error_code ec = raw_uintmax::parse(encoding, buf, innerValue);
						if (ec) return ec;
						if (innerValue > std::numeric_limits<value_type>::max()) {
							ec = std::error_code(error::integer_overflow);
						} else {
							value = static_cast<value_type>(innerValue);
						}
						return ec;
					}
				};

				template<>
				struct raw_integer<uintmax_t, void> : raw_uintmax {
				};

				template<typename Int>
				struct raw_integer<Int, std::enable_if_t<std::is_integral<Int>::value && std::is_signed<Int>::value>> {
					using value_type = Int;
					static constexpr PC pc{PC::Primitive};
					static std::error_code parse(Encoding encoding, memory::tmp_const_buf& buf, value_type& value) {
						uintmax_t innerValue;
						std::error_code ec = raw_uintmax::parse(encoding, buf, innerValue);
						if (ec) return ec;
						if (innerValue > std::numeric_limits<value_type>::max() || innerValue < std::numeric_limits<value_type>::min()) {
							ec = std::error_code(error::integer_overflow);
						} else {
							value = static_cast<value_type>(innerValue);
						}
						return ec;
					}
				};

				template<>
				struct raw_integer<intmax_t, void> : raw_intmax {
				};

				// template<typename Int, std::enable_if_t<std::is_integral<Int>::value>* = nullptr>
				// struct Xraw_integer : std::conditional<std::is_signed<Int>::value, raw_int<Int>, raw_uint<Int>>::type {
				// };

				template<typename Int, std::enable_if_t<std::is_integral<Int>::value>* = nullptr>
				using integer = tagged_explicit<raw_integer<Int>, tag_primitive_marker<UniversalPrimitiveTag::Integer>>;

				struct null_value {
				};

				struct raw_null {
					using value_type = null_value;
					static constexpr PC pc{PC::Primitive};
					static std::error_code parse(Encoding encoding, memory::tmp_const_buf& buf, value_type& value) {
						if (!buf.empty()) return error::unused_content;
						return std::error_code();
					}
				};

				using null = tagged_explicit<raw_null, tag_primitive_marker<UniversalPrimitiveTag::Null>>;

				template<typename ElementParser, typename Container = std::vector<typename ElementParser::value_type>>
				struct raw_list {
					using value_type = Container;
					static constexpr PC pc{PC::Constructed};

					static std::error_code parse(Encoding encoding, memory::tmp_const_buf& buf, value_type& value) {
						while (!buf.empty()) {
							typename Container::value_type elem;
							if (auto ec = ElementParser::parse(encoding, buf, elem)) return ec;
							value.push_back(std::move(elem));
						}
						return std::error_code();
					}
				};

				template<typename ElementParser, typename Container = std::vector<typename ElementParser::value_type>>
				using list = tagged_explicit<raw_list<ElementParser, Container>, tag_constructed_marker<UniversalConstructedTag::Sequence>>;

				struct raw_inner {
					using value_type = memory::shared_const_buf;
					static std::error_code parse(Encoding encoding, memory::tmp_const_buf& buf, value_type& value) {
						value = buf.shared_copy();
						return std::error_code();
					}
				};

				template<typename TagMarker>
				using inner = tagged_explicit<raw_inner, TagMarker>;

				struct raw_string {
					using value_type = std::string;
					static constexpr PC pc{PC::Primitive};

					static std::error_code parse(Encoding encoding, memory::tmp_const_buf& buf, value_type& value) {
						value = std::string(buf.char_begin(), buf.size());
						return std::error_code();
					}
				};

				template<UniversalPrimitiveTag TagNumber>
				using tagged_primitive_string = tagged_explicit<raw_string, tag_primitive_marker<TagNumber>>;
				using graphic_string = tagged_primitive_string<UniversalPrimitiveTag::GraphicString>;
				using visible_string = tagged_primitive_string<UniversalPrimitiveTag::VisibleString>;
				using general_string = tagged_primitive_string<UniversalPrimitiveTag::GeneralString>;
				using universal_string = tagged_primitive_string<UniversalPrimitiveTag::UniversalString>;
				using character_string = tagged_primitive_string<UniversalPrimitiveTag::CharacterString>;
			}
		}
	} // namespace memory
} // namespace caney
