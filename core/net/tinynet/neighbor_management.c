
#include "net/tinynet/neighbor_management.h"
#include "net/tinynet/adaptation.h"


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


static struct tinynet_echo_reply_notification tinynet_le_notification;
static process_event_t ble_link_estimation_event;
static clock_time_t start_time;
static clock_time_t end_time;
static clock_time_t le_delay;
static uint8_t echo_count_rd;

static nbr_utilities* nbr_list;


struct rtimer nd_timer;
struct rtimer letimer;



static bool isLE;

#define LE_ECHO_INTERVAL              (CLOCK_SECOND / 4) //
#define LE_EXECUTE_INTERVAL              (CLOCK_SECOND)  // interval for post LE events.
#define LEWINDOW              10 // link estimator window size. we only store LEWINDOW data.
// #define ECHO_REPLY_TIMEOUT			CLOCK_SECOND // Reply delay larger than this, then we stop wait.
#define MAX_ECHO_COUNT			3// the maximum number of echo try for each dst.

typedef struct{
	linkaddr_t addr;
	uint32_t on;
	uint32_t off;
	uint32_t delay;
	uint32_t etx;
	uint32_t le;
	uint32_t le_window[LEWINDOW];
	uint32_t txc;
}nbr_utilities;


void tinynet_le_reply_handler(uip_ipaddr_t *source, uint8_t ttl,
                   uint8_t *data, uint16_t datalen)
{
  echo_received = 1;
  end_time= clock_time();
  
  struct link_stats *stats;
  static clock_time_t dif_time = end_time - start_time;

  stats = nbr_table_get_from_lladdr(link_stats, lladdr);
  if(stats == NULL) {
    stats = nbr_table_add_lladdr(link_stats, lladdr, NBR_TABLE_REASON_LINK_STATS, NULL);
    if(stats != NULL) {
      stats->time = dif_time;
      stats->etx = LINK_STATS_INIT_ETX(stats);
    }
    return;
  }

  stats->rssi = ((int32_t)stats->time * (EWMA_SCALE - EWMA_ALPHA) +
      (int32_t)dif_time * EWMA_ALPHA) / EWMA_SCALE;
}

uint8_t start_link_quality(){
	
	nbr_utilities *nbr_tmplist = get_nbr_list();
	isLE = true;
	for( int i=0; i%length(nbr_tmplist) < length(nbr_tmplist); i++ ){
		if(isLE == false){
			break;
		}
		etimer_set(&letimer, LE_EXECUTE_INTERVAL);
	    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&letimer));
	    process_post(&nbr_management_process, ble_link_estimation_event, (void *)nbr_tmplist[i]);
	}
}

uint32_t EWMA_le(uint32_t* le_window){
	struct link_stats *stats;
  uint16_t packet_etx;
  uint8_t ewma_alpha;


  stats = nbr_table_get_from_lladdr(link_stats, lladdr);
  if(stats == NULL) {
    stats = nbr_table_add_lladdr(link_stats, lladdr, NBR_TABLE_REASON_LINK_STATS, NULL);
    if(stats != NULL) {
      stats->etx = LINK_STATS_INIT_ETX(stats);
    } else {
      return; 
    }
  }

  stats->last_tx_time = clock_time();
  stats->freshness = MIN(stats->freshness + numtx, FRESHNESS_MAX);

  packet_etx = ((status == MAC_TX_NOACK) ? ETX_NOACK_PENALTY : numtx) * ETX_DIVISOR;
  ewma_alpha = link_stats_is_fresh(stats) ? EWMA_ALPHA : EWMA_BOOTSTRAP_ALPHA;

  stats->etx = ((uint32_t)stats->etx * (EWMA_SCALE - ewma_alpha) +
      (uint32_t)packet_etx * ewma_alpha) / EWMA_SCALE;
}

// only estimate one neighbor
uint8_t start_one_link_quality(linkaddr_t *dst){
	do{
		etimer_set(&letimer, LE_EXECUTE_INTERVAL);
	    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&letimer));
	    process_post(&nbr_management_process, ble_link_estimation_event, (void *)dst);
	}while(isLE);
}

uint8_t stop_link_quality(){
	isLE = false;
	etimer_stop(&letimer);
	return LE_SUCCESS;
}


bool dst_has_le(linkaddr_t *dst, nbr_utilities* nbr_listmp){
	uint8_t i;
	for(i=0 ; i< get_length(nbr_listmp); i++){
		
		if(dst == nbr_listmp[i]->addr){
			return true;
		}
	}
	return false;
}


