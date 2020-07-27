
#ifndef PACKET_RELAY_H_
#define PACKET_RELAY_H_

#include "contiki-conf.h"
#include "dev/radio.h"


typedef void (* mac_callback_t)(void *ptr, int status, int transmissions);

void mac_call_sent_callback(mac_callback_t sent, void *ptr, int status, int num_tx);

struct packet_relay_driver {
  char *name;

  void (* init)(void);

  void (* send)(mac_callback_t sent_callback, void *ptr);

  void (* input)(void);
  
  int (* on)(void);

  int (* off)(int on);

};


#endif /* PACKET_RELAY_H_ */

