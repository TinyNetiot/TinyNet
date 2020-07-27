

#include "net/tinynet/neighbor_management.h"
#include "net/tinynet/adaptation.h"
#include "net/tinynet/packet_transmission.h"
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



void unicast(uint8_t* payload, uint8_t length, linkaddr_t* dst){
	linkaddr_t addr_rx = PACKET_QUEUE.get_addrx_from_payload(payload);
	PACKET_QUEUE.set_attr(addr_rx, dst);
	NETSTACK_MAC.send(payload, length);
}

void broadcast(uint8_t* payload, uint8_t length, linkaddr_t* dst){
	linkaddr_t addr_rx = PACKET_QUEUE.get_addrx_from_payload(payload);
	PACKET_QUEUE.set_attr(addr_rx, dst);
	char* radio_list = ADDRESSING.map_radio_from_addr(dst); 
	for(each radio in radio_list){		
		common_send(radio, payload);
	}
	
}

void common_send(char* radio){
	uint16_t channel = get_channel();
	if(radio == "ZigBee" && PACKET_QUEUE.is_channel_busy(channel)){
		NETSTACK_MAC_ZigBee.send(payload, length);
		PACKET_QUEUE.set_channel_busy(channel);
	}
	else if(radio =="LoRa"&& PACKET_QUEUE.is_channel_busy(channel)){
		NETSTACK_MAC_LoRa.send(payload, length);
		PACKET_QUEUE.set_channel_busy(channel);
	}
	else if(radio =="BLE"&& PACKET_QUEUE.is_channel_busy(channel)){
		NETSTACK_MAC_BLE.send(payload, length);
		PACKET_QUEUE.set_channel_busy(channel);
	}

}

void multicast(uint8_t* payload, uint8_t length, linkaddr_t* dst){
	linkaddr_t addr_rx = PACKET_QUEUE.get_addrx_from_payload(payload);
	PACKET_QUEUE.set_attr(addr_rx, dst);
	char* radio_list = ADDRESSING.map_radio_from_addr(dst); 
	for(each radio in radio_list){		
		common_send(radio, payload);
	}
}

#define PACKET_TX packet_tx

const struct packet_transmission_driver packet_tx = {
	"packet transmission"
	init,
	broadcast,
	unicast,
	multicast,

}



