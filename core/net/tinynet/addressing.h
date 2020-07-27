
#ifndef ADDRESSING_H_
#define ADDRESSING_H_

#include "contiki-conf.h"
#include "dev/radio.h"


#define NETSTACK_ADAP adaptation;

typedef void (* tinynet_echo_reply_callback_t)(uip_ipaddr_t *source,
                                                 uint8_t ttl,
                                                 uint8_t *data,
                                                 uint16_t datalen);
struct tinynet_echo_reply_notification {
  struct tinynet_echo_reply_notification *next;
  tinynet_echo_reply_callback_t callback;
};


struct addressing_driver {
  char *name;

  void (* init)(void);

  linkaddr_t** (* extract_addr_list_from_radio)(char* radio);
  
  char* (* map_radio_from_addr)(linkaddr_t* address);
  

};


#endif /* ADDRESSING_H_ */


