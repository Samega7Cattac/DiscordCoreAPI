#[=======================================================================[.rst:
FindFFMPEG
-------

Finds the FFMPEG library.

Imported Targets
^^^^^^^^^^^^^^^^

This module provides the following imported targets, if found:

``FFMPEG::AVCodec``
``FFMPEG::AVFormat``
``FFMPEG::AVUtil``
``FFMPEG::SWResample``
  The FFMPEG libraries.

Result Variables
^^^^^^^^^^^^^^^^

This will define the following variables:

``FFMPEG_FOUND``
  True if the system has the FFMPEG library.
``FFMPEG_INCLUDE_DIRS``
  Include directories needed to use FFMPEG.
``FFMPEG_LIBRARIES``
  Libraries needed to link to FFMPEG.

Cache Variables
^^^^^^^^^^^^^^^

The following cache variables may also be set:

``FFMPEG_INCLUDE_DIR``
  The directory containing ``ffmpeg.h``.
``FFMPEG_ROOT_DIR``
  The root path to the FFMPEG libraries.

#]=======================================================================]
include(FindPackageHandleStandardArgs)

find_path(FFMPEG_INCLUDE_DIR
  NAMES "ffmpeg/ffmpeg.h"
  PATHS ${FFMPEG_INCLUDE_DIR}
)

find_library(
	AVCODEC_RELEASE_LIBRARY 
	NAMES "${LIBRARY_PREFIX}avcodec${LIBRARY_SUFFIX}" 
	PATHS "${FFMPEG_ROOT_DIR}/release" "${FFMPEG_ROOT_DIR}/lib" NO_DEFAULT_PATH
)
find_library(
	AVCODEC_DEBUG_LIBRARY 
	NAMES "${LIBRARY_PREFIX}avcodec${LIBRARY_SUFFIX}" 
	PATHS "${FFMPEG_ROOT_DIR}/debug" "${FFMPEG_ROOT_DIR}/lib" NO_DEFAULT_PATH
)
find_library(
	AVFORMAT_RELEASE_LIBRARY 
	NAMES "${LIBRARY_PREFIX}avformat${LIBRARY_SUFFIX}"
	PATHS "${FFMPEG_ROOT_DIR}/release" "${FFMPEG_ROOT_DIR}/lib" NO_DEFAULT_PATH
)
find_library(
	AVFORMAT_DEBUG_LIBRARY 
	NAMES "${LIBRARY_PREFIX}avformat${LIBRARY_SUFFIX}"
	PATHS "${FFMPEG_ROOT_DIR}/debug" "${FFMPEG_ROOT_DIR}/lib" NO_DEFAULT_PATH
)
find_library(
	AVUTIL_RELEASE_LIBRARY 
	NAMES "${LIBRARY_PREFIX}avutil${LIBRARY_SUFFIX}" 
	PATHS "${FFMPEG_ROOT_DIR}/release" "${FFMPEG_ROOT_DIR}/lib" NO_DEFAULT_PATH
)
find_library(
	AVUTIL_DEBUG_LIBRARY 
	NAMES "${LIBRARY_PREFIX}avutil${LIBRARY_SUFFIX}"
	PATHS "${FFMPEG_ROOT_DIR}/debug" "${FFMPEG_ROOT_DIR}/lib" NO_DEFAULT_PATH
)
find_library(
	SWRESAMPLE_RELEASE_LIBRARY 
	NAMES "${LIBRARY_PREFIX}swresample${LIBRARY_SUFFIX}"
	PATHS "${FFMPEG_ROOT_DIR}/release" "${FFMPEG_ROOT_DIR}/lib" NO_DEFAULT_PATH
)
find_library(
	SWRESAMPLE_DEBUG_LIBRARY 
	NAMES "${LIBRARY_PREFIX}swresample${LIBRARY_SUFFIX}"
	PATHS "${FFMPEG_ROOT_DIR}/debug" "${FFMPEG_ROOT_DIR}/lib" NO_DEFAULT_PATH
)

