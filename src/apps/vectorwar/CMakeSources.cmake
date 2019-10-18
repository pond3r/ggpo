set(GGPO_EXAMPLES_VECTORWAR_INC_NOFILTER
	"gamestate.hpp"
	"gdi_renderer.hpp"
	"ggpo_perfmon.hpp"
	"nongamestate.hpp"
	"renderer.hpp"
	"resource.hpp"
	"targetver.hpp"
	"vectorwar.hpp"
)

set(GGPO_EXAMPLES_VECTORWAR_SRC_NOFILTER
	"gamestate.cpp"
	"gdi_renderer.cpp"
	"ggpo_perfmon.cpp"
	"main.cpp"
	"vectorwar.cpp"
)

set(GGPO_EXAMPLES_VECTORWAR_WIN32RES
	"VectorWar.rc"
)

source_group(" " FILES ${GGPO_EXAMPLES_VECTORWAR_INC_NOFILTER} ${GGPO_EXAMPLES_VECTORWAR_SRC_NOFILTER} ${GGPO_EXAMPLES_VECTORWAR_WIN32RES})

set(GGPO_EXAMPLES_VECTORWAR_SRC
	${GGPO_EXAMPLES_VECTORWAR_INC_NOFILTER}
	${GGPO_EXAMPLES_VECTORWAR_SRC_NOFILTER}
	${GGPO_EXAMPLES_VECTORWAR_WIN32RES}
)