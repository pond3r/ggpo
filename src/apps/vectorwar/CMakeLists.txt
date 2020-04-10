include(CMakeSources.cmake)

add_executable(VectorWar WIN32 
	${GGPO_EXAMPLES_VECTORWAR_SRC}
)

if(WIN32 AND BUILD_SHARED_LIBS)
	add_definitions(-DGGPO_SHARED_LIB)
endif()

add_common_flags(VectorWar)
# Change the character set to unicode.
add_definitions(-D_UNICODE -DUNICODE)

set_target_properties(VectorWar PROPERTIES
	CXX_STANDARD 11
	CXX_STANDARD_REQUIRED YES
	CXX_EXTENSIONS NO
)

# Link against GGPO, libSDL2, winmm (Windows Multimedia API), and ws2_32
# (Winsock).
if(WIN32 AND BUILD_SHARED_LIBS)
	target_link_libraries(VectorWar LINK_PUBLIC GGPO SDL2 SDL2main winmm ws2_32)
else()
	target_link_libraries(VectorWar LINK_PUBLIC GGPO SDL2)
endif()
