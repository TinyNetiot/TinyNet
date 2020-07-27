
#ifndef PACKET_TRANSMISSION_H_
#define PACKET_TRANSMISSION_H_

#include "contiki-conf.h"
#include "dev/radio.h"



struct packet_transmission_driver {
  char *name;

  void (* init)(void);

  void (* broadcast)(uint8_t* payload, uint8_t length, linkaddr_t* dst);

  void (* unicast)(uint8_t* payload, uint8_t length, linkaddr_t* dst);

  void (* multicast)(uint8_t* payload, uint8_t length, linkaddr_t* dst);

	  

};


#endif /* PACKET_TRANSMISSION_H_ */


