# - Try to find LibDBus-c++-1
# Once done, this will define
#
#  LibDBus-c++_FOUND - system has libdbus-c++-1
#  LibDBus-c++_INCLUDE_DIRS - the libdbus-c++-1 include directories
#  LibDBus-c++_LIBRARIES - link these to use libdbus-c++-1

include(LibFindMacros)

# Dependencies
libfind_package(LibDBus-c++-1 LibDBus-1)

# Use pkg-config to get hints about paths
libfind_pkg_check_modules(LibDBus-c++-1_PKGCONF dbus-c++-1)

# Include dir
find_path(LibDBus-c++-1_INCLUDE_DIR
  NAMES dbus-c++/dbus.h
  PATH_SUFFIXES dbus-c++-1
  PATHS ${LibDBus-c++-1_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(LibDBus-c++-1_LIBRARY
  NAMES dbus-c++-1
  PATHS ${LibDBus-c++-1_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(LibDBus-c++-1_PROCESS_INCLUDES LibDBus-c++-1_INCLUDE_DIR LibDBus-1_INCLUDE_DIRS)
set(LibDBus-c++-1_PROCESS_LIBS LibDBus-c++-1_LIBRARY LibDBus-1_LIBRARIES)
libfind_process(LibDBus-c++-1)