CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
PROJECT(PicasaFUSE)
INCLUDE(CheckIncludeFile)
INCLUDE(CheckLibraryExists)
INCLUDE(UsePkgConfig)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
FIND_PACKAGE(CURL REQUIRED)
FIND_PACKAGE(Boost COMPONENTS thread REQUIRED)

#check_include_file("fuse.h" CMAKE_HAVE_FUSE_H)
#if (CMAKE_HAVE_FUSE_H)
    check_library_exists(fuse fuse_main "" CMAKE_HAVE_FUSE)
    if (CMAKE_HAVE_FUSE)
        find_library(FUSE_LIBRARY NAMES fuse)
    else (CMAKE_HAVE_FUSE)
        message(FATAL_ERROR "libfuse not found")
    endif (CMAKE_HAVE_FUSE)
    #else (CMAKE_HAVE_FUSE_H)
    #message(FATAL_ERROR "fuse.h not found")
    #endif (CMAKE_HAVE_FUSE_H)


INCLUDE_DIRECTORIES(
	${BOOST_INCLUDE_DIRS}
	${CURL_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}
)


FILE(GLOB TINY_XML_SRC ticpp/*.cpp)

SET(PICASAFUSE_SRC
	gAPI.cpp
	curlRequest.cpp
	picasaFS.cpp
	picasaFUSE.cpp
	picasaCache.cpp
	pathParser.cpp
)

ADD_EXECUTABLE(picasaFUSE
	${PICASAFUSE_SRC}
	${TINY_XML_SRC}
)

ADD_EXECUTABLE(testPathParser
	testPathParser.cpp
	pathParser.cpp
)

TARGET_LINK_LIBRARIES(picasaFUSE
	${Boost_LIBRARIES}
	${CURL_LIBRARIES}
	${FUSE_LIBRARY}
)


 
LINK_DIRECTORIES(
	${CMAKE_CURRENT_BINARY_DIR}/ticpp
) 



ADD_DEFINITIONS(
        -DDEBUG
        -g
        -DTIXML_USE_STL
        -DTIXML_USE_TICPP
	-D_FILE_OFFSET_BITS=64
)

INSTALL(TARGETS picasaFUSE
        DESTINATION "bin"
)