uint32_t get_le_by_linkaddr(linkaddr_t *dst){
	if(dst_has_le(dst, nbr_list)){ // if we have the le data of *dst*。
		nbr_list[dst]->le = EWMA_le(nbr_list[dst]->le_window);
		return nbr_list[dst]->le;
	}
	return 0;
}


void input(void* data){
	linkaddr_t* src = get_src_addr_from_packet(data);
	char* data_content = get_data_from_packet(data);
	if(data_content == "TINYNET_ECHO_REQUEST"){
		tinynet_echo_reply_callback_t(&src, NULL, data, length(data));
	}
	else if(data_content == "TINYNET_FIND_NEIGHBOR"){
		scanned_callback(&src, NULL, data, length(data));
	}
	else if(data_content == "TINYNET_LPL_RECV"){
		recv_lpl_callback(&src, NULL, data, length(data));
	}

}


static struct rtimer scan_timer;
static struct rtimer adv_timer;


static process_event_t nd_event;
static process_event_t nd_data_mode_event;
static uint16_t l_scan;
static uint16_t l_norm;
static uint16_t l_inta;
static uint16_t l_b;

static uint16_t m;    // periodic of nd patterns.
static uint16_t m_adv_offset;  // used to determine which slot is advertising.
static uint16_t current_m_offset;
static bool isAdvOk;
static bool isNDenable;
static bool isFOUND;

void stopScan(){
	etimer_stop(&scan_timer);
}

void stopAdv(){
	etimer_stop(&adv_timer);
}

void nd_event(){
	if(! isNDenable){return;}
	if( current_m_offset++ % m != m_adv_offset){
		rtimer_set(&nd_timer, l_scan+l_norm, 0, nd_event, ptr);
		isAdvOk = false;
		// STATE.set_scan( l_scan, SCAN_CHANNEL);// for lora and ble, it may be different.
		neighbor_discovery_process(ND_SCAN, [l_scan, SCAN_CHANNEL]);

	}
	else{
		rtimer_set(&nd_timer, l_scan+l_norm, 0, nd_event, ptr);
		rtimer_set(&nd_timer, l_inta, 0, nd_data_mode_event, ptr);
		isAdvOk = true;
		// STATE.set_adv(l_b , CHANNEL_LIST); //
		neighbor_discovery_process(ND_ADVERTISEMENT, [l_b, CHANNEL_LIST]);//

	}
}

void nd_scan_timeout_event(){ // stop scan when it is done or timeout.
	// STATE.set_data_mode();
	neighbor_discovery_process(ND_DATA_TRANSMISSION, []);// 

}

void nd_data_mode_event(){
	if(! isNDenable){return;}
	if(isAdvOk){
		rtimer_set(&nd_timer, l_inta, 0, nd_data_mode_event, ptr);
		// STATE.set_adv(l_b , CHANNEL_LIST); // still set adv when the next scan event not come
		neighbor_discovery_process(ND_ADVERTISEMENT, [l_b, CHANNEL_LIST]);// 

		NETSTACK_MAC.send(adv_data);
	}
}

void scanned_callback(linkaddr_t *source,uint8_t ttl,
                                                 uint8_t *data,
                                                 uint16_t datalen){
	linkaddr_t* dst = get_dst_from_packet(data);
	list_add(get_nbr_list(), dst);
	isFOUND = true;
	neighbor_discovery_process(ND_FOUNDNEIGHBOR, data);

}


void nd_lpl_broadcast_event(){
	if(! isNDenable){return;}
	rtimer_set(&nd_timer, CLOCK_SECOND * 16 + random_rand() % (CLOCK_SECOND * 16), 0, nd_lpl_broadcast_event, ptr);
	broadcast_send(&broadcast);
}


void recv_lpl_callback(linkaddr_t *source,uint8_t ttl,
                                                 uint8_t *data,
                                                 uint16_t datalen){
	linkaddr_t* dst = get_dst_from_packet(data);
	list_add(get_nbr_list(), dst);
	
}

