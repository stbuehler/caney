#pragma once

#include <array>
#include <cstdint>
#include <limits>
#include <climits>

/* Usage:
 *
 * enum class my_flag : size_t {
 *   foo0 = 0,
 *   foo1 = 1,
 *   // ...
 *   foo10 = 10,
 *   Last = foo10,
 * };
 *
 * using my_flags = caney::flags<my_flag>;
 * CANEY_FLAGS(my_flags)
 *
 * The base enum defines the indices to look up in a bitset; you should use an enum class so there are no predefined (and wrong) bit-wise operators.
 *
 * Bit operations |, & and ^ usually work on the complete bitset; only &, when one parameter is a single flag, will simple test whether the flag is set.
 */

#define CANEY_FLAGS(Flags) \
	Flags operator|(Flags::flag_t a, Flags::flag_t b) { return Flags(a) | b; } \
	Flags operator|(Flags::flag_t a, Flags b) { return Flags(a) | b; } \
	bool operator&(Flags::flag_t a, Flags::flag_t b) = delete; \
	bool operator&(Flags::flag_t a, Flags b) { return b & a; } \
	Flags operator^(Flags::flag_t a, Flags::flag_t b) { return Flags(a) ^ b; } \
	Flags operator^(Flags::flag_t a, Flags b) { return Flags(a) ^ b; } \
	Flags operator~(Flags::flag_t a) { return ~Flags(a); }

namespace caney {
	inline namespace stdv1 {
		namespace impl_traits {
			// idea from http://stackoverflow.com/a/10724828/1478356
			template<typename Enum, bool B = std::is_enum<Enum>::value>
			struct is_scoped_enum : std::false_type {
			};

			template<typename Enum>
			struct is_scoped_enum<Enum, true>
				: std::integral_constant<
					bool,
					!std::is_convertible<Enum, std::underlying_type_t<Enum>>::value> {
			};
		}

		template<typename FlagEnum, std::size_t Size = static_cast<std::size_t>(FlagEnum::Last) + 1, typename Elem = std::uint32_t>
		class flags {
		private:
			typedef std::underlying_type_t<FlagEnum> enum_t;
		public:
			static_assert(impl_traits::is_scoped_enum<FlagEnum>::value, "Only scoped enums allowed");
			static_assert(!std::numeric_limits<enum_t>::is_signed, "Only enums with unsigned underlying types allowed");
			static_assert(std::numeric_limits<Elem>::is_integer, "Only unsigned integers allowed as underlying array elements for flags");
			static_assert(!std::numeric_limits<Elem>::is_signed, "Only unsigned integers allowed as underlying array elements for flags");

			static_assert(8 == CHAR_BIT, "byte should have exactly 8 bits");
			static constexpr std::size_t BITS_PER_ELEM = sizeof(Elem)*CHAR_BIT;

			// how many array entries are needed to store all bits
			static constexpr std::size_t ARRAY_SIZE = (Size + BITS_PER_ELEM - 1)/BITS_PER_ELEM;
			static_assert(ARRAY_SIZE > 0, "Invalid array size");

			// usable bits in the last entry
			static constexpr Elem LAST_ENTRY_MASK = (Size % BITS_PER_ELEM == 0) ? ~Elem{0} : ((Elem{1} << (Size % BITS_PER_ELEM)) - Elem{1});

			typedef std::array<Elem, ARRAY_SIZE> array_t;
			typedef FlagEnum flag_t;

			static constexpr std::size_t size() {
				return Size;
			}

			class reference {
			public:
				reference& operator=(bool value) {
					if (value) {
						set();
					} else {
						reset();
					}
					return *this;
				}

				void set() {
					*m_elem |= m_mask;
				}

				void reset() {
					*m_elem &= ~m_mask;
				}

				void flip() {
					*m_elem ^= m_mask;
				}

				bool test() const {
					return 0 != (*m_elem & m_mask);
				}

				explicit operator bool() const {
					return test();
				}

			private:
				friend class flags<FlagEnum, Size, Elem>;
				explicit reference(Elem& elem, Elem mask)
				: m_elem(&elem), m_mask(mask) {
				}

				Elem* m_elem;
				Elem m_mask;
			};

			explicit flags() = default;
			explicit flags(array_t const& raw)
			: m_array(raw) {
			}
			// implicit
			flags(flag_t flag)
			{
				set(flag);
			}

			reference operator[](flag_t flag) {
				size_t flagNdx{static_cast<enum_t>(flag)};
				Elem const mask = Elem{1} << (flagNdx % BITS_PER_ELEM);
				return reference(m_array[flagNdx / BITS_PER_ELEM], mask);
			}

			constexpr bool operator[](flag_t flag) const {
				size_t flagNdx{static_cast<enum_t>(flag)};
				Elem const mask = Elem{1} << (flagNdx % BITS_PER_ELEM);
				return 0 != (m_array[flagNdx / BITS_PER_ELEM] & mask);
			}

			bool operator==(flags const& other) const {
				return m_array == other.m_array;
			}

			bool operator!=(flags const& other) const {
				return m_array != other.m_array;
			}

			void set(flag_t flag) {
				operator[](flag).set();
			}

			void reset(flag_t flag) {
				operator[](flag).reset();
			}

			void flip(flag_t flag) {
				operator[](flag).flip();
			}

			constexpr bool test(flag_t flag) const {
				return operator[](flag);
			}

			void clear() {
				m_array = array_t{};
			}

			bool operator&=(flag_t) = delete;
			constexpr bool operator&(flag_t flag) const {
				return test(flag);
			}

			flags& operator&=(flags const& other) {
				for (size_t i = 0; i < ARRAY_SIZE; ++i) {
					m_array[i] &= other.m_array[i];
				}
				return *this;
			}
			constexpr flags operator&(flags const& other) const { flags tmp(*this); tmp &= other; return tmp; }

			flags& operator|=(flag_t flag) {
				set(flag);
				return *this;
			}

			flags& operator|=(flags const& other) {
				for (size_t i = 0; i < ARRAY_SIZE; ++i) {
					m_array[i] |= other.m_array[i];
				}
				return *this;
			}
			constexpr flags operator|(flags const& other) const { flags tmp(*this); tmp |= other; return tmp; }

			flags& operator^=(flag_t flag) {
				flip(flag);
				return *this;
			}

			flags& operator^=(flags const& other) {
				for (size_t i = 0; i < ARRAY_SIZE; ++i) {
					m_array[i] ^= other.m_array[i];
				}
				return *this;
			}
			constexpr flags operator^(flags const& other) const { flags tmp(*this); tmp ^= other; return tmp; }

			constexpr flags operator~() const {
				flags tmp(*this);
				tmp.flip_all();
				return tmp;
			}

			void flip_all() {
				for (size_t i = 0; i < ARRAY_SIZE - 1; ++i) {
					m_array[i] = ~m_array[i];
				}
				m_array[ARRAY_SIZE-1] ^= LAST_ENTRY_MASK;
			}

			constexpr bool none() const {
				return m_array == array_t{};
			}

			constexpr bool any() const {
				return !none();
			}

			constexpr bool all() const {
				for (size_t i = 0; i < ARRAY_SIZE - 1; ++i) {
					if (0 != ~m_array[i]) return false;
				}
				return m_array[ARRAY_SIZE-1] == LAST_ENTRY_MASK;
			}

			array_t& underlying_array() {
				return m_array;
			}

			array_t const& underlying_array() const {
				return m_array;
			}

		private:
			array_t m_array{};
		};
	}
} // namespace caney
