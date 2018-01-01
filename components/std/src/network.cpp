#include <caney/std/network.hpp>

__CANEY_STDV1_BEGIN

std::string to_string(network_v4 value)
{
	return value.address().to_string() + "/" + std::to_string(value.length());
}

__CANEY_STDV1_END
