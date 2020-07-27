
#ifndef NEIGHBOR_MANAGEMNET_H_
#define NEIGHBOR_MANAGEMNET_H_

#include "contiki-conf.h"
#include "dev/radio.h"


#define LE_SUCCESS   				1
#define LE_FAIL   	   				2

#define ND_SCAN       	       		1
#define ND_ADVERTISEMENT	       	2
#define ND_DATA_TRANSMISSION  		3
#define ND_FOUNDNEIGHBOR        	4
#define ND_SLEEP        			5




// defined in the radio driver layer, when it is set, and
#ifdef BLE_RADIO
#define BLE_RADIO	        1 // cc2650
#else
#define BLE_RADIO	        NULL // 
#endif
#ifdef LORA_RADIO
#define LORA_RADIO	        1 // lora_sx127x_driver
#else
#define LORA_RADIO	        NULL // 
#endif
#ifdef ZIGBEE_RADIO
#define ZIGBEE_RADIO	        1 // cc2650
#else
#define ZIGBEE_RADIO	        NULL // 
#endif



typedef void (* tinynet_echo_reply_callback_t)(uip_ipaddr_t *source,
                                                 uint8_t ttl,
                                                 uint8_t *data,
                                                 uint16_t datalen);
struct tinynet_echo_reply_notification {
  struct tinynet_echo_reply_notification *next;
  tinynet_echo_reply_callback_t callback;
};


struct nbr_management_driver {
  char *name;

  void (* init)(void);

  uint8_t (* start_nd)();	

   uint8_t (* stop_nd)();	

   nbr_utilities* (* get_nbr_list)();

   uint8_t (*start_link_quality)();

	uint8_t (* start_one_link_quality)(linkaddr_t *dst);

	uint8_t (* stop_link_quality)();

	uint32_t (* get_le_by_linkaddr)(linkaddr_t *dst);

  void (* input)(void* data);
  
  int (* on)(void);

  int (* off)(int on);



};


#endif /* NEIGHBOR_MANAGEMNET_H_ */

