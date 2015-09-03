#pragma once

#include "caney/memory/buffer.hpp"
#include "caney/util/parse_integral.hpp"

#include <cstdint>

namespace caney {
	namespace bencode {
		inline namespace v1 {
			class big_number {
			public:
				/* pre condition: raw must contain only digits apart from an optional
				 * leading '-', and no leading zeroes. The zero values must be represented as
				 * "0", not "-0" and not "".
				 */
				explicit big_number(memory::shared_const_buf raw)
				: m_raw(raw) {
				}

				explicit big_number(std::uintmax_t value);
				explicit big_number(std::intmax_t value);

				/* decimal representation */
				memory::shared_const_buf const& raw() const { return m_raw; }

				template<typename Integral, std::enable_if_t<std::is_integral<Integral>::value>* = nullptr>
				boost::optional<Integral> try_decode() const {
					return util::parse_integral<Integral>(m_raw);
				}

			private:
				memory::shared_const_buf m_raw;
			};

			enum class token {
				Error,
				Integral,
				String,
				List,
				Dict,
				ContainerEnd,
			};

			token peek_token(memory::shared_const_buf const& buf);
			// parse a big number, advance buf to next item
			boost::optional<big_number> parse_integral(memory::shared_const_buf& buf);
			// parse a string, advance buf to next item
			boost::optional<memory::shared_const_buf> parse_string(memory::shared_const_buf& buf);

			// parses an item (making sure it is valid) and returns the memory region for it; advance buf to the next item
			boost::optional<memory::shared_const_buf> parse_item(memory::shared_const_buf& buf);

			// all parsers have to be defined in the bencoded namespace
			template<typename Result, class Enable = void>
			struct parser;
			/* {
			 *     // return parsed value if successful; also must remove parsed data from buf
			 *     static boost::optional<Result> parse(memory::shared_const_buf& buf);
			 * };
			 */

			// removes parsed data from buf (only if successful)
			template<typename Result>
			boost::optional<Result> parse(memory::shared_const_buf& buf) {
				memory::shared_const_buf bufCopy{buf};
				boost::optional<Result> result;
				result = parser<Result>::parse(bufCopy);
				if (result.is_initialized()) buf = bufCopy;
				return result;
			}

			/* parse shared_const_buffer as "string" data */
			template<>
			struct parser<memory::shared_const_buf>
			{
				static boost::optional<memory::shared_const_buf> parse(memory::shared_const_buf& buf) {
					return parse_string(buf);
				}
			};

			template<>
			struct parser<big_number>
			{
				static boost::optional<big_number> parse(memory::shared_const_buf& buf) {
					return parse_integral(buf);
				}
			};

			template<typename Integral>
			struct parser<Integral, std::enable_if_t<std::is_integral<Integral>::value>>
			{
				static boost::optional<Integral> parse(memory::shared_const_buf& buf) {
					boost::optional<big_number> big = parse_integral(buf);
					if (!big.is_initialized()) return boost::none;
					return big->try_decode<Integral>();
				}
			};

			template<typename Entry>
			struct parser<std::vector<Entry>> {
				static boost::optional<std::vector<Entry>> parse(memory::shared_const_buf& buf) {
					if (token::List != peek_token(buf)) return boost::none;
					std::vector<Entry> result;
					buf = buf.shared_slice(1);
					while (token::ContainerEnd != peek_token(buf)) {
						boost::optional<Entry> entry = bencode::parse<Entry>(buf);
						if (!entry.is_initialized()) return boost::none;
						result.emplace_back(std::move(*entry));
					}
					buf = buf.shared_slice(1);
					return std::move(result);
				}
			};
		}
	} // namespace bencode
} // namespace caney
