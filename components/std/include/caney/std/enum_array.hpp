/** @file */

#pragma once

#include "compiler_features.hpp"
#include "enum_helper.hpp"
#include "internal.hpp"

#include <array>
#include <limits>
#include <tuple>

__CANEY_STDV1_BEGIN

/**
 * @brief a wrapper for std::array taking enum (and size_t) values as indices
 *
 * Example:
 *    enum class Direction { In, Out, Last = Out };
 *    typedef enum_array<Direction, std::vector<Node*>> NodeEdges;
 *
 * If the enum doesn't have a "Last" member you need to give the size (maximum value + 1) as third template argument manually.
 */
template <typename Enum, typename Value, std::size_t SIZE = static_cast<std::size_t>(Enum::Last) + 1>
class enum_array {
public:
	static_assert(!std::numeric_limits<std::underlying_type_t<Enum>>::is_signed, "only unsigned index enum types allowed");

	/// wrapped array type
	using array_type = std::array<Value, SIZE>;

private:
	// required early to be in scope for noexcept(...)
	array_type m_array;

public:
	/// enum type used as array index
	using enum_type = Enum;

	/**
	  * @{
	  * @brief import type from wrapped array
	  */
	using value_type = typename array_type::value_type;
	using pointer = typename array_type::pointer;
	using const_pointer = typename array_type::const_pointer;
	using reference = typename array_type::reference;
	using const_reference = typename array_type::const_reference;
	using iterator = typename array_type::iterator;
	using const_iterator = typename array_type::const_iterator;
	using size_type = typename array_type::size_type;
	using difference_type = typename array_type::difference_type;
	using reverse_iterator = typename array_type::reverse_iterator;
	using const_reverse_iterator = typename array_type::const_reverse_iterator;
	/** @} */

	/// default construct array elements
	explicit enum_array() : m_array{{}} {}

	/// initialize wrapped array from another array
	explicit enum_array(array_type array) {
		m_array = std::move(array);
	}

	/// default copy constructor
	enum_array(enum_array const&) = default;
	/// default copy assignments
	enum_array& operator=(enum_array const&) = default;
	/// default move constructor
	enum_array(enum_array&&) = default;
	/// default move assignments
	enum_array& operator=(enum_array&&) = default;

	/**
	 * @brief explicit constructor from wrapped array type
	 */
	// explicit construct from mutable reference, otherwise sometimes the perfect-forward constructor wins the overload
	enum_array(enum_array&) = default;

	/**
	  * @brief explicit constructor from anything that can be used to construct wrapped array
	  * @param args arguments used to construct wrapped array
	  */
	// somehow this seems to overload the "mutable reference" copy constructor.
	template <typename... Args>
	explicit enum_array(Args&&... args) : m_array{{std::forward<Args>(args)...}} {
		static_assert(std::tuple_size<std::tuple<Args...>>::value == SIZE, "must initialize all entries");
	}

	/**
	  * @brief expose wrapped array value
	  */
	CANEY_RELAXED_CONSTEXPR array_type& array() & {
		return m_array;
	}

	/**
	  * @brief expose wrapped array value
	  */
	constexpr array_type const& array() const& {
		return m_array;
	}

	/**
	  * @brief expose wrapped array value
	  */
	CANEY_RELAXED_CONSTEXPR array_type&& array() && {
		return std::move(m_array);
	}

