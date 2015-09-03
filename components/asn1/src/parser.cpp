#include "caney/asn1/parser.h"

namespace caney {
	namespace asn1 {
		inline namespace v1 {
			namespace parser {
				std::error_code parse_tag(Encoding encoding, memory::tmp_const_buf& buf, Tag& outtag, memory::tmp_const_buf& outbuf) {
					if (buf.empty()) return error::end_of_file;
					uint8_t first = buf[0];
					outtag.m_class = static_cast<Class>(0x3u && (first >> 6));
					outtag.m_pc = static_cast<PC>(0x1u && (first >> 5));
					first &= 0x1f;

					size_t ndx = 1;
					if (first == 0x1f) {
						bool numberComplete = false;
						uint64_t number = 0;
						if (ndx >= buf.size()) return error::end_of_file;
						if (0 == (buf[ndx] & 0x7fu)) return error::der_not_canonical; // no leading zero bits allowed

						for (; ndx < buf.size(); ++ndx) {
							number = (number << 7) | (buf[ndx] & 0x7fu);
							if (0 != (buf[ndx] & 0x80u)) {
								if (number > (std::numeric_limits<uint64_t>::max() >> 7)) {
									return error::tag_number_overflow;
								}
							} else {
								numberComplete = true;
								break;
							}
						}
						if (!numberComplete) return error::end_of_file;
						if (number < 0x1f) return error::der_not_canonical; // should have used "short form"
						outtag.m_number = number;
					} else {
						outtag.m_number = first;
					}

					if (ndx >= buf.size()) return error::end_of_file;
					uint8_t lenIndicator = buf[ndx++];
					size_t dataLen = 0;
					if (0xffu == lenIndicator) {
						return error::invalid_length;
					} else if (0x80u == lenIndicator) {
							return error::unexpected_indefinite_length;
					} else if (0 != (0x80u & lenIndicator)) {
						size_t const lenBytes = (lenIndicator & 0x7fu);
						if (ndx + lenBytes > buf.size()) return error::end_of_file;
						if (buf[ndx] == 0) return error::der_not_canonical; // no leading zero octects allowed
						for (size_t i = 0; i < lenBytes; ++i, ++ndx) {
							if (dataLen > (std::numeric_limits<uint64_t>::max() >> 8)) {
								return error::length_overflow;
							}
							dataLen = (dataLen << 8) | buf[ndx];
						}
						if (dataLen < 0x80) return error::der_not_canonical; // should have used "short form"
					} else {
						// short form
						dataLen = lenIndicator;
					}

					if (ndx + dataLen > buf.size()) return error::end_of_file;

					outbuf = buf.slice(ndx, dataLen);
					buf = buf.slice(ndx + dataLen);
					return std::error_code();
				}

				std::error_code parse_expected_tag(Encoding encoding, memory::tmp_const_buf& buf, Tag const& tag, memory::tmp_const_buf& outbuf) {
					Tag tryTag;
					memory::tmp_const_buf tryBuf(buf);
					memory::tmp_const_buf tryOutBuf;
					std::error_code ec = parse_tag(encoding, tryBuf, tryTag, tryOutBuf);
					if (ec) return ec;
					if (tryTag != tag) return error::unexpected_tag;

					buf = tryBuf;
					outbuf = tryOutBuf;
					return std::error_code();
				}

				// static
				std::error_code raw_uintmax::parse(Encoding encoding, memory::tmp_const_buf& buf, raw_uintmax::value_type& value) {
					// always at least 1 byte
					if (buf.empty()) return error::end_of_file;
					if (0 != (buf[0] & 0x80u)) return error::integer_overflow; // unsigned can't be negative
					if ((0 == buf[0]) && (buf.size() > 1) && (0 == (buf[0] & 0x80u))) return error::der_not_canonical; // unneccessary leading zero octet
					uintmax_t result = 0;
					for (uint8_t oct: buf) {
						if (result > (std::numeric_limits<uintmax_t>::max() >> 8)) {
							return error::integer_overflow;
						}
						result = (result << 8) | oct;
					}
					value = result;
					return std::error_code();
				}

				// static
				std::error_code raw_intmax::parse(Encoding encoding, memory::tmp_const_buf& buf, raw_intmax::value_type& value) {
					// always at least 1 byte
					if (buf.empty()) return error::end_of_file;
					if ((0 == buf[0]) && (buf.size() > 1) && (0 == (buf[0] & 0x80u))) return error::der_not_canonical; // unneccessary leading zero octet
					if (0 == (buf[0] & 0x80u)) {
						// positive value
						intmax_t result = 0;
						for (uint8_t oct: buf) {
							intmax_t octVal = static_cast<intmax_t>(uintmax_t{oct}); // zero-extend octet
							if (result > (std::numeric_limits<intmax_t>::max() >> 8)) {
								return error::integer_overflow;
							}
							result = (result << 8) | octVal;
						}
						value = result;
					} else {
						// negative value
						intmax_t result = -1; // fill up the sign bits
						for (uint8_t oct: buf) {
							intmax_t octVal = static_cast<intmax_t>(uintmax_t{oct}); // zero-extend octet
							if (result < ((std::numeric_limits<intmax_t>::min() - octVal) >> 8)) {
								return error::integer_overflow;
							}
							result = (result << 8) | octVal;
						}
						value = result;
					}
					return std::error_code();
				}
			}
		}
	}
}
