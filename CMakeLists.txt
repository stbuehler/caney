cmake_minimum_required(VERSION 3.1 FATAL_ERROR)
cmake_policy(SET CMP0048 NEW)

project(caney
	VERSION 0.0.0
	LANGUAGES CXX
)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
	set(CANEY_TOP TRUE)

	# default to install everything
	set(CANEY_INSTALL_STATIC TRUE CACHE BOOLEAN "install static libraries")
	set(CANEY_INSTALL_SHARED TRUE CACHE BOOLEAN "install shared libraries")
	set(CANEY_INSTALL_HEADERS TRUE CACHE BOOLEAN "install headers")

	set(CANEY_EXCLUDE_LIBRARY_FROM_ALL FALSE)

	include(CaneyDefaultBuildType)
	include(CaneyExtraFlags)
	include(CaneyCxx14Flags)
else()
	set(CANEY_TOP FALSE)

	set(CANEY_INSTALL_STATIC FALSE CACHE BOOLEAN "install static libraries")
	set(CANEY_INSTALL_SHARED FALSE CACHE BOOLEAN "install shared libraries")
	set(CANEY_INSTALL_HEADERS FALSE CACHE BOOLEAN "install headers")

	set(CANEY_EXCLUDE_LIBRARY_FROM_ALL TRUE)
endif()

include(CaneyInternalAddComponent)
include(CaneyImportBoost)
include(CaneyDocumentation)
include(CaneyInstall)

# CANEY_BUILD_TESTS: whether to build unit tests
if(NOT CANEY_BUILD_TESTS)
	set(CANEY_BUILD_TESTS ${CANEY_TOP})
endif()
set(CANEY_BUILD_TESTS ${CANEY_BUILD_TESTS} CACHE BOOL "whether to build unit tests")
set_property(CACHE CANEY_BUILD_TESTS PROPERTY ADVANCED TRUE)
unset(CANEY_BUILD_TESTS) # unset local variable, use cache var
# message(STATUS "CANEY_BUILD_TESTS = ${CANEY_BUILD_TESTS}")

# component handling
set(CANEY_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}" CACHE INTERNAL "caney source directory")
set(CANEY_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}" CACHE INTERNAL "caney binary directory")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CANEY_BINARY_DIR}")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CANEY_BINARY_DIR}")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CANEY_BINARY_DIR}")

if(CANEY_TOP)
	# don't build unit tests when embedded in other projects
	enable_testing()
endif()
add_subdirectory(components)

add_custom_target("caney-other-texts" SOURCES README.md SOURCE.md COPYING)
