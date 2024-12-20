# ----------------------------------------------------------------------------
#  Root CMake file for websocket-client
#
# ----------------------------------------------------------------------------
cmake_minimum_required (VERSION 3.8)

project ("websocket-client")
set(PROJECT_DESCRIPTION "C/C++ WebSocket network library")

include(GNUInstallDirs)

# C++ standard requirements.
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Options
option(BUILD_SHARED_LIBS "Build shared library" ON)
option(BUILD_SHARED_DEPS_LIBS "Build submodules as shared libraries" OFF)
option(USE_GNUTLS "Use GnuTLS instead of OpenSSL" OFF)
option(USE_MBEDTLS "Use Mbed TLS instead of OpenSSL" OFF)
option(USE_SYSTEM_PLOG "Use system Plog" ${PREFER_SYSTEM_LIB})
option(WSC_UPDATE_VERSION_HEADER "Enable updating the version header" OFF)

if (USE_GNUTLS AND USE_MBEDTLS)
	message(FATAL_ERROR "Both USE_MBEDTLS and USE_GNUTLS cannot be enabled at the same time")
endif()

if(USE_GNUTLS)
	option(USE_NETTLE "Use Nettle in libjuice" ON)
else()
	option(USE_NETTLE "Use Nettle in libjuice" OFF)
endif()

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/Modules)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

if(WIN32)
	add_definitions(-DWIN32_LEAN_AND_MEAN)
	if(MSVC)
		add_definitions(-DNOMINMAX)
		add_definitions(-D_CRT_SECURE_NO_WARNINGS)
	endif()
endif()

set(LIBWEBSOCKET_CLIENT_HEADERS
  ${CMAKE_CURRENT_SOURCE_DIR}/include/channel.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/common.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/configuration.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/frameinfo.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/global.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/message.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/reliability.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/websocketclient.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/utils.hpp
  ${CMAKE_CURRENT_SOURCE_DIR}/include/websocket.hpp
)

set(
	WSC_SOURCE_FILES

  src/global.cpp
  src/message.cpp
	src/websocket.cpp
  src/channel.cpp
  src/configuration.cpp
  src/capi.cpp

  src/impl/certificate.hpp
  src/impl/certificate.cpp
  src/impl/channel.hpp
  src/impl/channel.cpp
  src/impl/http.hpp
  src/impl/http.cpp
  src/impl/httpproxytransport.hpp
  src/impl/httpproxytransport.cpp
  src/impl/init.hpp
  src/impl/init.cpp
  src/impl/internals.hpp
  src/impl/tls.hpp
  src/impl/tls.cpp
  src/impl/init.hpp
  src/impl/init.cpp
  src/impl/pollinterrupter.hpp
  src/impl/pollinterrupter.cpp
  src/impl/pollservice.hpp
  src/impl/pollservice.cpp
  src/impl/processor.hpp
  src/impl/processor.cpp
  src/impl/sha.hpp
  src/impl/sha.cpp
  src/impl/socket.hpp
  src/impl/tcptransport.hpp
  src/impl/tcptransport.cpp
  src/impl/threadpool.hpp
  src/impl/threadpool.cpp
  src/impl/tlstransport.hpp
  src/impl/tlstransport.cpp
  src/impl/transport.hpp
  src/impl/transport.cpp
  src/impl/utils.hpp
  src/impl/utils.cpp
  src/impl/verifiedtlstransport.hpp
  src/impl/verifiedtlstransport.cpp
  src/impl/websocketimpl.hpp
  src/impl/websocketimpl.cpp
  src/impl/wshandshake.hpp
  src/impl/wshandshake.cpp
  src/impl/wstransport.hpp
  src/impl/wstransport.cpp
)

if(WSC_UPDATE_VERSION_HEADER)
	configure_file (
		${PROJECT_SOURCE_DIR}/cmake/version.h.in
		${CMAKE_CURRENT_SOURCE_DIR}/include/version.h
	)
endif()

add_library (websocketclient
	${WSC_SOURCE_FILES}
  ${LIBWEBSOCKET_CLIENT_HEADERS})

add_library(websocketclient-static STATIC EXCLUDE_FROM_ALL
	${WSC_SOURCE_FILES}
  ${LIBWEBSOCKET_CLIENT_HEADERS})