	/**
	  * @{
	  * @brief forward `std::array` interface to wrapped array
	  */
	void fill(const value_type& value) {
		m_array.fill(value);
	}
	void swap(enum_array& other) noexcept(noexcept(m_array.swap(other.m_array))) {
		m_array.swap(other.m_array);
	}
	iterator begin() noexcept {
		return m_array.begin();
	}
	const_iterator begin() const noexcept {
		return m_array.begin();
	}
	iterator end() noexcept {
		return m_array.end();
	}
	const_iterator end() const noexcept {
		return m_array.end();
	}
	reverse_iterator rbegin() noexcept {
		return m_array.rbegin();
	}
	const_reverse_iterator rbegin() const noexcept {
		return m_array.rbegin();
	}
	reverse_iterator rend() noexcept {
		return m_array.rend();
	}
	const_reverse_iterator rend() const noexcept {
		return m_array.rend();
	}
	const_iterator cbegin() const noexcept {
		return m_array.cbegin();
	}
	const_iterator cend() const noexcept {
		return m_array.cend();
	}
	const_reverse_iterator crbegin() const noexcept {
		return m_array.crbegin();
	}
	const_reverse_iterator crend() const noexcept {
		return m_array.crend();
	}
	constexpr size_type size() const noexcept {
		return m_array.size();
	}
	constexpr size_type max_size() const noexcept {
		return m_array.max_size();
	}
	constexpr bool empty() const noexcept {
		return size() == 0;
	}
	reference front() noexcept {
		return m_array.front();
	}
	constexpr const_reference front() const noexcept {
		return m_array.front();
	}
	reference back() noexcept {
		return m_array.back();
	}
	constexpr const_reference back() const noexcept {
		return m_array.back();
	}
	pointer data() noexcept {
		return m_array.data();
	}
	const_pointer data() const noexcept {
		return m_array.data();
	}
	/** @} */

	/**
	 * @{
	 * @brief index operation by `size_t` forwarded to wrapped array
	 */
	reference operator[](size_type n) noexcept {
		return m_array[n];
	}
	constexpr const_reference operator[](size_type n) const noexcept {
		return m_array[n];
	}
	reference at(size_type n) {
		return m_array.at(n);
	}
	constexpr const_reference at(size_type n) const {
		return m_array.at(n);
	}
	/** @} */

	/**
	 * @{
	 * @brief index operations by @ref enum_type, casting enum to `size_t` for access to wrapped array
	 */
	reference operator[](enum_type n) noexcept {
		return m_array[size_type{caney::from_enum(n)}];
	}
	constexpr const_reference operator[](enum_type n) const noexcept {
		return m_array[size_type{caney::from_enum(n)}];
	}
	reference at(enum_type n) {
		return m_array.at(size_type{caney::from_enum(n)});
	}
	constexpr const_reference at(enum_type n) const {
		return m_array.at(size_type{caney::from_enum(n)});
	}
	/** @} */

	/**
	 * @brief access element at index known compile time
	 * @tparam Index index
	 */
	template <enum_type Index>
	CANEY_RELAXED_CONSTEXPR value_type& get() & {
		return std::get<size_type{caney::from_enum(Index)}>(m_array);
	}

	/**
	 * @brief access element at index known compile time
	 * @tparam Index index
	 */
	template <enum_type Index>
	constexpr value_type const& get() const& {
		return std::get<size_type{caney::from_enum(Index)}>(m_array);
	}

	/**
	 * @brief access element at index known compile time
	 * @tparam Index index
	 */
	template <enum_type Index>
	CANEY_RELAXED_CONSTEXPR value_type&& get() && {
		return std::get<size_type{caney::from_enum(Index)}>(std::move(m_array));
	}

	/**
	 * @{
	 * @brief comparison operators from wrapped array
	 */
	friend bool operator==(const enum_array& a, const enum_array& b) {
		return a.m_array == b.m_array;
	}
	friend bool operator!=(const enum_array& a, const enum_array& b) {
		return a.m_array != b.m_array;
	}
	friend bool operator<(const enum_array& a, const enum_array& b) {
		return a.m_array < b.m_array;
	}
	friend bool operator<=(const enum_array& a, const enum_array& b) {
		return a.m_array <= b.m_array;
	}
	friend bool operator>(const enum_array& a, const enum_array& b) {
		return a.m_array > b.m_array;
	}
	friend bool operator>=(const enum_array& a, const enum_array& b) {
		return a.m_array >= b.m_array;
	}
	/** @} */

	/**
	 * @brief swap on wrapped array
	 * @param a array to swap
	 * @param b array to swap
	 */
	friend void swap(enum_array& a, enum_array& b) noexcept(noexcept(a.swap(b))) {
		a.swap(b);
	}
};

__CANEY_STDV1_END
