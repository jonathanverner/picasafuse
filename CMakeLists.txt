CMAKE_MINIMUM_REQUIRED(VERSION 2.4)
PROJECT(PicasaFUSE)
INCLUDE(CheckIncludeFile)
INCLUDE(CheckLibraryExists)
INCLUDE(UsePkgConfig)

SET(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
FIND_PACKAGE(CURL REQUIRED)
SET(Boost_MINIMUM_VERSION "1.39")
#SET(Boost_ADDITIONAL_VERSIONS "1.39.0")
#SET(Boost_INCLUDE_DIR "/usr/local/include/boost-1_39")
SET(Boost_USE_STATIC_LIBS OFF)

FIND_PACKAGE(Boost COMPONENTS thread filesystem system serialization REQUIRED)
FIND_PACKAGE(ImageMagick COMPONENTS Magick++ REQUIRED)

FIND_PACKAGE(LibDBus-c++-1 QUIET)
IF (LibDBus-c++-1_FOUND)
  ADD_DEFINITIONS(-DHAVE_DBUS)
ELSE (${LibDBus-c++-1_FOUND)
  MESSAGE(STATUS "libdbus-c++-1 not found, disabling kwallet support")
ENDIF (LibDBus-c++-1_FOUND)


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
	${Boost_INCLUDE_DIR}
	${BOOST_INCLUDE_DIRS}
	${CURL_INCLUDE_DIRS}
	${CMAKE_CURRENT_SOURCE_DIR} 
	${CMAKE_CURRENT_BINARY_DIR}
        ${ImageMagick_INCLUDE_DIRS}
        ${LibDBus-c++-1_INCLUDE_DIRS}
)


FILE(GLOB TINY_XML_SRC ticpp/*.cpp)

SET(IMAGE_MANIP_SRC
	convert.cpp
)

SET(PICASAAPI_SRC
	curlRequest.cpp
	gAPI.cpp
	atomEntry.cpp
	atomObj.cpp
	atomFeed.cpp
	picasaAlbum.cpp
	picasaPhoto.cpp
	picasaService.cpp
)

	
SET(PICASAFUSE_SRC
	config.cpp
	picasaFS.cpp
	picasaFUSE.cpp
	picasaCache.cpp
	pathParser.cpp
)

ADD_EXECUTABLE(picasaFUSE
	${PICASAAPI_SRC}
	${PICASAFUSE_SRC}
	${TINY_XML_SRC}
	${IMAGE_MANIP_SRC}
)

ADD_EXECUTABLE(testPathParser
	testPathParser.cpp
	pathParser.cpp
)

ADD_EXECUTABLE(testAlbumList
	testAlbumList.cpp
	${PICASAAPI_SRC}
	${TINY_XML_SRC}
)

ADD_EXECUTABLE(testNewAlbum
	testNewAlbum.cpp
	${PICASAAPI_SRC}
	${TINY_XML_SRC}
)

ADD_EXECUTABLE(testPhotoList
	testPhotoList.cpp
	${PICASAAPI_SRC}
	${TINY_XML_SRC}
)

ADD_EXECUTABLE(testPhotoUpload
        testPhotoUpload.cpp
        ${PICASAAPI_SRC}
        ${TINY_XML_SRC}
	${IMAGE_MANIP_SRC}
)

TARGET_LINK_LIBRARIES(testPhotoList
	${Boost_LIBRARIES}
	${CURL_LIBRARIES}
)

TARGET_LINK_LIBRARIES(testAlbumList
	${Boost_LIBRARIES}
	${CURL_LIBRARIES}
)


TARGET_LINK_LIBRARIES(testNewAlbum
	${Boost_LIBRARIES}
	${CURL_LIBRARIES}
)

TARGET_LINK_LIBRARIES(testPhotoUpload
	${Boost_LIBRARIES}
	${CURL_LIBRARIES}
        ${ImageMagick_LIBRARIES}
)

TARGET_LINK_LIBRARIES(picasaFUSE
	${Boost_LIBRARIES}
	${CURL_LIBRARIES}
	${FUSE_LIBRARY}
	${ImageMagick_LIBRARIES}
	${LibDBus-c++-1_LIBRARIES}
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