cmake_path(GET AVCODEC_RELEASE_LIBRARY PARENT_PATH AVCODEC_RELEASE_FILE_PATH)
find_file(
	AVCODEC_RELEASE_DLL
	NAMES "avcodec-58.dll" "libavcodec-58.dll"
	PATHS "${AVCODEC_RELEASE_FILE_PATH}/" "${AVCODEC_RELEASE_FILE_PATH}/../bin/"
	NO_DEFAULT_PATH
)
cmake_path(GET AVCODEC_DEBUG_LIBRARY PARENT_PATH AVCODEC_DEBUG_FILE_PATH)
find_file(
	AVCODEC_DEBUG_DLL
	NAMES "avcodec-58.dll" "libavcodec-58.dll"
	PATHS "${AVCODEC_DEBUG_FILE_PATH}/" "${AVCODEC_DEBUG_FILE_PATH}/../bin/"
	NO_DEFAULT_PATH
)		
cmake_path(GET AVFORMAT_RELEASE_LIBRARY PARENT_PATH AVFORMAT_RELEASE_FILE_PATH)
find_file(
	AVFORMAT_RELEASE_DLL
	NAMES "avformat-58.dll" "libavformat-58.dll"
	PATHS "${AVFORMAT_RELEASE_FILE_PATH}/" "${AVFORMAT_RELEASE_FILE_PATH}/../bin/"
	NO_DEFAULT_PATH
)
cmake_path(GET AVFORMAT_DEBUG_LIBRARY PARENT_PATH AVFORMAT_DEBUG_FILE_PATH)
find_file(
	AVFORMAT_DEBUG_DLL
	NAMES "avformat-58.dll" "avformat-58.dll"
	PATHS "${AVFORMAT_DEBUG_FILE_PATH}/" "${AVFORMAT_DEBUG_FILE_PATH}/../bin/"
	NO_DEFAULT_PATH
)
cmake_path(GET AVUTIL_RELEASE_LIBRARY PARENT_PATH AVUTIL_RELEASE_FILE_PATH)
find_file(
	AVUTIL_RELEASE_DLL
	NAMES "avutil-56.dll" "libavutil-56.dll"
	PATHS "${AVUTIL_RELEASE_FILE_PATH}/" "${AVUTIL_RELEASE_FILE_PATH}/../bin/"
	NO_DEFAULT_PATH
)
cmake_path(GET AVUTIL_DEBUG_LIBRARY PARENT_PATH AVUTIL_DEBUG_FILE_PATH)
find_file(
	AVUTIL_DEBUG_DLL
	NAMES "avutil-56.dll" "libavutil-56.dll"
	PATHS "${AVUTIL_DEBUG_FILE_PATH}/" "${AVUTIL_DEBUG_FILE_PATH}/../bin/"
	NO_DEFAULT_PATH
)		
cmake_path(GET SWRESAMPLE_RELEASE_LIBRARY PARENT_PATH SWRESAMPLE_RELEASE_FILE_PATH)
find_file(
	SWRESAMPLE_RELEASE_DLL
	NAMES "swresample-3.dll" "libswresample-3.dll"
	PATHS "${SWRESAMPLE_RELEASE_FILE_PATH}/" "${SWRESAMPLE_RELEASE_FILE_PATH}/../bin/"
	NO_DEFAULT_PATH
)
cmake_path(GET SWRESAMPLE_DEBUG_LIBRARY PARENT_PATH SWRESAMPLE_DEBUG_FILE_PATH)
find_file(
	SWRESAMPLE_DEBUG_DLL
	NAMES "swresample-3.dll" "libswresample-3.dll"
	PATHS "${SWRESAMPLE_DEBUG_FILE_PATH}/" "${SWRESAMPLE_DEBUG_FILE_PATH}/../bin/"
	NO_DEFAULT_PATH
)
if (EXISTS "${AVCODEC_RELEASE_DLL}" AND EXISTS "${AVCODEC_DEBUG_DLL}" AND EXISTS "${AVFORMAT_RELEASE_DLL}" AND EXISTS "${AVFORMAT_DEBUG_DLL}"
	AND EXISTS "${AVUTIL_RELEASE_DLL}" AND EXISTS "${AVUTIL_DEBUG_DLL}" AND EXISTS "${SWRESAMPLE_RELEASE_DLL}" AND EXISTS "${SWRESAMPLE_DEBUG_DLL}")
	find_package_handle_standard_args(
		FFMPEG
		FOUND_VAR FFMPEG_FOUND
		REQUIRED_VARS
		AVCODEC_RELEASE_DLL AVCODEC_DEBUG_DLL AVCODEC_RELEASE_LIBRARY  AVCODEC_DEBUG_LIBRARY
		AVFORMAT_RELEASE_DLL AVFORMAT_DEBUG_DLL AVFORMAT_RELEASE_LIBRARY  AVFORMAT_DEBUG_LIBRARY
		AVUTIL_RELEASE_DLL AVUTIL_DEBUG_DLL AVUTIL_RELEASE_LIBRARY AVUTIL_DEBUG_LIBRARY
		SWRESAMPLE_RELEASE_DLL SWRESAMPLE_DEBUG_DLL SWRESAMPLE_RELEASE_LIBRARY SWRESAMPLE_DEBUG_LIBRARY
		FFMPEG_INCLUDE_DIR
	)
	if (FFMPEG_FOUND)
		set(FFMPEG_LIBRARIES "${AVCODEC_RELEASE_LIBRARY}" "${AVCODEC_DEBUG_LIBRARY}" "${AVCODEC_RELEASE_DLL}" "${AVCODEC_DEBUG_DLL}"
			"${AVFORMAT_RELEASE_LIBRARY}" "${AVFORMAT_DEBUG_LIBRARY}" "${AVFORMAT_RELEASE_DLL}" "${AVFORMAT_DEBUG_DLL}"
			"${AVUTIL_RELEASE_LIBRARY}" "${AVUTIL_DEBUG_LIBRARY}" "${AVUTIL_RELEASE_DLL}" "${AVUTIL_DEBUG_DLL}"
			"${SWRESAMPLE_RELEASE_LIBRARY}" "${SWRESAMPLE_DEBUG_LIBRARY}" "${SWRESAMPLE_RELEASE_DLL}" "${SWRESAMPLE_DEBUG_DLL}"
		)
		set(FFMPEG_INCLUDE_DIRS "${FFMPEG_INCLUDE_DIR}")
		add_library(FFMPEG::AVCodec SHARED IMPORTED GLOBAL)
		set_target_properties(
			FFMPEG::AVCodec PROPERTIES 
			IMPORTED_LOCATION_RELEASE "${AVCODEC_RELEASE_DLL}" IMPORTED_LOCATION_DEBUG "${AVCODEC_DEBUG_DLL}"
			IMPORTED_IMPLIB_RELEASE "${AVCODEC_RELEASE_LIBRARY}" IMPORTED_IMPLIB_DEBUG "${AVCODEC_DEBUG_LIBRARY}"
		)
		target_include_directories(FFMPEG::AVCodec INTERFACE "${INCLUDE_DIR}")
		add_library(FFMPEG::AVFormat SHARED IMPORTED GLOBAL)
		set_target_properties(
			FFMPEG::AVFormat PROPERTIES 
			IMPORTED_LOCATION_RELEASE "${AVFORMAT_RELEASE_DLL}" IMPORTED_LOCATION_DEBUG "${AVFORMAT_DEBUG_DLL}"
			IMPORTED_IMPLIB_RELEASE "${AVFORMAT_RELEASE_LIBRARY}" IMPORTED_IMPLIB_DEBUG "${AVFORMAT_DEBUG_LIBRARY}"
		)
		target_include_directories(FFMPEG::AVFormat INTERFACE "${INCLUDE_DIR}")
		add_library(FFMPEG::AVUtil SHARED IMPORTED GLOBAL)
		set_target_properties(
			FFMPEG::AVUtil PROPERTIES 
			IMPORTED_LOCATION_RELEASE "${AVUTIL_RELEASE_DLL}" IMPORTED_LOCATION_DEBUG "${AVUTIL_DEBUG_DLL}"
			IMPORTED_IMPLIB_RELEASE "${AVUTIL_RELEASE_LIBRARY}" IMPORTED_IMPLIB_DEBUG "${AVUTIL_DEBUG_LIBRARY}"
		)
		target_include_directories(FFMPEG::AVUtil INTERFACE "${INCLUDE_DIR}")
		add_library(FFMPEG::SWResample SHARED IMPORTED GLOBAL)
		set_target_properties(
			FFMPEG::SWResample PROPERTIES 
			IMPORTED_LOCATION_RELEASE "${SWRESAMPLE_RELEASE_DLL}" IMPORTED_LOCATION_DEBUG "${SWRESAMPLE_DEBUG_DLL}"
			IMPORTED_IMPLIB_RELEASE "${SWRESAMPLE_RELEASE_LIBRARY}" IMPORTED_IMPLIB_DEBUG "${SWRESAMPLE_DEBUG_LIBRARY}"
		)
		target_include_directories(FFMPEG::SWResample INTERFACE "${INCLUDE_DIR}")
	endif()
