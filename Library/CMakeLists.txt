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
	VERSION_MINOR "2"
	VERSION_PATCH "0"
	VERSION_REVISION "0"
)

if (UNIX)
	set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG -Ofast")
	set(CMAKE_CXX_FLAGS_DEBUG "-DDEBUG -O1")
endif()

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
find_package(PkgConfig REQUIRED)

pkg_check_modules(SIMDJSON REQUIRED simdjson)
target_compile_definitions(${LIB_NAME}
	PRIVATE
		SIMDJSON_THREADS_ENABLED)

pkg_check_modules(OPENSSL REQUIRED openssl)
pkg_check_modules(AVFORMAT REQUIRED libavformat)
pkg_check_modules(AVCODEC REQUIRED libavcodec)
pkg_check_modules(AVUTIL REQUIRED libavutil)
pkg_check_modules(SWRESAMPLE REQUIRED libswresample)
pkg_check_modules(OPUS REQUIRED opus)
pkg_check_modules(SODIUM REQUIRED libsodium)

target_include_directories(
	"${LIB_NAME}" PUBLIC
	"$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/include>"
	PRIVATE
		"../Include"
		${SIMDJSON_INCLUDE_DIR}
		${OPENSSL_INCLUDE_DIR}
		${SODIUM_INCLUDE_DIR}
		${AVFORMAT_INCLUDE_DIR}
		${AVCODEC_INCLUDE_DIR}
		${AVUTIL_INCLUDE_DIRS}
		${SWRESAMPLE_INCLUDE_DIR}
		${OPUS_INCLUDE_DIR}
)

target_link_libraries(
	"${LIB_NAME}" PUBLIC
		${SIMDJSON_LIBRARY_PATH}
		${OPENSSL_LIBRARY_PATH}
		${SODIUM_LIBRARY_PATH}
		${AVFORMAT_LIBRARY_PATH}
		${AVCODEC_LIBRARY_PATH}
		${AVUTIL_LIBRARY_PATH}
		${SWRESAMPLE_LIBRARY_PATH}
		${OPUS_LIBRARY_PATH}
	)

target_compile_features(
	"${LIB_NAME}" PUBLIC
	"cxx_std_23"
)

target_compile_definitions(
	"${LIB_NAME}" PUBLIC 
	"$<$<STREQUAL:${VCPKG_TARGET_TRIPLET},x64-uwp>:UWP>"
	"$<$<CXX_COMPILER_ID:MSVC>:SIMDJSON_USING_WINDOWS_DYNAMIC_LIBRARY=1>"
)

target_compile_options(
	"${LIB_NAME}" PUBLIC
	"$<$<NOT:$<PLATFORM_ID:Linux>>:/bigobj>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:$ENV{EnableASAN},true>:/fsanitize=address>>"
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:$ENV{EnableASAN},true>:/Zi>>"
	"$<$<CXX_COMPILER_ID:MSVC>:/MP>"
	"$<$<CXX_COMPILER_ID:MSVC>:/arch:AVX2>"
	"$<$<CXX_COMPILER_ID:GNU>:$<$<STREQUAL:$ENV{EnableASAN},true>:-fsanitize=address>>"
	"$<$<CXX_COMPILER_ID:GNU>:-fpermissive>"
	"$<$<CXX_COMPILER_ID:GNU>:-mavx512f>"
	"$<$<CXX_COMPILER_ID:GNU>:-march=native>"
)

target_link_options(
	"${LIB_NAME}" PUBLIC
	"$<$<CXX_COMPILER_ID:MSVC>:$<$<STREQUAL:$ENV{EnableASAN},true>:/Debug>>"
	"$<$<CXX_COMPILER_ID:MSVC>:/DEBUG>"
	"$<$<CXX_COMPILER_ID:GNU>:$<$<STREQUAL:$ENV{EnableASAN},true>:-fsanitize=address>>"
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
