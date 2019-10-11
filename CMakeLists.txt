cmake_minimum_required(VERSION 3.2)

# TODO: SDK Versioning.
project(ggpo VERSION 1.0.0)

# What do we want to build?
option(BUILD_GGPO "Enable the build of the GGPO SDK" ON)
option(BUILD_VECTORWAR "Enable the build of the Vector War example app" ON)
option(BUILD_SHARED_LIBS "Enable the build of shared libraries (.dll/.so) instead of static ones (.lib/.a)" ON)

set(GGPO_SDK
	src/include/ggponet.h
	src/lib/ggpo/main.cpp
	src/lib/ggpo/bitvector.h
	src/lib/ggpo/bitvector.cpp
	src/lib/ggpo/game_input.h
	src/lib/ggpo/game_input.cpp
	src/lib/ggpo/input_queue.h
	src/lib/ggpo/input_queue.cpp
	src/lib/ggpo/log.h
	src/lib/ggpo/log.cpp
	src/lib/ggpo/poll.h
	src/lib/ggpo/poll.cpp
	src/lib/ggpo/ring_buffer.h
	src/lib/ggpo/static_buffer.h
	src/lib/ggpo/sync.h
	src/lib/ggpo/sync.cpp
	src/lib/ggpo/timesync.h
	src/lib/ggpo/timesync.cpp
	src/lib/ggpo/types.h
	src/lib/ggpo/zconf.h
	src/lib/ggpo/zlib.h
	src/lib/ggpo/backends/backend.h
	src/lib/ggpo/backends/p2p.h
	src/lib/ggpo/backends/p2p.cpp
	src/lib/ggpo/backends/spectator.h
	src/lib/ggpo/backends/spectator.cpp
	src/lib/ggpo/backends/synctest.h
	src/lib/ggpo/backends/synctest.cpp
	src/lib/ggpo/network/udp.h
	src/lib/ggpo/network/udp.cpp
	src/lib/ggpo/network/udp_msg.h
	src/lib/ggpo/network/udp_proto.h
	src/lib/ggpo/network/udp_proto.cpp
)

if(BUILD_GGPO)
	add_library(ggpo ${GGPO_SDK})
	
	target_include_directories(ggpo PUBLIC src/include)
	target_include_directories(ggpo PUBLIC src/lib/ggpo)
	
	if(WIN32 AND BUILD_SHARED_LIBS)
		# Link to Multimedia API and Winsocks during a static build.
		target_link_libraries(ggpo LINK_PUBLIC winmm.lib ws2_32.lib)
	endif()
	
	set_target_properties(ggpo PROPERTIES VERSION ${PROJECT_VERSION})
endif()

if(BUILD_VECTORWAR)
	# Vector War is windows only.
	if(WIN32)
		add_subdirectory(src/apps/vectorwar)
	else()
		message(WARNING "The Vector War app only supports windows, skipping...")
	endif()
endif()