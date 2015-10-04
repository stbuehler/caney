set(CANEY_CREATE_DOCUMENTATION "ON" CACHE BOOL "whether to build documentation")

if(CANEY_CREATE_DOCUMENTATION)
	find_package(Doxygen REQUIRED)

	configure_file("${CMAKE_CURRENT_SOURCE_DIR}/Doxyfile.in" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile" @ONLY)

	add_custom_target(doc
		ALL
		COMMAND "${DOXYGEN_EXECUTABLE}" "${CMAKE_CURRENT_BINARY_DIR}/Doxyfile"
		WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}"
		COMMENT "Generating API documentation with Doxygen"
		VERBATIM
	)
endif()
