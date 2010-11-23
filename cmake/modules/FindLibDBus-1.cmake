# - Try to find libdbus-1
# Once done, this will define
#
#  LibDBus-1_FOUND - system has libdbus-1
#  LibDBus-1_INCLUDE_DIRS - the libdbus-1 include directories
#  LibDBus-1_LIBRARIES - link these to use libdbus-c++-1

include(LibFindMacros)


# Use pkg-config to get hints about paths
libfind_pkg_check_modules(LibDBus-1_PKGCONF dbus-1)

# Include dir
find_path(LibDBus-1_INCLUDE_DIR
  NAMES dbus/dbus-arch-deps.h
  PATH_SUFFIXES dbus-1.0
  PATHS ${LibDBus-1_PKGCONF_INCLUDE_DIRS}
)

# Finally the library itself
find_library(LibDBus-1_LIBRARY
  NAMES dbus-1
  PATHS ${LibDBus-1_PKGCONF_LIBRARY_DIRS}
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(LibDBus-1_PROCESS_INCLUDES LibDBus-1_INCLUDE_DIR)
set(LibDBus-1_PROCESS_LIBS LibDBus-1_LIBRARY)
libfind_process(LibDBus-1)