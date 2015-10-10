#pragma once

#include "internal.hpp"

#include <string>
#include <type_traits>
#include <vector>

__CANEY_MEMORYV1_BEGIN

namespace impl {
	template<typename Storage>
	struct buffer_storage;

	template<typename Char, typename Traits, typename Allocator>
	struct string_buffer_storage {
		typedef std::basic_string<Char, Traits, Allocator> container_t;

		static unsigned char const* data(container_t const& c) {
			return reinterpret_cast<unsigned char const*>(c.data());
		}

		static unsigned char* data(container_t& c) {
			return reinterpret_cast<unsigned char*>(&c[0]);
		}

		static std::size_t size(container_t const& c) {
			return c.size();
		}
	};

	template<typename Traits, typename Allocator>
	struct buffer_storage<std::basic_string<char, Traits, Allocator>> : string_buffer_storage<char, Traits, Allocator> {
	};

	template<typename Traits, typename Allocator>
	struct buffer_storage<std::basic_string<unsigned char, Traits, Allocator>> : string_buffer_storage<unsigned char, Traits, Allocator> {
	};

	template<typename Char, typename Allocator>
	struct vector_buffer_storage {
		typedef std::vector<Char, Allocator> container_t;

		static unsigned char const* data(container_t const& c) {
			return reinterpret_cast<unsigned char const*>(c.data());
		}

		static unsigned char* data(container_t& c) {
			return reinterpret_cast<unsigned char*>(c.data());
		}

		static std::size_t size(container_t const& c) {
			return c.size();
		}
	};

	template<typename Allocator>
	struct buffer_storage<std::vector<char, Allocator>> : vector_buffer_storage<char, Allocator> {
	};

	template<typename Allocator>
	struct buffer_storage<std::vector<unsigned char, Allocator>> : vector_buffer_storage<unsigned char, Allocator> {
	};
} // namespace impl

__CANEY_MEMORYV1_END
