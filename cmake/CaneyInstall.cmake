# support multiarch, e.g. with debian: cmake -DCANEY_INSTALL_LIB_DIR=lib/${DEB_HOST_MULTIARCH}
set(CANEY_INSTALL_LIB_DIR "lib" CACHE STRING "directory relative to CMAKE_INSTALL_PREFIX to install libraries in")
set_property(CACHE CANEY_INSTALL_LIB_DIR PROPERTY ADVANCED TRUE)
