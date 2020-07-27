
#ifndef ADAPTATION_H_
#define ADAPTATION_H_

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


struct adaptation_driver {
  char *name;

  void (* start_scan)(uint16_t scan_interval, uint16_t* scann_channel);

  void (* start_adv)(uint16_t adv_interval, uint16_t* adv_channel);

  


  void (* init)(void);

  };


#endif /* ADAPTATION_H_ */


