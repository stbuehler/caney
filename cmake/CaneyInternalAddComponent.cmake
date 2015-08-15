include(CMakeParseArguments)

set(CANEY_LIBRARY_TYPE "STATIC" CACHE STRING "use static/shared libraries by default")
set_property(CACHE CANEY_LIBRARY_TYPE PROPERTY STRINGS "STATIC;SHARED")

macro(_CaneyGlobFiles _type _glob_var _define_var)
	# message(STATUS "for ${_type} got defined '${${_define_var}}' (${_define_var})")
	if("${${_define_var}}" STREQUAL "auto")
		set(${_define_var} ${${_glob_var}})
	else()
		if((${_glob_var} AND (NOT ${_define_var})) OR ((NOT ${_glob_var}) AND ${_define_var}))
			message(AUTHOR_WARNING "${_type} list not matching: defined '${${_define_var}}' but found '${${_glob_var}}'")
		elseif(${_glob_var} AND ${_define_var})
			list(SORT ${_define_var})
			list(SORT ${_glob_var})
			if(NOT(${_define_var} STREQUAL ${_glob_var}))
				message(AUTHOR_WARNING "${_type} list not matching: defined '${${_define_var}}' but found '${${_glob_var}}'")
			endif()
		endif()
	endif()
endmacro()