target_compile_definitions(websocketclient PRIVATE WSC_EXPORTS)
if (NOT BUILD_SHARED_LIBS)
	target_compile_definitions(websocketclient PUBLIC WSC_STATIC)
endif()
target_compile_definitions(websocketclient-static PUBLIC WSC_STATIC)

if(USE_SYSTEM_PLOG)
	find_package(plog REQUIRED)
else()
	set(CMAKE_POLICY_DEFAULT_CMP0048 NEW)
	if (NOT TARGET plog)
    add_subdirectory(deps/plog EXCLUDE_FROM_ALL)
  endif()
endif()

target_include_directories(websocketclient PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(websocketclient PRIVATE
	$<BUILD_INTERFACE:plog::plog>)

target_include_directories(websocketclient-static PRIVATE ${PROJECT_SOURCE_DIR}/include)
target_link_libraries(websocketclient-static PRIVATE
  $<BUILD_INTERFACE:plog::plog>)

if(WIN32)
	target_link_libraries(websocketclient PUBLIC ws2_32) # winsock2
	target_link_libraries(websocketclient-static PUBLIC ws2_32) # winsock2
endif()

if (USE_GNUTLS)
	find_package(GnuTLS REQUIRED)
	if(NOT TARGET GnuTLS::GnuTLS)
		add_library(GnuTLS::GnuTLS UNKNOWN IMPORTED)
		set_target_properties(GnuTLS::GnuTLS PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${GNUTLS_INCLUDE_DIRS}"
			INTERFACE_COMPILE_DEFINITIONS "${GNUTLS_DEFINITIONS}"
			IMPORTED_LINK_INTERFACE_LANGUAGES C
			IMPORTED_LOCATION "${GNUTLS_LIBRARIES}")
	endif()
	target_compile_definitions(websocketclient PRIVATE USE_GNUTLS=1)
	target_compile_definitions(websocketclient-static PRIVATE USE_GNUTLS=1)
	target_link_libraries(websocketclient PRIVATE GnuTLS::GnuTLS)
	target_link_libraries(websocketclient-static PRIVATE GnuTLS::GnuTLS)
elseif(USE_MBEDTLS)
	if(NOT TARGET MbedTLS::MbedTLS)
		find_package(MbedTLS 3 REQUIRED)
	endif()
	target_compile_definitions(websocketclient PRIVATE USE_MBEDTLS=1)
	target_compile_definitions(websocketclient-static PRIVATE USE_MBEDTLS=1)
	target_link_libraries(websocketclient PRIVATE MbedTLS::MbedTLS)
	target_link_libraries(websocketclient-static PRIVATE MbedTLS::MbedTLS)
else()
	if(APPLE)
		# This is a bug in CMake that causes it to prefer the system version over
		# the one in the specified ROOT folder
		if(EXISTS ${OPENSSL_ROOT_DIR})
            # Use static files when OPENSSL_USE_STATIC_LIBS is set.
            # OPENSSL_USE_STATIC_LIBS is what CMake's FindOpenSSL looks at
            # to decide whether to use static libraries.
            if(OPENSSL_USE_STATIC_LIBS)
                set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libcrypto.a" CACHE FILEPATH "" FORCE)
                set(OPENSSL_SSL_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libssl.a" CACHE FILEPATH "" FORCE)
            else()
                set(OPENSSL_CRYPTO_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libcrypto.dylib" CACHE FILEPATH "" FORCE)
                set(OPENSSL_SSL_LIBRARY "${OPENSSL_ROOT_DIR}/lib/libssl.dylib" CACHE FILEPATH "" FORCE)
            endif()
		endif()
	endif()
	find_package(OpenSSL REQUIRED)
	target_compile_definitions(websocketclient PRIVATE USE_GNUTLS=0)
	target_compile_definitions(websocketclient-static PRIVATE USE_GNUTLS=0)
	target_link_libraries(websocketclient PRIVATE OpenSSL::SSL)
	target_link_libraries(websocketclient-static PRIVATE OpenSSL::SSL)
endif()

if(NOT BUILD_EXAMPLES)
	set(BUILD_EXAMPLES true)
endif()
# examples
if(BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()
