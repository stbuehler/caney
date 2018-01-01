/** @file */

#pragma once

#include "internal.hpp"

#include <boost/asio/ip/address_v4.hpp>
#include <boost/asio/ip/address_v6.hpp>

#include <stdint.h>

__CANEY_STDV1_BEGIN

/** represents an IPv4 network, i.e. base address + network (netmask length) */
class network_v4 {
public:
	/** underlying address type */
	typedef boost::asio::ip::address_v4 address_type;

	/** clip network length to allowed range */
	static constexpr unsigned char clip_network(unsigned char network) {
		return network >= 32 ? 32 : network;
	}

	/** calculate mask of bits which can be changed without leaving the network */
	static constexpr uint32_t native_hostmask(unsigned char network) {
		return static_cast<uint32_t>((uint64_t{1} << (32u - clip_network(network))) - 1u);
	}

	/** calculate mask of bits which are always the same in a network */
	static constexpr uint32_t native_netmask(unsigned char network) {
		return ~native_hostmask(network);
	}

	/** 0.0.0.0/0 */
	explicit network_v4() = default;

	/** addr/32 */
	explicit network_v4(address_type const& addr)
	: m_address(addr), m_network(32) {
	}

	/** addr/network; clears any host bits in addr */
	explicit network_v4(address_type const& addr, unsigned char network)
	: m_address(addr.to_ulong() & native_netmask(network)), m_network(clip_network(network)) {
	}

	/** first address of network range */
	address_type const& address() const {
		return m_address;
	}

	/** native host representation of address, i.e. 1.2.3.4 becomes 0x01020304 */
	uint32_t native_address() const {
		return static_cast<uint32_t>(m_address.to_ulong());
	}

	/** length of network prefix in address */
	unsigned char length() const {
		return m_network;
	}

private:
	address_type m_address;
	unsigned char m_network{0};
};

/** return string representation of network */
std::string to_string(network_v4 value);

/** represents an IPv4 network, i.e. base address + network (netmask length) */
class network_v6 {
public:
	/** underlying address type */
	typedef boost::asio::ip::address_v6 address_type;

	/** clip network length to allowed range */
	static constexpr unsigned char clip_network(unsigned char network) {
		return network >= 128 ? 128 : network;
	}

	/** ::/0 */
	explicit network_v6() = default;

	/** addr/128 */
	explicit network_v6(address_type const& addr)
	: m_addr_bytes(addr.to_bytes()), m_scope_id(addr.scope_id()), m_network(128) {
	}

	/** addr/network; clears any host bits in addr */
	explicit network_v6(address_type const& addr, unsigned char network)
	: m_addr_bytes(addr.to_bytes()), m_scope_id(addr.scope_id()), m_network(clip_network(network)) {
		// TODO: clear bits
	}

	/** first address of network range */
	address_type address() const {
		return address_type(m_addr_bytes, m_scope_id);
	}

	/** bytes of first address of network range */
	address_type::bytes_type const& address_bytes() const {
		return m_addr_bytes;
	}

	/** length of network prefix in address */
	unsigned char length() const {
		return m_network;
	}

private:
	// The underlying IPv6 address.
	address_type::bytes_type m_addr_bytes;

	// The scope ID associated with the address.
	unsigned long m_scope_id;

	unsigned char m_network{0};
};

/** return string representation of network */
std::string to_string(network_v6 value);

__CANEY_STDV1_END
