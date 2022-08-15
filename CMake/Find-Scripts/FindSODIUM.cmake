#
#	DiscordCoreAPI, A bot library for Discord, written in C++, and featuring explicit multithreading through the usage of custom, asynchronous C++ CoRoutines.
#
#	Copyright 2021, 2022 Chris M. (RealTimeChris)
#
#	This file is part of DiscordCoreAPI.
#	DiscordCoreAPI is free software: you can redistribute it and/or modify it under the terms of the GNU
#	General Public License as published by the Free Software Foundation, either version 3 of the License,
#	or (at your option) any later version.
#	DiscordCoreAPI is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without
#	even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
#	You should have received a copy of the GNU General Public License along with DiscordCoreAPI.
#	If not, see <https://www.gnu.org/licenses/>.
#
# FindSODIUM.cmake - This file locates the Sodium library, using a couple of provided paths for searching.
# May 13, 2021
# https://discordcoreapi.com

# Usage:
#	Set the following directories: SODIUM_RELEASE_ROOT, SODIUM_DEBUG_ROOT, SODIUM_INCLUDE_DIR
# Where:
#	SODIUM_RELEASE_ROOT = The directory containing the RELEASE version of the library, or library's linker file.
#	SODIUM_DEBUG_ROOT = The directory containing the DEBUG version of the library, or library's linker file.
#	SODIUM_INCLUDE_DIR = The directory containing the public headers.
# What it produces:
#	SODIUM::Sodium
if (UNIX)
	set(LIB_SUFFIX ".a")
	set(LIB_PREFIX "lib")
else()
	set(LIB_SUFFIX ".lib")
	set(LIB_PREFIX "")
endif()
find_library(
	SODIUM_DEBUG_LIBRARY 
	NAMES "libsodium${LIB_SUFFIX}" 
	PATHS "${SODIUM_DEBUG_ROOT}"
	NO_DEFAULT_PATH
)
find_library(
	SODIUM_RELEASE_LIBRARY 
	NAMES "libsodium${LIB_SUFFIX}" 
	PATHS "${SODIUM_RELEASE_ROOT}"
	NO_DEFAULT_PATH
)
if (EXISTS "${SODIUM_DEBUG_LIBRARY}" AND EXISTS "${SODIUM_RELEASE_LIBRARY}" AND EXISTS "${SODIUM_INCLUDE_DIR}")
	message(STATUS "Found Sodium: TRUE")
else()
	message(FATAL_ERROR "Found Sodium: FALSE")
endif()
cmake_path(GET SODIUM_DEBUG_LIBRARY PARENT_PATH SODIUM_DEBUG_FILE_PATH)
find_file(
	SODIUM_DEBUG_DLL
	NAMES "libsodium.dll" "sodium.dll" 
	PATHS "${SODIUM_DEBUG_FILE_PATH}/" "${SODIUM_DEBUG_FILE_PATH}/../bin/"
	NO_DEFAULT_PATH
)
cmake_path(GET SODIUM_RELEASE_LIBRARY PARENT_PATH SODIUM_RELEASE_FILE_PATH)
find_file(
	SODIUM_RELEASE_DLL
	NAMES "libsodium.dll" "sodium.dll" 
	PATHS "${SODIUM_RELEASE_FILE_PATH}/" "${SODIUM_RELEASE_FILE_PATH}/../bin/"
	NO_DEFAULT_PATH
)
if (EXISTS "${SODIUM_DEBUG_DLL}" AND EXISTS "${SODIUM_RELEASE_DLL}")
	add_library(SODIUM::Sodium SHARED IMPORTED GLOBAL)
	set_target_properties(
		SODIUM::Sodium PROPERTIES 
		IMPORTED_LOCATION_DEBUG "${SODIUM_DEBUG_DLL}" IMPORTED_LOCATION_RELEASE "${SODIUM_RELEASE_DLL}"
		IMPORTED_IMPLIB_DEBUG "${SODIUM_DEBUG_LIBRARY}" IMPORTED_IMPLIB_RELEASE "${SODIUM_RELEASE_LIBRARY}"
	)
	target_include_directories(SODIUM::Sodium INTERFACE "${SODIUM_INCLUDE_DIR}")
	message(STATUS "Found Sodium Dlls: TRUE")
else()
	add_library(SODIUM::Sodium STATIC IMPORTED GLOBAL)
	set_target_properties(
		SODIUM::Sodium PROPERTIES 
		IMPORTED_LOCATION_DEBUG "${SODIUM_DEBUG_LIBRARY}" IMPORTED_LOCATION_RELEASE "${SODIUM_RELEASE_LIBRARY}"
	)
	target_include_directories(SODIUM::Sodium INTERFACE "${SODIUM_INCLUDE_DIR}")
	unset(SODIUM_DEBUG_DLL CACHE)
	unset(SODIUM_RELEASE_DLL CACHE)
	message(STATUS "Found Sodium Dlls: FALSE - linking statically")
endif()