set(GGPO_EXAMPLES_VECTORWAR_INC_NOFILTER
	"font.h"
	"gamestate.h"
	"sdl_renderer.h"
	# "ggpo_perfmon.h"
	"nongamestate.h"
	"Resource.h"
	"targetver.h"
	"vectorwar.h"
	"platform_helpers.h"
)

set(GGPO_EXAMPLES_VECTORWAR_SRC_NOFILTER
	"gamestate.cpp"
	"sdl_renderer.cpp"
	# "ggpo_perfmon.cpp"
	"main.cpp"
	"vectorwar.cpp"
)

if(WIN32)
	set(GGPO_EXAMPLES_VECTORWAR_SRC_NOFILTER ${GGPO_EXAMPLES_VECTORWAR_SRC_NOFILTER} "platform_helpers_windows.cpp")
else()
	set(GGPO_EXAMPLES_VECTORWAR_SRC_NOFILTER ${GGPO_EXAMPLES_VECTORWAR_SRC_NOFILTER} "platform_helpers_unix.cpp")
endif()

set(GGPO_EXAMPLES_VECTORWAR_WIN32RES
	"VectorWar.rc"
)

source_group(" " FILES ${GGPO_EXAMPLES_VECTORWAR_INC_NOFILTER} ${GGPO_EXAMPLES_VECTORWAR_SRC_NOFILTER} ${GGPO_EXAMPLES_VECTORWAR_WIN32RES})

set(GGPO_EXAMPLES_VECTORWAR_SRC
	${GGPO_EXAMPLES_VECTORWAR_INC_NOFILTER}
	${GGPO_EXAMPLES_VECTORWAR_SRC_NOFILTER}
	${GGPO_EXAMPLES_VECTORWAR_WIN32RES}
)
