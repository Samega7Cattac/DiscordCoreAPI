# find_openssl(RELEASE_ROOT_DIR DEBUG_ROOT_DIR)
#
# This function locates the Libz library, using a couple of provided paths for searching.
#
# Usage:
#	find_openssl(RELEASE_ROOT_DIR DEBUG_ROOT_DIR)
# Where:
#	RELEASE_ROOT_DIR = The directory containing the RELEASE version of the library, or library's linker file.
#	DEBUG_ROOT_DIR = The directory containing the DEBUG version of the library, or library's linker file.
# What it produces:
#	LIBSSL_RELEASE_LIBRARY, LIBSSL_DEBUG_LIBRARY and in the case of SHARED LIBRARIES - 
#	LIBSSL_RELEASE_DLL, and LIBSSL_DEBUG_DLL. Which each points to the respective files of each kind. 
#	LIBCRYPTO_RELEASE_LIBRARY, LIBCRYPTO_DEBUG_LIBRARY and in the case of SHARED LIBRARIES - 
#	LIBCRYPTO_RELEASE_DLL, and LIBCRYPTO_DEBUG_DLL. Which each points to the respective files of each kind. 
#
function(find_openssl RELEASE_ROOT_DIR DEBUG_ROOT_DIR)
	find_library(
		LIBCRYPTO_RELEASE_LIBRARY 
		NAMES "libcrypto${LIBRARY_SUFFIX}" 
		PATHS "${RELEASE_ROOT_DIR}" NO_DEFAULT_PATH
	)
	find_library(
		LIBCRYPTO_DEBUG_LIBRARY
		NAMES "libcrypto${LIBRARY_SUFFIX}"
		PATHS "${DEBUG_ROOT_DIR}" NO_DEFAULT_PATH
	)
	find_library(
		LIBSSL_RELEASE_LIBRARY 
		NAMES "libssl${LIBRARY_SUFFIX}" 
		PATHS "${RELEASE_ROOT_DIR}" NO_DEFAULT_PATH
	)
	find_library(
		LIBSSL_DEBUG_LIBRARY 
		NAMES "libssl${LIBRARY_SUFFIX}" 
		PATHS "${DEBUG_ROOT_DIR}" NO_DEFAULT_PATH
	)
	if (LIBSSL_RELEASE_LIBRARY AND LIBSSL_DEBUG_LIBRARY AND LIBCRYPTO_RELEASE_LIBRARY AND LIBCRYPTO_DEBUG_LIBRARY)
		cmake_path(GET LIBSSL_RELEASE_LIBRARY FILENAME LIBSSL_RELEASE_LIBRARY_NAME)
		cmake_path(GET LIBSSL_RELEASE_LIBRARY PARENT_PATH LIBSSL_RELEASE_FILE_PATH)
		cmake_path(RELATIVE_PATH LIBSSL_RELEASE_FILE_PATH OUTPUT_VARIABLE LIBSSL_RELEASE_LIBRARY_NEW)
		cmake_path(GET LIBSSL_DEBUG_LIBRARY FILENAME LIBSSL_DEBUG_LIBRARY_NAME)
		cmake_path(GET LIBSSL_DEBUG_LIBRARY PARENT_PATH LIBSSL_DEBUG_FILE_PATH)
		cmake_path(RELATIVE_PATH LIBSSL_DEBUG_FILE_PATH OUTPUT_VARIABLE LIBSSL_DEBUG_LIBRARY_NEW)
		list(APPEND RELEASE_LIBRARIES "${LIBSSL_RELEASE_LIBRARY_NEW}/${LIBSSL_RELEASE_LIBRARY_NAME}")
		list(APPEND DEBUG_LIBRARIES "${LIBSSL_DEBUG_LIBRARY_NEW}/${LIBSSL_DEBUG_LIBRARY_NAME}")
		cmake_path(GET LIBCRYPTO_RELEASE_LIBRARY FILENAME LIBCRYPTO_RELEASE_LIBRARY_NAME)
		cmake_path(GET LIBCRYPTO_RELEASE_LIBRARY PARENT_PATH LIBCRYPTO_RELEASE_FILE_PATH)
		cmake_path(RELATIVE_PATH LIBCRYPTO_RELEASE_FILE_PATH OUTPUT_VARIABLE LIBCRYPTO_RELEASE_LIBRARY_NEW)
		cmake_path(GET LIBCRYPTO_DEBUG_LIBRARY FILENAME LIBCRYPTO_DEBUG_LIBRARY_NAME)
		cmake_path(GET LIBCRYPTO_DEBUG_LIBRARY PARENT_PATH LIBCRYPTO_DEBUG_FILE_PATH)
		cmake_path(RELATIVE_PATH LIBCRYPTO_DEBUG_FILE_PATH OUTPUT_VARIABLE LIBCRYPTO_DEBUG_LIBRARY_NEW)
		list(APPEND RELEASE_LIBRARIES "${LIBCRYPTO_RELEASE_LIBRARY_NEW}/${LIBCRYPTO_RELEASE_LIBRARY_NAME}")
		list(APPEND DEBUG_LIBRARIES "${LIBCRYPTO_DEBUG_LIBRARY_NEW}/${LIBCRYPTO_DEBUG_LIBRARY_NAME}")
		set(RELEASE_LIBRARIES "${RELEASE_LIBRARIES}" PARENT_SCOPE)
		set(DEBUG_LIBRARIES  "${DEBUG_LIBRARIES}" PARENT_SCOPE)
		message(STATUS "Found OpenSSL libraries!")
	else()
		message(FATAL_ERROR "Couldn't find OpenSSL!")
		return()
	endif()
	if (WIN32)
		cmake_path(GET LIBSSL_RELEASE_LIBRARY PARENT_PATH LIBSSL_RELEASE_FILE_PATH)
		find_file(
			LIBSSL_RELEASE_DLL
			NAMES "libssl-3-x64.dll" 
			PATHS "${LIBSSL_RELEASE_FILE_PATH}/" "${LIBSSL_RELEASE_FILE_PATH}/../bin/"
			NO_DEFAULT_PATH
		)
		cmake_path(GET LIBSSL_DEBUG_LIBRARY PARENT_PATH LIBSSL_DEBUG_FILE_PATH)
		find_file(
			LIBSSL_DEBUG_DLL
			NAMES "libssl-3-x64.dll"
			PATHS "${LIBSSL_DEBUG_FILE_PATH}/" "${LIBSSL_DEBUG_FILE_PATH}/../bin/"
			NO_DEFAULT_PATH
		)
		cmake_path(GET LIBCRYPTO_RELEASE_LIBRARY PARENT_PATH LIBCRYPTO_RELEASE_FILE_PATH)
		find_file(
			LIBCRYPTO_RELEASE_DLL
			NAMES "libcrypto-3-x64.dll" 
			PATHS "${LIBCRYPTO_RELEASE_FILE_PATH}/" "${LIBCRYPTO_RELEASE_FILE_PATH}/../bin/"
			NO_DEFAULT_PATH
		)
		cmake_path(GET LIBCRYPTO_DEBUG_LIBRARY PARENT_PATH LIBCRYPTO_DEBUG_FILE_PATH)
		find_file(
			LIBCRYPTO_DEBUG_DLL
			NAMES "libcrypto-3-x64.dll"
			PATHS "${LIBCRYPTO_DEBUG_FILE_PATH}/" "${LIBCRYPTO_DEBUG_FILE_PATH}/../bin/"
			NO_DEFAULT_PATH
		)
		if (LIBSSL_RELEASE_DLL AND LIBSSL_DEBUG_DLL AND LIBCRYPTO_RELEASE_DLL AND LIBCRYPTO_DEBUG_DLL)
			list(APPEND LIBRARY_NAMES "LIBSSL")
			list(APPEND LIBRARY_NAMES "LIBCRYPTO")
			set(LIBRARY_NAMES "${LIBRARY_NAMES}" PARENT_SCOPE)
			list(APPEND RELEASE_DLLS "${LIBSSL_RELEASE_DLL}")
			list(APPEND DEBUG_DLLS  "${LIBSSL_DEBUG_DLL}")
			list(APPEND RELEASE_DLLS "${LIBCRYPTO_RELEASE_DLL}")
			list(APPEND DEBUG_DLLS  "${LIBCRYPTO_DEBUG_DLL}")
			set(RELEASE_DLLS "${RELEASE_DLLS}" PARENT_SCOPE)
			set(DEBUG_DLLS  "${DEBUG_DLLS}" PARENT_SCOPE)
			message(STATUS "Found OpenSSL Dlls!")
		else()
			list(APPEND LIBRARY_NAMES "LIBSSL")
			list(APPEND LIBRARY_NAMES "LIBCRYPTO")
			set(LIBRARY_NAMES "${LIBRARY_NAMES}" PARENT_SCOPE)
			list(APPEND RELEASE_DLLS "NOTFOUND")
			list(APPEND DEBUG_DLLS  "NOTFOUND")
			list(APPEND RELEASE_DLLS "NOTFOUND")
			list(APPEND DEBUG_DLLS  "NOTFOUND")
			set(RELEASE_DLLS "${RELEASE_DLLS}" PARENT_SCOPE)
			set(DEBUG_DLLS  "${DEBUG_DLLS}" PARENT_SCOPE)
			unset(LIBCRYPTO_RELEASE_DLL CACHE)
			unset(LIBCRYPTO_DEBUG_DLL CACHE)
			unset(LIBSSL_RELEASE_DLL CACHE)
			unset(LIBSSL_DEBUG_DLL CACHE)
			message(STATUS "Couldn't find OpenSSL Dlls - linking statically!")
		endif()
	endif()
endfunction()