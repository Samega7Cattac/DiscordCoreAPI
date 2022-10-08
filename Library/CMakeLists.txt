#
#	DiscordCoreAPI, A bot library for Discord, written in C++, and featuring explicit multithreading through the usage of custom, asynchronous C++ CoRoutines.
#
#	Copyright 2021, 2022 Chris M. (RealTimeChris)
#
#	This library is free software; you can redistribute it and/or
#	modify it under the terms of the GNU Lesser General Public
#	License as published by the Free Software Foundation; either
#	version 2.1 of the License, or (at your option) any later version.
#
#	This library is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
#	Lesser General Public License for more details.
#
#	You should have received a copy of the GNU Lesser General Public
#	License along with this library; if not, write to the Free Software
#	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301
#	USA
#
# CMakeLists.txt - The CMake script for building this library.
# May 13, 2021
# https://discordcoreapi.com

include(GenerateProductVersion)
generate_product_version(
	PRODUCT_INFO
	NAME "${LIB_NAME}"
	FILE_DESCRIPTION "${DESCRIPTION}"
	ORIGINAL_FILENAME "discordcoreapi.dll"
	COMPANY_NAME "RealTimeChris Inc."
	VERSION_MAJOR "1"
	VERSION_MINOR "0"
	VERSION_PATCH "0"
	VERSION_REVISION "0"
)

