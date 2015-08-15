#include "caney/std/flags.hpp"

enum class my_flag : size_t {
	foo0 = 0,
	foo1,
	foo2,
	foo3,
	foo4,
	Last = foo4,
};
using my_flags = caney::flags<my_flag>;
CANEY_FLAGS(my_flags);

int main() {
	my_flags f = my_flag::foo0 | my_flag::foo1;
	f ^= my_flag::foo3;
	f[my_flag::foo4];
	(void)bool{my_flag::foo0 & f};
	(void)bool{f & my_flag::foo0};
	(void)bool(f == (f ^ my_flag::foo1));
	f = ~my_flag::foo2;
	f.flip_all();

	return 0;
}
