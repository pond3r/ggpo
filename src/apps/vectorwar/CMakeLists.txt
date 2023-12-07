include(CMakeSources.cmake)

set(STEAMWORKS_PATH "${CMAKE_SOURCE_DIR}/external/steamworks/sdk" CACHE STRING "Path to Steamworks SDK")

add_executable(VectorWar WIN32 
	${GGPO_EXAMPLES_VECTORWAR_SRC}
)

target_include_directories(VectorWar PUBLIC
    ${STEAMWORKS_PATH}/public/steam
)

if(WIN32 AND BUILD_SHARED_LIBS)
	add_definitions(-DGGPO_SHARED_LIB)
endif()

add_common_flags(VectorWar)
# Change the character set to unicode.
add_definitions(-D_UNICODE -DUNICODE)

# Link against GGPO, winmm (Windows Multimedia API), and ws2_32 (Winsock).
target_link_libraries(VectorWar LINK_PUBLIC GGPO winmm.lib ws2_32.lib "${STEAMWORKS_PATH}/redistributable_bin/win64/steam_api64.lib")
target_compile_options(VectorWar PRIVATE "/W4" "/WX")
