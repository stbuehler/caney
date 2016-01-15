#pragma once

#include "caney/memory/buffer.hpp"
#include "caney/util/parse_integral.hpp"

#include "internal.hpp"

#include <cstdint>

__CANEY_BENCODEV1_BEGIN

/**
 * @brief The big_number class represents big integers using an internal decimal representation
 */
class big_number {
public:
	/**
	 * @brief construct a new @ref big_number from a VALID normalized decimal representation.
	 * @pre raw must contain only digits apart from an optional leading '-', and no leading
	 *      zeroes. The zero values must be represented as "0", not "-0" and not ""
	 */
	explicit big_number(memory::shared_const_buf raw) : m_raw(raw) {}

	/**
	 * @brief construct a new @ref big_number from an unsigned integer, converting to a
	 *        decimal representation internally.
	 */
	explicit big_number(std::uintmax_t value);

	/**
	 * @brief construct a new @ref big_number from a signed integer, converting to a
	 *        decimal representation internally.
	 */
	explicit big_number(std::intmax_t value);

	/** @brief return decimal representation */
	memory::shared_const_buf const& raw() const {
		return m_raw;
	}

	/** @brief try parsing decimal representation into some fixed-size integer type */
	template <typename Integral, std::enable_if_t<std::is_integral<Integral>::value>* = nullptr>
	caney::optional<Integral> try_decode() const {
		return util::parse_integral<Integral>(m_raw);
	}

private:
	memory::shared_const_buf m_raw;
};

/**
 * @brief a token represents the type of a token returned by @ref peek_token()
 * but does not contain the actual data.
 */
enum class token {
	Error, //! invalid data
	Integral, //! found integer value
	String, //! found string
	List, //! found list begin; advance buffer by 1 byte to read first list element
	Dict, //! found dict begin; advance buffer by 1 byte to read first dict element key
	ContainerEnd, //! found container (list or dict) end
};

/** @brief determine type of next item in bencoded buffer */
token peek_token(memory::shared_const_buf const& buf);
/**
 * @brief parse integer; return @ref nullopt if any error is encountered.
 * advance @param buf to next item
 */
caney::optional<big_number> parse_integral(memory::shared_const_buf& buf);
/**
 * @brief parse string; return @ref nullopt if any error is encountered.
 * advance @param buf to next item
 */
caney::optional<memory::shared_const_buf> parse_string(memory::shared_const_buf& buf);
/**
 * @brief parse next item (of any type, but make sure it is valid);
 * return @ref nullopt if any error is encountered.
 * advance @param buf to next item.
 * @return @ref nullopt on errors or the memory region defining the item
 */
caney::optional<memory::shared_const_buf> parse_item(memory::shared_const_buf& buf);

/**
 * @brief generic parser template; needs to be defined in the bencoded namespace
 *
 * must define a function ``static caney::optional<Result> parse(memory::shared_const_buf& buf);``
 * which, if successful, returns the parsed value and advances buf to next item
 */
template <typename Result, class Enable = void>
struct parser;

/**
 * @brief parses a value of type @tparam result if @ref parser has an instance for ``parser<Result>``.
 *
 * if successful returns the parsed value and advances buf to next item.
 * if not successful, @ref nullopt is returned and buf is not modified.
 */
template <typename Result>
caney::optional<Result> parse(memory::shared_const_buf& buf) {
	memory::shared_const_buf bufCopy{buf};
	caney::optional<Result> result;
	result = parser<Result>::parse(bufCopy);
	if (result) buf = bufCopy;
	return result;
}

/** @brief @ref parser instance to parse string as @ref memory::shared_const_buf */
template <>
struct parser<memory::shared_const_buf> {
	//! @nowarn
	static caney::optional<memory::shared_const_buf> parse(memory::shared_const_buf& buf) {
		return parse_string(buf);
	}
	//! @endnowarn
};

/** @brief @ref parser instance to parse number as @ref big_number */
template <>
struct parser<big_number> {
	//! @nowarn
	static caney::optional<big_number> parse(memory::shared_const_buf& buf) {
		return parse_integral(buf);
	}
	//! @endnowarn
};

/** @brief @ref parser instance to parse number as some fixed-size @tparam Integral */
template <typename Integral>
struct parser<Integral, std::enable_if_t<std::is_integral<Integral>::value>> {
	//! @nowarn
	static caney::optional<Integral> parse(memory::shared_const_buf& buf) {
		caney::optional<big_number> big = parse_integral(buf);
		if (!big) return caney::nullopt;
		return big->try_decode<Integral>();
	}
	//! @endnowarn
};

/** @brief @ref parser instance to parse a list of values of the same type vector of @tparam Entry */
template <typename Entry>
struct parser<std::vector<Entry>> {
	//! @nowarn
	static caney::optional<std::vector<Entry>> parse(memory::shared_const_buf& buf) {
		if (token::List != peek_token(buf)) return caney::nullopt;
		std::vector<Entry> result;
		buf = buf.shared_slice(1);
		while (token::ContainerEnd != peek_token(buf)) {
			caney::optional<Entry> entry = bencode::parse<Entry>(buf);
			if (!entry) return caney::nullopt;
			result.emplace_back(std::move(*entry));
		}
		buf = buf.shared_slice(1);
		return std::move(result);
	}
	//! @endnowarn
};

__CANEY_BENCODEV1_END
