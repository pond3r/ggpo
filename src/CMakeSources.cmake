set(GGPO_LIB_INC_NOFILTER
	"lib/ggpo/bitvector.h"
	"lib/ggpo/game_input.h"
	"lib/ggpo/input_queue.h"
	"lib/ggpo/log.h"
	"lib/ggpo/poll.h"
	"lib/ggpo/ring_buffer.h"
	"lib/ggpo/sync.h"
	"lib/ggpo/timesync.h"
	"lib/ggpo/types.h"
	"lib/ggpo/zconf.h"
	"lib/ggpo/zlib.h"
)

set(GGPO_LIB_SRC_NOFILTER
	"lib/ggpo/bitvector.cpp"
	"lib/ggpo/game_input.cpp"
	"lib/ggpo/input_queue.cpp"
	"lib/ggpo/log.cpp"
	"lib/ggpo/main.cpp"
	"lib/ggpo/poll.cpp"
	"lib/ggpo/sync.cpp"
	"lib/ggpo/timesync.cpp"
)

set(GGPO_LIB_INC_NETWORK
	"lib/ggpo/network/udp.h"
	"lib/ggpo/network/udp_msg.h"
	"lib/ggpo/network/udp_proto.h"
)

set(GGPO_LIB_SRC_NETWORK
	"lib/ggpo/network/udp.cpp"
	"lib/ggpo/network/udp_proto.cpp"
)

set(GGPO_LIB_INC_BACKENDS
	"lib/ggpo/backends/backend.h"
	"lib/ggpo/backends/p2p.h"
	"lib/ggpo/backends/spectator.h"
	"lib/ggpo/backends/synctest.h"
)

set(GGPO_LIB_SRC_BACKENDS
	"lib/ggpo/backends/p2p.cpp"
	"lib/ggpo/backends/spectator.cpp"
	"lib/ggpo/backends/synctest.cpp"
)

set(GGPO_PUBLIC_INC
	"include/ggponet.h"
)

source_group(" " FILES ${GGPO_LIB_INC_NOFILTER} ${GGPO_LIB_SRC_NOFILTER})
source_group("Network" FILES ${GGPO_LIB_INC_NETWORK} ${GGPO_LIB_SRC_NETWORK})
source_group("Backends" FILES ${GGPO_LIB_INC_BACKENDS} ${GGPO_LIB_SRC_BACKENDS})
source_group("Public" FILES ${GGPO_PUBLIC_INC})

set(GGPO_LIB_SRC
	${GGPO_LIB_INC_NOFILTER}
	${GGPO_LIB_SRC_NOFILTER}
	${GGPO_LIB_INC_NETWORK}
	${GGPO_LIB_SRC_NETWORK}
	${GGPO_LIB_INC_BACKENDS}
	${GGPO_LIB_SRC_BACKENDS}
	${GGPO_PUBLIC_INC}
)