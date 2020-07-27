

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



void  start_scan(uint16_t scan_interval, uint16_t* scann_channel){
	NETSTACK_MAC.start_scan(scan_interval, scann_channel);
}
void  start_adv(uint16_t adv_interval, uint16_t* adv_channel){
	NETSTACK_MAC.start_adv(adv_interval, adv_channel);
}

void init(){
	return;
}

#define NETSTACK_ADAP adaptation;

const struct adaptation_driver adaptation = {
	"adaptation",
	init,
	start_scan,	
	start_adv,
};



