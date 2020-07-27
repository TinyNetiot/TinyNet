

#include "net/tinynet/neighbor_management.h"
#include "net/tinynet/adaptation.h"
#include "net/tinynet/addressing.h"

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

#define MAX_RADIO_C  		10
#define RADIO_NAME_LEN		10
#define DST_ADDR_LIST		10

typedef struct // the mapping of "addr:radio_type"
{
	char* radio_type[RADIO_NAME_LEN];
	linkaddr_t* addr[DST_ADDR_LIST];
}address_radio_map;


static address_radio_map addrRadioMapp[MAX_RADIO_C]; 
static uint32_t map_len;

linkaddr_t** extract_addr_list_from_radio(char* radio){
	
	 uint8_t i;
	 for(i=0 ;i< map_len; i++){
	 	if(addrRadioMapp[i]->radio_type == radio){
			return addrRadioMapp[i]->addr;
	 	}
	 }
	 return NULL;

}

char* map_radio_from_addr(linkaddr_t* address){
	uint8_t i;
	for(i=0 ;i< map_len; i++){
		linkaddr_t* addr_list = addrRadioMapp[i]->addr;
		uint8_t j;
		for(j=0 ; j< length(addr_list); j++){
			if(addr_list[j]->addr == address){
				return addrRadioMapp[i]->radio_type;
			}
		}
	}
	return NULL;
}


void add_addr_to_radio(linkaddr_t* address, radio_type_t* radio_type){
	addrRadioMapp[map_len]->radio_type = radio_type;
	addrRadioMapp[map_len]->addr = radio_type;
	map_len++;
}		

void init(){
	map_len=0;
}

#define ADDRESSING addressing;

const struct addressing_driver addressing = {
	"addressing",
	init,
	extract_addr_list_from_radio,
	map_radio_from_addr,
};



