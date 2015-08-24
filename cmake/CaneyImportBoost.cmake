find_package(Boost REQUIRED COMPONENTS system thread unit_test_framework)

# extra arguments: dependencies on other boost components
function(_caney_add_boost_compoment _comp)
	string(TOUPPER "${_comp}" _upperComp)
	# message(STATUS "Importing caney::boost::${_comp}, depending on '${ARGN}' boost components, located at '${Boost_${_upperComp}_LIBRARY_RELEASE}', includes in '${Boost_INCLUDE_DIR}'")

	add_library("caney::boost::${_comp}" SHARED IMPORTED GLOBAL)
	set_property(TARGET "caney::boost::${_comp}" PROPERTY IMPORTED_LOCATION ${Boost_${_upperComp}_LIBRARY_RELEASE})
	set_property(TARGET "caney::boost::${_comp}" APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES ${Boost_INCLUDE_DIR})

	foreach(_dep ${ARGN})
		target_link_libraries("caney::boost::${_comp}" INTERFACE "caney::boost::${_dep}")
	endforeach()
endfunction()

_caney_add_boost_compoment(system)
_caney_add_boost_compoment(thread system)
_caney_add_boost_compoment(unit_test_framework system)

if("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Clang" OR "${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	set(CANEY_BOOST_LINK_THREAD_SUPPORT "pthread" CACHE STRING "extra libraries to link for thread support")
else()
	set(CANEY_BOOST_LINK_THREAD_SUPPORT "" CACHE STRING "extra libraries to link for thread support")
endif()

target_link_libraries("caney::boost::thread" INTERFACE ${CANEY_BOOST_LINK_THREAD_SUPPORT})
