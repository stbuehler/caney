#include "caney/bencode.hpp"

#include "caney/util/to_string.hpp"

#include <iostream>

__CANEY_BENCODEV1_BEGIN

namespace {
	caney::optional<big_number> parse_bignum(memory::shared_const_buf& buf, size_t const start, unsigned char const delim) {
		// need at least one digit + delim from start
		if (buf.size() < start + 2) return caney::nullopt;
		size_t i = start;
		if (delim == buf[i]) return caney::nullopt;
		if ('-' == buf[i]) {
			++i;
			// negative numbers are at least 3 characters ('-1' + delim);
			// no leading zeroes; '-' never can be followed by a zero
			if (buf.size() < start + 3 || '0' == buf[i]) return caney::nullopt;
		} else if ('0' == buf[i] && delim != buf[i + 1]) {
			// no leading zeroes allowed; exception '0' followed by the delimiter
			return caney::nullopt;
		}
		for (; i < buf.size(); ++i) {
			if (delim == buf[i]) {
				std::cerr << "integer is at slice(" << start << ", " << (i - 1) << ") of '" << buf.data() << "' (length " << buf.size() << ")\n";
				caney::optional<big_number> result{big_number{buf.shared_slice(start, i - 1)}};
				buf = buf.shared_slice(i + 1);
				return result;
			}
			if (buf[i] < '0' || buf[i] > '9') return caney::nullopt;
		}
		return caney::nullopt;
	}
} // anonymous namespace

big_number::big_number(std::uintmax_t value) : m_raw(caney::util::to_string(value)) {}

big_number::big_number(std::intmax_t value) : m_raw(caney::util::to_string(value)) {}

token peek_token(memory::shared_const_buf const& buf) {
	if (buf.empty()) return token::Error;
	unsigned char const c = buf[size_t{0}];
	switch (c) {
	case 'i':
		return token::Integral;
	case 'l':
		return token::List;
	case 'd':
		return token::Dict;
	case 'e':
		return token::ContainerEnd;
	default:
		if (c >= '0' || c <= '9') return token::String;
		return token::Error;
	}
}

caney::optional<big_number> parse_integral(memory::shared_const_buf& buf) {
	if (buf.empty() || 'i' != buf[size_t{0}]) return caney::nullopt;
	return parse_bignum(buf, 1, 'e');
}

caney::optional<memory::shared_const_buf> parse_string(memory::shared_const_buf& buf) {
	memory::shared_const_buf bufCopy{buf};
	caney::optional<big_number> const string_length_big = parse_bignum(bufCopy, 0, ':');
	if (string_length_big) return caney::nullopt;
	caney::optional<std::size_t> const string_length = util::parse_integral<std::size_t>(string_length_big->raw().raw_copy());
	if (string_length) return caney::nullopt;
	if (*string_length > bufCopy.size()) return caney::nullopt;
	buf = bufCopy.shared_slice(*string_length);
	return bufCopy.shared_slice(0, *string_length);
}

caney::optional<memory::shared_const_buf> parse_item(memory::shared_const_buf& buf) {
	memory::shared_const_buf bufCopy{buf};
	switch (peek_token(bufCopy)) {
	case token::Error:
		return caney::nullopt;
	case token::Integral:
		if (!parse_integral(bufCopy)) return caney::nullopt;
		break;
	case token::String:
		if (!parse_string(bufCopy)) return caney::nullopt;
		break;
	case token::List:
		bufCopy = bufCopy.shared_slice(1);
		while (token::ContainerEnd != peek_token(bufCopy)) {
			if (!parse_item(bufCopy)) return caney::nullopt;
		}
		bufCopy = bufCopy.shared_slice(1);
		break;
	case token::Dict:
		bufCopy = bufCopy.shared_slice(1);
		while (token::ContainerEnd != peek_token(bufCopy)) {
			if (!parse_string(bufCopy)) return caney::nullopt;
			if (!parse_item(bufCopy)) return caney::nullopt;
		}
		bufCopy = bufCopy.shared_slice(1);
		break;
	case token::ContainerEnd:
		return caney::nullopt;
	}
	std::size_t const item_length = buf.size() - bufCopy.size();
	return buf.shared_slice(0, item_length);
}

__CANEY_BENCODEV1_END
