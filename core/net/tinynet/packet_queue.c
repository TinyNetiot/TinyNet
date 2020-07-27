



#include "net/tinynet/neighbor_management.h"
#include "net/tinynet/adaptation.h"
#include "net/tinynet/addressing.h"
#include "net/tinynet/packet_queue.h"


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




#define PACKETQUEUE_MAX   255 // max number of packets stored in the buf.
#define PACKETBUF_MAX   255 // max number of  bytes in one packets.
#define MAX_CHANNEL   80 // max number of  bytes in one packets.



#define MAX_CHANNEL_MHZ 		80
static uint8_t conflict_graph[MAX_CHANNEL][MAX_CHANNEL];



static uint8_t* channel_global[MAX_CHANNEL];

static uint32_t* packet_queue[PACKETQUEUE_MAX];
static uint32_t* packet_buf[PACKETBUF_MAX];

static uint32_t* packet_queue_BLE[PACKETQUEUE_MAX];
static uint32_t* packet_queue_LoRA[PACKETQUEUE_MAX];
static uint32_t* packet_queue_ZigBee[PACKETQUEUE_MAX];

uint32_t* create_a_packet(){
	uint32_t* packet_buf = memb_alloc(&packet_memb);; 
	return packet_buf;
}


bool is_channel_busy(uint8_t channel){
	return channel_global[channel]  == 1;
}


void set_channel_busy(uint8_t channel){
	channel_global[channel] = 1;
}

void init(){
	memb_init(&packet_memb);
}

#define PACKET_QUEUE packet_queue

const struct packet_queue_driver packet_queue = {
	"packet_relay",
	init,
	is_channel_busy,
	set_channel_busy,
	create_a_packet,

}


