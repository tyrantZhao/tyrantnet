cmake_minimum_required (VERSION 2.8.9)
project(tyrantnet)

OPTION(TYRANTNET_BUILD_TYPE_DEBUG "Build tyrantnet in debug mode" OFF)
option(TYRANTNET_BUILD_EXAMPLES "Build tyrantnet examples" ON)
option(TYRANTNET_BUILD_TESTINGS "Build tyrantnet testings" OFF)
option(TYRANTNET_USE_OPENSSL "use openssl if found openssl" ON)

if(TYRANTNET_USE_OPENSSL)
	find_library(LIB_OPENSSL NAMES crypto ssl)
	if(LIB_OPENSSL)
		add_definitions(-DUSE_OPENSSL)
		SET(CMAKE_EXE_LINKER_FLAGS  "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-as-needed -ldl -lrt -lssl -lcrypto")
	elseif()
		message(WARNING "not found openssl")
	endif()
endif()

if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
elseif(UNIX)
	if(TYRANTNET_BUILD_TYPE_DEBUG)
		set(CMAKE_BUILD_TYPE "Release")
	endif()
	set(CMAKE_CXX_FLAGS_DEBUG   "-O0 -g -ggdb -Wall -D_DEBUG")
	set(CMAKE_CXX_FLAGS_RELEASE "-O3 -g -ggdb -Wall -DNDEBUG")
	if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
	endif()
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif()

set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")

file(GLOB_RECURSE ALL_SRC
	${PROJECT_SOURCE_DIR}/src/*.cc
	${PROJECT_SOURCE_DIR}/src/*.c)

include_directories("${PROJECT_SOURCE_DIR}/src/")
if(WIN32)
	add_library(tyrantnet ${ALL_SRC})
	set_target_properties(tyrantnet PROPERTIES OUTPUT_NAME_DEBUG "tyrantnetd")
elseif(UNIX)
	add_library(tyrantnet SHARED ${ALL_SRC})
	find_package(Threads REQUIRED)
	target_link_libraries(tyrantnet pthread)
endif()

install(TARGETS tyrantnet DESTINATION lib)

file(GLOB TYRANTNET_NET_PUBLIC_HEADERS ${PROJECT_SOURCE_DIR}/src/tyrantnet/net/*.h)
install(FILES ${TYRANTNET_NET_PUBLIC_HEADERS} DESTINATION "include/tyrantnet/net")

file(GLOB TYRANTNET_TIMER_PUBLIC_HEADERS ${PROJECT_SOURCE_DIR}/src/tyrantnet/timer/*.h)
install(FILES ${TYRANTNET_TIMER_PUBLIC_HEADERS} DESTINATION "include/tyrantnet/timer")

file(GLOB TYRANTNET_COMMON_PUBLIC_HEADERS ${PROJECT_SOURCE_DIR}/src/tyrantnet/common/*.h)
install(FILES ${TYRANTNET_COMMON_PUBLIC_HEADERS} DESTINATION "include/tyrantnet/common")

file(GLOB TYRANTNET_HTTP_PUBLIC_HEADERS ${PROJECT_SOURCE_DIR}/src/tyrantnet/net/http/*.h)
install(FILES ${TYRANTNET_HTTP_PUBLIC_HEADERS} DESTINATION "include/tyrantnet/net/http")

file(GLOB TYRANTNET_PORT_PUBLIC_HEADERS ${PROJECT_SOURCE_DIR}/src/tyrantnet/net/port/*.h)
install(FILES ${TYRANTNET_PORT_PUBLIC_HEADERS} DESTINATION "include/tyrantnet/net/port")

if(TYRANTNET_BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if(TYRANTNET_BUILD_TESTINGS)
	if(WIN32)
		if (MSVC_VERSION VERSION_GREATER 1910)
			add_subdirectory(tests)
		endif()
	elseif(UNIX)
		if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 4.9)
			add_subdirectory(tests)
		endif()
	endif()
endif()



