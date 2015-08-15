#pragma once

#include <array>

namespace caney {
	inline namespace stdv1 {
		template<typename Enum, std::size_t SIZE, typename T>
		class enum_array {
		private:
			using array_type = std::array<T, SIZE>;

		public:
			using enum_type = Enum;

			// array element is public
			array_type array;

			// import std::array
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

			template<typename... Args>
			enum_array(Args&&... args)
			: array{{std::forward<Args>(args)...}} {
			}

			// forward almost all std::array
			void fill(const value_type& value) { array.fill(value); }
			void swap(enum_array& other) noexcept(noexcept(array.swap(other.array))) { array.swap(other.array); }
			iterator begin() noexcept { return array.begin(); }
			const_iterator begin() const noexcept { return array.begin(); }
			iterator end() noexcept { return array.end(); }
			const_iterator end() const noexcept { return array.end(); }
			reverse_iterator rbegin() noexcept { return array.rbegin(); }
			const_reverse_iterator rbegin() const noexcept { return array.rbegin(); }
			reverse_iterator rend() noexcept { return array.rend(); }
			const_reverse_iterator rend() const noexcept { return array.rend(); }
			const_iterator cbegin() const noexcept { return array.cbegin(); }
			const_iterator cend() const noexcept { return array.cend(); }
			const_reverse_iterator crbegin() const noexcept { return array.crbegin(); }
			const_reverse_iterator crend() const noexcept { return array.crend(); }
			constexpr size_type size() const noexcept { return array.size(); }
			constexpr size_type max_size() const noexcept { return array.max_size(); }
			constexpr bool empty() const noexcept { return size() == 0; }
			reference front() noexcept { return array.front(); }
			constexpr const_reference front() const noexcept { return array.front(); }
			reference back() noexcept { return array.back(); }
			constexpr const_reference back() const noexcept { return array.back(); }
			pointer data() noexcept { return array.data(); }
			const_pointer data() const noexcept { return array.data(); }

			// offer index operations by std::size_t as std::array
			reference operator[](size_type n) noexcept { return array[n]; }
			constexpr const_reference operator[](size_type n) const noexcept { return array[n]; }
			reference at(size_type n) { return array.at(n); }
			constexpr const_reference at(size_type n) const { return array.at(n); }

			// offer index operations by enum type
			reference operator[](enum_type n) noexcept { return array[size_type(n)]; }
			constexpr const_reference operator[](enum_type n) const noexcept { return array[size_type(n)]; }
			reference at(enum_type n) { return array.at(size_typ(n)); }
			constexpr const_reference at(enum_type n) const { return array.at(size_type(n)); }

			template<enum_type Index>
			constexpr value_type& get() & { return std::get<size_type(Index)>(array); }

			template<enum_type Index>
			constexpr value_type const& get() const& { return std::get<size_type(Index)>(array); }

			template<enum_type Index>
			constexpr value_type&& get() && { return std::get<size_type(Index)>(std::move(array)); }
		};

		template<typename Enum, std::size_t SIZE, typename T>
		inline bool operator==(const enum_array<Enum, SIZE, T>& a, const enum_array<Enum, SIZE, T>& b) { return a.array == b.array; }
		template<typename Enum, std::size_t SIZE, typename T>
		inline bool operator!=(const enum_array<Enum, SIZE, T>& a, const enum_array<Enum, SIZE, T>& b) { return a.array != b.array; }
		template<typename Enum, std::size_t SIZE, typename T>
		inline bool operator< (const enum_array<Enum, SIZE, T>& a, const enum_array<Enum, SIZE, T>& b) { return a.array <  b.array; }
		template<typename Enum, std::size_t SIZE, typename T>
		inline bool operator<=(const enum_array<Enum, SIZE, T>& a, const enum_array<Enum, SIZE, T>& b) { return a.array <= b.array; }
		template<typename Enum, std::size_t SIZE, typename T>
		inline bool operator> (const enum_array<Enum, SIZE, T>& a, const enum_array<Enum, SIZE, T>& b) { return a.array >  b.array; }
		template<typename Enum, std::size_t SIZE, typename T>
		inline bool operator>=(const enum_array<Enum, SIZE, T>& a, const enum_array<Enum, SIZE, T>& b) { return a.array >= b.array; }

		template<typename Enum, std::size_t SIZE, typename T>
		inline void swap(enum_array<Enum, SIZE, T>& a, enum_array<Enum, SIZE, T>& b) noexcept(noexcept(a.swap(b))) { a.swap(b); }
	}
} // namespace caney

namespace std {
	template<typename Enum, std::size_t SIZE, typename T, Enum Index>
	constexpr T& get(caney::enum_array<Enum, SIZE, T>& arr) { return std::get<size_type(Index)>(arr.array); }

	template<typename Enum, std::size_t SIZE, typename T, Enum Index>
	constexpr T const& get(caney::enum_array<Enum, SIZE, T> const& arr) { return std::get<size_type(Index)>(arr.array); }

	template<typename Enum, std::size_t SIZE, typename T, Enum Index>
	constexpr T&& get(caney::enum_array<Enum, SIZE, T>&& arr) { return std::get<size_type(Index)>(std::move(arr.array)); }
} // namespace std
