
#ifndef PACKET_QUEUE_H_
#define PACKET_QUEUE_H_

#include "contiki-conf.h"
#include "dev/radio.h"



struct packet_queue_driver {
  char *name;

  void (* init)(void);

	uint32_t* (* create_a_packet)();

	bool (* is_channel_busy)(uint8_t channel);

	void (* set_channel_busy)(uint8_t channel);
	  

};


#endif /* PACKET_QUEUE_H_ */