void start_channel_hopping_based_nd(){
	// for channel hopping based.
	if( current_m_offset++ % m != m_adv_offset){
		rtimer_set(&nd_timer, l_scan+l_norm, 0, nd_event, ptr);
		rtimer_set(&nd_timer, l_scan, 0, nd_scan_timeout_event, ptr); 
		isAdvOk = false;
		// STATE.set_scan( l_scan, SCAN_CHANNEL);// for lora and ble, it may be different.
		neighbor_discovery_process(ND_SCAN, [l_scan, SCAN_CHANNEL]);

	}
	else{
		rtimer_set(&nd_timer, l_scan+l_norm, 0, nd_event, ptr);
		rtimer_set(&nd_timer, l_inta, 0, nd_data_mode_event, ptr);
		isAdvOk = true;
		// STATE.set_adv(l_b , CHANNEL_LIST); //
		neighbor_discovery_process(ND_ADVERTISEMENT, [l_b, CHANNEL_LIST]);// 
	}
}

void start_LPL_based_nd(){
	rtimer_set(&nd_timer, CLOCK_SECOND * 16 + random_rand() % (CLOCK_SECOND * 16), 0, nd_lpl_broadcast_event, ptr);
	// NETSTACK_MAC.broadcast_send(&broadcast);	
	PACKET_TX.broadcast(&broadcast);
}

uint8_t start_nd(){
	isNDenable = true;
	if( BLE_RADIO || LORA_RADIO ){
		start_channel_hopping_based_nd();
	}
	else if( ZIGBEE_RADIO){
		start_LPL_based_nd();
	}
	else if (BLE_RADIO || LORA_RADIO  || ZIGBEE_RADIO){ // at GW, we should start two types of ND simulatencely.
		start_channel_hopping_based_nd();
		start_LPL_based_nd();
	}	
}



void advertisement_event(){
	if(isFOUND){
		return ;
	}
	rtimer_set(&nd_timer, adv_interval, 0, advertisement_event, ptr);
	PACKET_TX.broadcast(&broadcast);
}


uint8_t start_advertisement(){

	rtimer_set(&nd_timer, adv_interval, 0, advertisement_event, ptr);
	PACKET_TX.broadcast(&broadcast);

}
nbr_utilities* get_nbr_list(){
	return nbr_list;
}


uint8_t stop_nd(){
	isNDenable = false;
	etimer_stop(&nd_timer);
}

// channel hopping nd process.
void neighbor_discovery_process(uint8_t           states, uint8_t*  data){

	switch(states){
		case ND_ADVERTISEMENT:
			start_advertisement(adv_interval);
			uint16_t adv_int = data[0];
			uint16_t* adv_channel = data[1];
			NETSTACK_ADAP.start_adv(adv_int, adv_channel);

		case ND_SCAN:
			uint16_t scan_interval = data[0];
			uint16_t* scan_channel = data[1];
			NETSTACK_ADAP.start_scan(scan_interval, scan_channel);

		case ND_DATA_TRANSMISSION:
			PACKET_RELAY.send();
		case ND_SLEEP:
			NETSTACK_MAC.sleep();


		case ND_FOUNDNEIGHBOR:
			PACKET_RELAY.data_mode_slot_allocate();

		default:
			printf("Error state");
	}
	
}



void init(){
	isLE = false;
	
}

void on(){
	return;
}

void off(int on){
	return ;
}

PROCESS_THREAD(nbr_management_process, ev, data){

	while(1){
		PROCESS_YIELD_UNTIL(ev == ble_link_estimation_event);
		// perform ping; wait response; therefore, they shall have installed the application to estimate la.
		tinynet_echo_reply_callback_add(&tinynet_le_notification, tinynet_le_reply_handler);
		 start_time = clock_time(); // for now. it is x*7.8ms
		  do {
		    etimer_set(&timer, LE_ECHO_INTERVAL);
		    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
		    tinynet_le_echo_send(&(linkaddr_t* )data, "TINYNET_ECHO_REQUEST", 0, 20);
		    echo_count_rd++;
		    if(echo_count_rd > MAX_ECHO_COUNT){// we stop to send echo, and direct estimate the LE.
	    	  end_time= clock_time();
		    	break;
		    }
		  } while(!echo_received);
		le_delay = end_time - start_time;
		APPEND_DATA(nbr_list[(linkaddr_t* )data].le_window, le_delay);


	}
}

#define NEIGHBOR_MANA nbr_mge

const struct nbr_management_driver nbr_mge = {
	"Neighbor management",
	init,
	start_nd,
	stop_nd,
	get_nbr_list,
	start_link_quality,
	start_one_link_quality,
	stop_link_quality,
	get_le_by_linkaddr,	
	input,
	on,
	off,

};