else()
	find_package_handle_standard_args(
		FFMPEG
		FOUND_VAR FFMPEG_FOUND
		REQUIRED_VARS
		FFMPEG_INCLUDE_DIR
		AVCODEC_RELEASE_LIBRARY  AVCODEC_DEBUG_LIBRARY
		AVFORMAT_RELEASE_LIBRARY  AVFORMAT_DEBUG_LIBRARY
		AVUTIL_RELEASE_LIBRARY AVUTIL_DEBUG_LIBRARY
		SWRESAMPLE_RELEASE_LIBRARY SWRESAMPLE_DEBUG_LIBRARY
	)
	if (FFMPEG_FOUND)
		add_library(FFMPEG::AVCodec STATIC IMPORTED GLOBAL)
		set_target_properties(
			FFMPEG::AVCodec PROPERTIES 
			IMPORTED_LOCATION_RELEASE "${AVCODEC_RELEASE_LIBRARY}" IMPORTED_LOCATION_DEBUG "${AVCODEC_DEBUG_LIBRARY}"
		)
		target_include_directories(FFMPEG::AVCodec INTERFACE "${INCLUDE_DIR}")
		add_library(FFMPEG::AVFormat STATIC IMPORTED GLOBAL)
		set_target_properties(
			FFMPEG::AVFormat PROPERTIES 
			IMPORTED_LOCATION_RELEASE "${AVFORMAT_RELEASE_LIBRARY}" IMPORTED_LOCATION_DEBUG "${AVFORMAT_DEBUG_LIBRARY}"
		)
		target_include_directories(FFMPEG::AVFormat INTERFACE "${INCLUDE_DIR}")
		add_library(FFMPEG::AVUtil STATIC IMPORTED GLOBAL)
		set_target_properties(
			FFMPEG::AVUtil PROPERTIES 
			IMPORTED_LOCATION_RELEASE "${AVUTIL_RELEASE_LIBRARY}" IMPORTED_LOCATION_DEBUG "${AVUTIL_DEBUG_LIBRARY}"
		)
		target_include_directories(FFMPEG::AVUtil INTERFACE "${INCLUDE_DIR}")
		add_library(FFMPEG::SWResample STATIC IMPORTED GLOBAL)
		set_target_properties(
			FFMPEG::SWResample PROPERTIES 
			IMPORTED_LOCATION_RELEASE "${SWRESAMPLE_RELEASE_LIBRARY}" IMPORTED_LOCATION_DEBUG "${SWRESAMPLE_DEBUG_LIBRARY}"
		)
		target_include_directories(FFMPEG::SWResample INTERFACE "${INCLUDE_DIR}")
		unset(AVCODEC_RELEASE_DLL CACHE)
		unset(AVCODEC_DEBUG_DLL CACHE)
		unset(AVFORMAT_RELEASE_DLL CACHE)
		unset(AVFORMAT_DEBUG_DLL CACHE)
		unset(AVUTIL_RELEASE_DLL CACHE)
		unset(AVUTIL_DEBUG_DLL CACHE)
		unset(SWRESAMPLE_RELEASE_DLL CACHE)
		unset(SWRESAMPLE_DEBUG_DLL CACHE)
		message(STATUS "Couldn't find FFMPEG Dlls - linking statically!")
	endif()
endif()