file(GLOB SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Source/*.cpp")

if (WIN32)
	add_library("${LIB_NAME}" SHARED "${SOURCES}" "${PRODUCT_INFO}")
elseif (UNIX)
	add_library("${LIB_NAME}" STATIC "${SOURCES}")
	find_package(Threads REQUIRED)
endif()

add_library("${PROJECT_NAME}::${LIB_NAME}" ALIAS "${LIB_NAME}")

file(GLOB HEADERS CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Include/discordcoreapi/*.hpp")

set_target_properties(
	"${LIB_NAME}" PROPERTIES
	PUBLIC_HEADER "${HEADERS}"
	OUTPUT_NAME "discordcoreapi"
	CXX_STANDARD_REQUIRED ON
	CXX_EXTENSIONS OFF
)

find_package(Threads REQUIRED)

set (SIMDJSON_INCLUDE_DIR "" CACHE PATH "Path to the include dir of ffmpeg simdjson lib")
set (SIMDJSON_LIBRARY_PATH "" CACHE PATH "Path to the binary of ffmpeg simdjson lib")
if ("${SIMDJSON_INCLUDE_DIR}" STREQUAL "" AND
	"${SIMDJSON_LIBRARY_PATH}" STREQUAL "")
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SIMDJSON REQUIRED simdjson)
	set (SIMDJSON_LIBRARY_PATH ${SIMDJSON_LINK_LIBRARIES})
	set (SIMDJSON_INCLUDE_DIR ${SIMDJSON_INCLUDE_DIRS})
endif()

set (OPENSSL_INCLUDE_DIR "" CACHE PATH "Path to the include dir of OpenSSL lib")
set (OPENSSL_LIBRARY_PATH "" CACHE PATH "Path to the binary of OpenSSL lib")
if ("${OPENSSL_INCLUDE_DIR}" STREQUAL "" AND
	"${OPENSSL_LIBRARY_PATH}" STREQUAL "")
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(OPENSSL REQUIRED openssl)
	set (OPENSSL_LIBRARY_PATH ${OPENSSL_LINK_LIBRARIES})
	set (OPENSSL_INCLUDE_DIR ${OPENSSL_INCLUDE_DIRS})
#	pkg_check_modules(CRYPTO REQUIRED libcrypto)
#	list (APPEND OPENSSL_LIBRARY_PATH ${CRYPTO_LINK_LIBRARIES})
#	list (APPEND OPENSSL_INCLUDE_DIR ${CRYPTO_INCLUDE_DIRS})
#	pkg_check_modules(SSL REQUIRED libssl)
#	list (APPEND OPENSSL_LIBRARY_PATH ${SSL_LINK_LIBRARIES})
#	list (APPEND OPENSSL_INCLUDE_DIR ${SSL_INCLUDE_DIRS})
endif()

set (SODIUM_INCLUDE_DIR "" CACHE PATH "Path to the include dir of Sodium lib")
set (SODIUM_LIBRARY_PATH "" CACHE PATH "Path to the binary of Sodium lib")
if ("${SODIUM_INCLUDE_DIR}" STREQUAL "" AND
	"${SODIUM_LIBRARY_PATH}" STREQUAL "")
#	if (EXISTS "${VCPKG_ROOT}")
#		find_package(unofficial-sodium REQUIRED)
#		get_target_property(SODIUM_LIBRARY_PATH unofficial-sodium::sodium LINK_DIRECTORIES)
#		get_target_property(SODIUM_INCLUDE_DIR unofficial-sodium::sodium INCLUDE_DIRECTORIES)
#	else()
		find_package(PkgConfig REQUIRED)
		pkg_check_modules(SODIUM REQUIRED libsodium)
		set (SODIUM_LIBRARY_PATH ${SODIUM_LINK_LIBRARIES})
		set (SODIUM_INCLUDE_DIR ${SODIUM_INCLUDE_DIRS})
#	endif()
endif()

set (AVFORMAT_INCLUDE_DIR "" CACHE PATH "Path to the include dir of ffmpeg AVFormat lib")
set (AVFORMAT_LIBRARY_PATH "" CACHE PATH "Path to the binary of ffmpeg AVFormat lib")
if ("${AVFORMAT_INCLUDE_DIR}" STREQUAL "" AND
	"${AVFORMAT_LIBRARY_PATH}" STREQUAL "")
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(AVFORMAT REQUIRED libavformat)
	set (AVFORMAT_LIBRARY_PATH ${AVFORMAT_LINK_LIBRARIES})
	set (AVFORMAT_INCLUDE_DIR ${AVFORMAT_INCLUDE_DIRS})
endif()

set (AVCODEC_INCLUDE_DIR "" CACHE PATH "Path to the include dir of ffmpeg AVCodec lib")
set (AVCODEC_LIBRARY_PATH "" CACHE PATH "Path to the binary of ffmpeg AVCodec lib")
if ("${AVCODEC_INCLUDE_DIR}" STREQUAL "" AND
	"${AVCODEC_LIBRARY_PATH}" STREQUAL "")
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(AVCODEC REQUIRED libavcodec)
	set (AVCODEC_LIBRARY_PATH ${AVCODEC_LINK_LIBRARIES})
	set (AVCODEC_INCLUDE_DIR ${AVCODEC_INCLUDE_DIRS})
endif()

set (AVUTIL_INCLUDE_DIR "" CACHE PATH "Path to the include dir of ffmpeg AVUtil lib")
set (AVUTIL_LIBRARY_PATH "" CACHE PATH "Path to the binary of ffmpeg AVUtil lib")
if ("${AVUTIL_INCLUDE_DIR}" STREQUAL "" AND
	"${AVUTIL_LIBRARY_PATH}" STREQUAL "")
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(AVUTIL REQUIRED libavutil)
	set (AVUTIL_LIBRARY_PATH ${AVUTIL_LINK_LIBRARIES})
	set (AVUTIL_INCLUDE_DIR ${AVUTIL_INCLUDE_DIRS})
endif()

set (SWRESAMPLE_INCLUDE_DIR "" CACHE PATH "Path to the include dir of ffmpeg SWResample lib")
set (SWRESAMPLE_LIBRARY_PATH "" CACHE PATH "Path to the binary of ffmpeg SWResample lib")
if ("${SWRESAMPLE_INCLUDE_DIR}" STREQUAL "" AND
	"${SWRESAMPLE_LIBRARY_PATH}" STREQUAL "")
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(SWRESAMPLE REQUIRED libswresample)
	set (SWRESAMPLE_LIBRARY_PATH ${SWRESAMPLE_LINK_LIBRARIES})
	set (SWRESAMPLE_INCLUDE_DIR ${SWRESAMPLE_INCLUDE_DIRS})
endif()

set (OPUS_INCLUDE_DIR "" CACHE PATH "Path to the include dir of OPUS lib")
set (OPUS_LIBRARY_PATH "" CACHE PATH "Path to the binary of OPUS lib")
if ("${OPUS_INCLUDE_DIR}" STREQUAL "" AND
	"${OPUS_LIBRARY_PATH}" STREQUAL "")
	find_package(PkgConfig REQUIRED)
	pkg_check_modules(OPUS REQUIRED opus)
	set (OPUS_LIBRARY_PATH ${OPUS_LINK_LIBRARIES})
	set (OPUS_INCLUDE_DIR ${OPUS_INCLUDE_DIRS})
endif()

target_link_libraries(
	"${LIB_NAME}" PRIVATE
	${SIMDJSON_LIBRARY_PATH}
	${OPENSSL_LIBRARY_PATH}
	${SODIUM_LIBRARY_PATH}
	${AVFORMAT_LIBRARY_PATH}
	${AVCODEC_LIBRARY_PATH}
	${AVUTIL_LIBRARY_PATH}
	${SWRESAMPLE_LIBRARY_PATH}
	${OPUS_LIBRARY_PATH}
	Threads::Threads
)

target_include_directories(
	"${LIB_NAME}" PUBLIC
	"$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>"
	PRIVATE
	"$<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/Include/>"
	${SIMDJSON_INCLUDE_DIR}
	${OPENSSL_INCLUDE_DIR}
	${SODIUM_INCLUDE_DIR}
	${AVFORMAT_INCLUDE_DIR}
	${AVCODEC_INCLUDE_DIR}
	${AVUTIL_INCLUDE_DIRS}
	${SWRESAMPLE_INCLUDE_DIR}
	${OPUS_INCLUDE_DIR}
)

target_compile_features(
	"${LIB_NAME}" PUBLIC
	"cxx_std_23"
)

if (DEFINED ENV{EnableASAN})
	if(MSVC)
		target_compile_options("${LIB_NAME}" PRIVATE "/fsanitize=address" "/Zi" "/Debug")
		target_link_options("${LIB_NAME}" PRIVATE "/incremental:no" "/Debug")
	else()
		target_compile_options("${LIB_NAME}" PRIVATE "-fsanitize=address")
		target_link_options("${LIB_NAME}" PRIVATE "-fsanitize=address")
	endif()
endif()

if ("${VCPKG_TARGET_TRIPLET}" STREQUAL "x64-uwp")
	target_compile_definitions("${LIB_NAME}" PUBLIC "UWP")
endif()

target_compile_definitions("${LIB_NAME}" PUBLIC "SIMDJSON_USING_WINDOWS_DYNAMIC_LIBRARY=1")

target_compile_options(
	"${LIB_NAME}" PRIVATE
	"$<$<NOT:$<PLATFORM_ID:Linux>>:/bigobj>"
	"$<$<PLATFORM_ID:Windows>:/MP>"
	"$<$<PLATFORM_ID:Linux>:-O3>"
	"$<$<PLATFORM_ID:Linux>:-fpermissive>"
)

target_link_options(
	"${LIB_NAME}" PUBLIC
	"$<$<PLATFORM_ID:Windows>:/DEBUG>"
)

set(CONFIG_FILE_NAME "${PROJECT_NAME}Config.cmake")
set(EXPORTED_TARGETS_NAME "${PROJECT_NAME}Targets")
set(EXPORTED_TARGETS_FILE_NAME "${EXPORTED_TARGETS_NAME}.cmake")
set(EXPORTED_TARGETS_FILE_PATH "share/discordcoreapi/${EXPORTED_TARGETS_FILE_NAME}")
set(RELEASE_PDB_FILE_PATH "bin/discordcoreapi.pdb")
set(DEBUG_PDB_FILE_PATH "debug/bin/discordcoreapi.pdb")

include(CMakePackageConfigHelpers)
configure_package_config_file(
	"${CMAKE_SOURCE_DIR}/CMake/${CONFIG_FILE_NAME}.in"
	"${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_FILE_NAME}"
	INSTALL_DESTINATION "share/discordcoreapi"
	PATH_VARS
	EXPORTED_TARGETS_FILE_PATH
	RELEASE_PDB_FILE_PATH
	DEBUG_PDB_FILE_PATH
)

set(VERSION_FILE_NAME "${PROJECT_NAME}ConfigVersion.cmake")

write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/${VERSION_FILE_NAME}"
	VERSION "${PRODUCT_VERSION}"
	COMPATIBILITY AnyNewerVersion
)

install(
	FILES
	"${CMAKE_CURRENT_BINARY_DIR}/${CONFIG_FILE_NAME}"
	"${CMAKE_CURRENT_BINARY_DIR}/${VERSION_FILE_NAME}"
	DESTINATION "share/discordcoreapi"
)

install(
	TARGETS "${LIB_NAME}"
	EXPORT "${EXPORTED_TARGETS_NAME}"
	RUNTIME DESTINATION "$<$<NOT:$<PLATFORM_ID:Linux>>:$<IF:$<CONFIG:Debug>,${DEBUG_PREFIX}bin,bin>>"
	ARCHIVE DESTINATION "$<IF:$<CONFIG:Debug>,${DEBUG_PREFIX}lib,lib>"
	PUBLIC_HEADER DESTINATION "include/discordcoreapi"
)

install(
	EXPORT "${EXPORTED_TARGETS_NAME}"
	FILE "${EXPORTED_TARGETS_FILE_NAME}"
	NAMESPACE "${PROJECT_NAME}::"
	DESTINATION "share/discordcoreapi"
)

if (WIN32)
	install(
		FILES 
		"$<TARGET_PDB_FILE:${LIB_NAME}>"
		DESTINATION "$<IF:$<CONFIG:Debug>,${DEBUG_PREFIX}bin,bin>"
		OPTIONAL
	)
endif()
