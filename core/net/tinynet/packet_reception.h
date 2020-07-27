
#ifndef PACKET_RECEPTION_H_
#define PACKET_RECEPTION_H_

#include "contiki-conf.h"
#include "dev/radio.h"



struct packet_reception_driver {
  char *name;

  void (* init)(void);

	void (* input)(void);

};


#endif /* PACKET_RECEPTION_H_ */


