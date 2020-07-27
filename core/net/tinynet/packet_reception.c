

#include "net/tinynet/neighbor_management.h"
#include "net/tinynet/adaptation.h"
#include "net/tinynet/packet_transmission.h"
#include "net/tinynet/packet_queue.h"
#include "net/tinynet/packet_reception.h"

#include "net/packetbuf.h"
#include "net/queuebuf.h"

#include "sys/ctimer.h"
#include "sys/clock.h"

#include "lib/random.h"

#include "net/netstack.h"

#include "lib/list.h"
#include "lib/memb.h"

#include <string.h>

#include <stdio.h>

#define DEBUG 0
#if DEBUG
#include <stdio.h>
#define PRINTF(...) printf(__VA_ARGS__)
#else /* DEBUG */
#define PRINTF(...)
#endif /* DEBUG */


#define PACKET_RX packet_rx

void input(){
	char* data_content = get_data_from_packet(data);
	if(data_content == "TINYNET_ECHO_REQUEST"){
		NEIGHBOR_MANA.inpu(data);
	}
	else if(data_content == "TINYNET_FIND_NEIGHBOR"){
		NEIGHBOR_MANA.inpu(data);
	}
	else if(data_content == "TINYNET_LPL_RECV"){
		NEIGHBOR_MANA.inpu(data);
	}
	else{
		NETSTACK_NETWORK.inpu();
	}
}

void init(){
	return ;
}

const struct packet_reception_driver packet_rx = {
	"packet transmission"
	init,
	input,

}



