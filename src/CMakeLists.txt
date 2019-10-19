include(CMakeSources.cmake)

add_library(GGPO
	${GGPO_LIB_SRC}
)

add_common_flags(GGPO)

target_include_directories(GGPO PUBLIC
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/ggpo>
)

if(WIN32 AND BUILD_SHARED_LIBS)
	# Link to Multimedia API and Winsocks during a shared build.
	target_link_libraries(GGPO PUBLIC winmm.lib ws2_32.lib)
endif()

set_target_properties(GGPO PROPERTIES VERSION ${PROJECT_VERSION})

# Install
install(
	DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include
	DESTINATION ./
	FILES_MATCHING
	PATTERN "*.h"
	PATTERN "*.hpp"
)

install(
	TARGETS GGPO
	EXPORT ggpo
	RUNTIME DESTINATION bin
	LIBRARY DESTINATION lib
	ARCHIVE DESTINATION lib
)

if(MSVC)
	if(BUILD_SHARED_LIBS)
		install(
			FILES $<TARGET_PDB_FILE:GGPO>
			DESTINATION lib
			OPTIONAL
		)
	else()
		## NOTE: This is rather hacky, since CMake doesn't allow to install the PDB files for
		##	     static libraries, so we get the debug PDB file and copy it there.
		install(
			FILES "${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}/GGPO.pdb"
			DESTINATION lib
			OPTIONAL
		)
	endif()
else()
## TODO place the symbols install when GGPO became cross-platform
endif()