# CaneyAddLibrary(component <....>)
#	HEADERS header files...
#	SOURCES sources files...
#	DEPENDS caney components to depend on (must be defined before)
#	LINK other targets the interface depends on
#	PRIVATE_LINK other targets the implementation depends on
function(CaneyAddLibrary _comp)
	cmake_parse_arguments(args "" "" "HEADERS;SOURCES;DEPENDS;LINK;PRIVATE_LINK" ${ARGN})
	if(args_UNPARSED_ARGUMENTS)
		message(FATAL_ERROR "CaneyAddLibrary: unknown arguments '${args_UNPARSED_ARGUMENTS}'")
	endif()

	# message(STATUS "CaneyAddLibrary ${_comp}: headers '${args_HEADERS}', sources '${args_SOURCES}', depends '${args_DEPENDS}', links '${args_LINK}', private links '${args_PRIVATE_LINK}'")

	file(GLOB_RECURSE glob_headers RELATIVE "${CMAKE_CURRENT_SOURCE}" include/*.h include/*.hpp include/*.inc)
	_CaneyGlobFiles("header" glob_headers args_HEADERS)

	file(GLOB_RECURSE glob_sources RELATIVE "${CMAKE_CURRENT_SOURCE}" src/*.cpp)
	_CaneyGlobFiles("source" glob_sources args_SOURCES)

	set(_target "caney-${_comp}")

	if(args_SOURCES)
		# have source: provide a STATIC and a SHARED library target, default to one of them
		# also pull all dependencies (libs + include paths)

		# use a common OBJECT library to compile the sources only once; cannot link against
		# an OBJECT library (which would pull all dependencies to the static/shared target);
		# instead need to manually "copy" them to STATIC and SHARED targets; OBJECT library
		# also needs all include paths (but not the libs)
		message(STATUS "creating ${CANEY_LIBRARY_TYPE} caney::${_comp} library")

		add_library("caney-objects-${_comp}" OBJECT ${args_SOURCES} ${args_HEADERS})
		set_property(TARGET "caney-objects-${_comp}" PROPERTY POSITION_INDEPENDENT_CODE ON) # needed to build shared library from this
		set_property(TARGET "caney-objects-${_comp}" PROPERTY EXCLUDE_FROM_ALL ${CANEY_EXCLUDE_LIBRARY_FROM_ALL})
		target_include_directories("caney-objects-${_comp}" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

		add_library("caney-static-${_comp}" STATIC "$<TARGET_OBJECTS:caney-objects-${_comp}>")
		set_property(TARGET "caney-static-${_comp}" PROPERTY EXCLUDE_FROM_ALL ${CANEY_EXCLUDE_LIBRARY_FROM_ALL})
		target_include_directories("caney-static-${_comp}" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
		set_property(TARGET "caney-static-${_comp}" PROPERTY OUTPUT_NAME "caney_${_comp}")
		set_property(TARGET "caney-static-${_comp}" PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CANEY_BINARY_DIR}")

		add_library("caney-shared-${_comp}" SHARED "$<TARGET_OBJECTS:caney-objects-${_comp}>")
		set_property(TARGET "caney-shared-${_comp}" PROPERTY EXCLUDE_FROM_ALL ${CANEY_EXCLUDE_LIBRARY_FROM_ALL})
		target_include_directories("caney-shared-${_comp}" PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
		set_property(TARGET "caney-shared-${_comp}" PROPERTY OUTPUT_NAME "caney_${_comp}")
		set_property(TARGET "caney-shared-${_comp}" PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CANEY_BINARY_DIR}")
		set_property(TARGET "caney-shared-${_comp}" PROPERTY SOVERSION ${caney_VERSION_MAJOR})
		set_property(TARGET "caney-shared-${_comp}" PROPERTY VERSION ${caney_VERSION})

		foreach(_dep ${args_DEPENDS})
			if(NOT(TARGET "caney-objects-${_dep}"))
				message(FATAL_ERROR "${_dep} is not a valid caney component (${_comp} requires it)")
			endif()
			target_include_directories("caney-objects-${_comp}" PUBLIC $<TARGET_PROPERTY:caney-objects-${_dep},INTERFACE_INCLUDE_DIRECTORIES>)
			# the shared library pulls shared targets, the static library static targets
			target_link_libraries("caney-static-${_comp}" PUBLIC "caney-static-${_dep}")
			target_link_libraries("caney-shared-${_comp}" PUBLIC "caney-shared-${_dep}")
		endforeach()

		target_link_libraries("caney-static-${_comp}" PUBLIC ${args_PRIVATE_LINK} ${args_LINK})
		target_link_libraries("caney-shared-${_comp}" PRIVATE ${args_PRIVATE_LINK} PUBLIC ${args_LINK})

		set_property(TARGET "caney-static-${_comp}" APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:caney-objects-${_comp},INTERFACE_INCLUDE_DIRECTORIES>)
		set_property(TARGET "caney-shared-${_comp}" APPEND PROPERTY INTERFACE_INCLUDE_DIRECTORIES $<TARGET_PROPERTY:caney-objects-${_comp},INTERFACE_INCLUDE_DIRECTORIES>)

		if(CANEY_INSTALL_STATIC)
			install(TARGETS "caney-static-${_comp}"
				ARCHIVE DESTINATION "${CANEY_INSTALL_LIB_DIR}")
		endif()
		if(CANEY_INSTALL_SHARED)
			install(TARGETS "caney-shared-${_comp}"
				LIBRARY DESTINATION "${CANEY_INSTALL_LIB_DIR}")
		endif()
	else()
		# no source: provide a INTERFACE library
		# also pull all dependencies (libs + include paths)
		message(STATUS "creating HEADER caney::${_comp} library")

		add_library("caney-objects-${_comp}" INTERFACE)
		target_sources("caney-objects-${_comp}" INTERFACE ${args_HEADERS})
		target_include_directories("caney-objects-${_comp}" INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
		target_link_libraries("caney-objects-${_comp}" INTERFACE ${args_PRIVATE_LINK} ${args_LINK})

		# make the files show up in qtcreator even if the library isn't used
		add_custom_target("caney-objects-${_comp}-headers" SOURCES ${args_HEADERS})

		add_library("caney-static-${_comp}" INTERFACE)
		target_link_libraries("caney-static-${_comp}" INTERFACE "caney-objects-${_comp}")

		add_library("caney-shared-${_comp}" INTERFACE)
		target_link_libraries("caney-shared-${_comp}" INTERFACE "caney-objects-${_comp}")

		foreach(_dep ${args_DEPENDS})
			target_link_libraries("caney-static-${_comp}" INTERFACE "caney-static-${_dep}")
			target_link_libraries("caney-shared-${_comp}" INTERFACE "caney-shared-${_dep}")
		endforeach()
	endif()

	if(CANEY_INSTALL_HEADERS AND args_HEADERS)
		install(DIRECTORY include/ DESTINATION include)
	endif()

	if("SHARED" STREQUAL ${CANEY_LIBRARY_TYPE})
		add_library("caney::${_comp}" ALIAS "caney-shared-${_comp}")
	else()
		add_library("caney::${_comp}" ALIAS "caney-static-${_comp}")
	endif()
endfunction()
