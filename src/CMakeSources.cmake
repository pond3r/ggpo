set(GGPO_LIB_INC_NOFILTER
	"lib/ggpo/bitvector.hpp"
	"lib/ggpo/game_input.hpp"
	"lib/ggpo/input_queue.hpp"
	"lib/ggpo/log.hpp"
	"lib/ggpo/poll.hpp"
	"lib/ggpo/ring_buffer.hpp"
	"lib/ggpo/sync.hpp"
	"lib/ggpo/timesync.hpp"
	"lib/ggpo/types.hpp"
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
	"lib/ggpo/network/udp.hpp"
	"lib/ggpo/network/udp_msg.hpp"
	"lib/ggpo/network/udp_proto.hpp"
)

set(GGPO_LIB_SRC_NETWORK
	"lib/ggpo/network/udp.cpp"
	"lib/ggpo/network/udp_proto.cpp"
)

set(GGPO_LIB_INC_BACKENDS
	"lib/ggpo/backends/backend.hpp"
	"lib/ggpo/backends/p2p.hpp"
	"lib/ggpo/backends/spectator.hpp"
	"lib/ggpo/backends/synctest.hpp"
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