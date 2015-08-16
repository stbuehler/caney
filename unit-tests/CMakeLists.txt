add_executable(flags_test
	flags_test.cpp
)
target_link_libraries(flags_test PRIVATE caney::std caney::boost_unit_test)
add_test(NAME flags COMMAND flags_test)

add_executable(weakfn_test
	weakfn_test.cpp
)
target_link_libraries(weakfn_test PRIVATE caney::std)
add_test(NAME weak_fn COMMAND weakfn_test)

add_executable(movearg_test
	movearg_test.cpp
)
target_link_libraries(movearg_test PRIVATE caney::util caney::boost_unit_test)
add_test(NAME movearg COMMAND movearg_test)

add_executable(bencode_test
	bencode_test.cpp
)
target_link_libraries(bencode_test PRIVATE caney::bencode caney::boost_unit_test)
add_test(NAME bencode COMMAND bencode_test)
