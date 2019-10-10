## Based on https://github.com/GameFoundry/bsf/blob/master/Source/CMake/Properties.cmake
## which is covered under the MIT License, same as GGPO.

include(CheckCXXCompilerFlag)

# Configuration types
if(NOT CMAKE_CONFIGURATION_TYPES) # Multiconfig generator?
	if(NOT CMAKE_BUILD_TYPE)
		message(STATUS "Defaulting to release build.")
		set_property(CACHE CMAKE_BUILD_TYPE PROPERTY VALUE "Release")
	endif()
endif()

if(${CMAKE_SIZEOF_VOID_P} EQUAL 8)
	set(GGPO_64BIT true)
endif()

set(CMAKE_CXX_STANDARD 98)
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)

# Enable colored output
if (CMAKE_GENERATOR STREQUAL "Ninja")
	check_cxx_compiler_flag("-fdiagnostics-color=always" F_DIAGNOSTIC_COLOR_ALWAYS)
	if (F_DIAGNOSTIC_COLOR_ALWAYS)
		add_compile_options("-fdiagnostics-color=always")
	endif()
endif()

# Output
if(GGPO_64BIT)
	set(GGPO_OUTPUT_DIR_PREFIX x64)
else()
	set(GGPO_OUTPUT_DIR_PREFIX x86)
endif()

set(GGPO_BINARY_OUTPUT_DIR ${PROJECT_BINARY_DIR}/bin/${GGPO_OUTPUT_DIR_PREFIX})
set(GGPO_LIBRARY_OUTPUT_DIR ${PROJECT_BINARY_DIR}/lib/${GGPO_OUTPUT_DIR_PREFIX})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${GGPO_BINARY_OUTPUT_DIR}/Debug)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${GGPO_BINARY_OUTPUT_DIR}/Release)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${GGPO_BINARY_OUTPUT_DIR}/Debug)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${GGPO_BINARY_OUTPUT_DIR}/Release)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${GGPO_LIBRARY_OUTPUT_DIR}/Debug)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${GGPO_LIBRARY_OUTPUT_DIR}/Release)

set_property(GLOBAL PROPERTY USE_FOLDERS TRUE)