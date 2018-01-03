cmake_minimum_required (VERSION 2.6)
project(tyrantnet)

if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
elseif(UNIX)
	if (CMAKE_COMPILER_IS_GNUCC AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 7.0)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
	else()
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
	endif()
endif()

set(LIBRARY_OUTPUT_PATH "${PROJECT_BINARY_DIR}/lib")

set(net_src_list
	${PROJECT_SOURCE_DIR}/src/tyrant/net/eventloop.cc
	${PROJECT_SOURCE_DIR}/src/tyrant/net/currentthread.cc
	${PROJECT_SOURCE_DIR}/src/tyrant/net/datasocket.cc
	${PROJECT_SOURCE_DIR}/src/tyrant/net/socketlibfunction.cc
	${PROJECT_SOURCE_DIR}/src/tyrant/net/connector.cc
    ${PROJECT_SOURCE_DIR}/src/tyrant/net/listenthread.cc
	${PROJECT_SOURCE_DIR}/src/tyrant/net/fdset.c
	${PROJECT_SOURCE_DIR}/src/tyrant/net/tcpservice.cc
	${PROJECT_SOURCE_DIR}/src/tyrant/net/wraptcpservice.cc
	${PROJECT_SOURCE_DIR}/src/tyrant/net/sslhelper.cc
	)

set(timer_src_list
    ${PROJECT_SOURCE_DIR}/src/tyrant/timer/timer.cc
)

set(common_src_list
	${PROJECT_SOURCE_DIR}/src/tyrant/common/base64.cc
	${PROJECT_SOURCE_DIR}/src/tyrant/common/md5calc.cc
	${PROJECT_SOURCE_DIR}/src/tyrant/common/sha1.cc
	${PROJECT_SOURCE_DIR}/src/tyrant/common/array.c
	${PROJECT_SOURCE_DIR}/src/tyrant/common/buffer.c
)

set(http_src_list
	${PROJECT_SOURCE_DIR}/src/tyrant/net/http/http_parser.c
	${PROJECT_SOURCE_DIR}/src/tyrant/net/http/httpparser.cc
	${PROJECT_SOURCE_DIR}/src/tyrant/net/http/httpservice.cc
	)

include_directories("${PROJECT_SOURCE_DIR}/src/")
if(WIN32)
	add_library(tyrantnet ${net_src_list} ${timer_src_list} ${http_src_list} ${common_src_list})
elseif(UNIX)
  	add_library(tyrantnet SHARED ${net_src_list} ${timer_src_list} ${http_src_list} ${common_src_list})
  	find_package(Threads REQUIRED)
  	target_link_libraries(tyrantnet pthread)
endif()

install(TARGETS tyrantnet DESTINATION lib)

file(GLOB TYRANT_NET_PUBLIC_HEADERS ${PROJECT_SOURCE_DIR}/src/tyrant/net/*.h)
install (FILES ${TYRANT_NET_PUBLIC_HEADERS} DESTINATION "include/tyrant/net")

file(GLOB TYRANT_TIMER_PUBLIC_HEADERS ${PROJECT_SOURCE_DIR}/src/tyrant/timer/*.h)
install (FILES ${TYRANT_TIMER_PUBLIC_HEADERS} DESTINATION "include/tyrant/timer")

file(GLOB TYRANT_COMMON_PUBLIC_HEADERS ${PROJECT_SOURCE_DIR}/src/tyrant/common/*.h)
install (FILES ${TYRANT_COMMON_PUBLIC_HEADERS} DESTINATION "include/tyrant/common")

file(GLOB TYRANT_HTTP_PUBLIC_HEADERS ${PROJECT_SOURCE_DIR}/src/tyrant/net/http/*.h)
install (FILES ${TYRANT_HTTP_PUBLIC_HEADERS} DESTINATION "include/tyrant/net/http")
add_subdirectory(example)
