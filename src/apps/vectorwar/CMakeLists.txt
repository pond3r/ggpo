set(VECTORWAR_APP
	main.cpp
	gamestate.h
	gamestate.cpp
	gdi_renderer.h
	gdi_renderer.cpp
	ggpo_perfmon.h
	ggpo_perfmon.cpp
	nongamestate.h
	renderer.h
	Resource.h
	targetver.h
	vectorwar.h
	vectorwar.cpp
	VectorWar.rc
)

add_executable(vectorwar WIN32 ${VECTORWAR_APP})

# Change the character set to unicode.
add_definitions(-D_UNICODE -DUNICODE)

# Link against GGPO, winmm (Windows Multimedia API), and ws2_32 (Winsock).
target_link_libraries(vectorwar LINK_PUBLIC ggpo winmm.lib ws2_32.lib)