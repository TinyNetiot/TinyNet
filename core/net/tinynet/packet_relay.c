

#include "net/tinynet/packet_relay.h"

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



#define PACKET_RELAY packet_relay_driver

ble_conn_param_t* conn[conn_counter];

//关键可能是这里，如何启动两个角色之后，如何协调多种slot。
void init(){
	NETSTACK_MAC.init();

//for ble,
	if( IS_RELAY_INITIAL){
		set_relay_initiator();
	}

//for lorawan.


//for zigbee,


}

typedef struct ble_role_t{
	linkaddr_t* address;
}BLE_ROLE_LINK_MAP;


BLE_ROLE_LINK_MAP* slave_dst_list[MAX_SLAVE];
BLE_ROLE_LINK_MAP* master;

packet_t* MATER_BUF[MAX_MASTER_PKT]; //  slave connection use this buf.
packet_t* SLAVE_BUF[MAX_SLAVE_PKT]; // master connection event share the same buf. 
// - [ok, radio done.][how to determine] 借助conn保存了各种连接的全局结构体变量，来维持。（cc26xx-radio里面的conn)。
// 		- [ok][packet queue]修改？ 不用。cc26xx里面，维护了不同角色，event对应的buf。到时候，只要直接调用cc26xx-radio就可以把包放在对的位置。每个event仅仅根据自己的conn选择包发送。
// - [TODO] 区分不同的radio地址的时候，需要我们来处理。


void on(){}
void off(){}



// allocate, when new neighbor is discovered.
void data_mode_slot_allocate(){

}

void send(mac_callback_t sent, void *ptr)
{

	radio_type radio = ADDRESSING.determine_radio();

// for ble
	if(NETSTACK_RADIO_CC2650 == true && radio == "BLE"){
		// 发送之前，需要维护一个地址映射表吗？ 即那个目标地址，应该使用哪个角色发送？ 否则每种角色都会发送一遍数据包，而且有可能有些数据包完全不可能传递过去。
		// 	- [ok]不需要。蓝牙里面的cc26xx-radoi.send (RADIO.send())函数里面，就自动会根据link地址将包存放在指定的conn的tx_buffer里面。 master,slave都从conn里面匹配。
		// 	- [ok] BLE已经做了。 这里直接将包放到不同的buffer里面：master_buf, slave_buf。 那么我们要做的，就是只有同时启动两种角色，并且对它们的时隙进行适当的安排。


		NETSTACK_MAC.send(sent, ptr); 
	}

// for lorawan
	else if (NETSTACK_RADIO_SX1278 == true && radio == "LoRa"){

	}

// for zigbee. or lora
	else if(NETSTACK_RADIO_CC2538 == true && radio == "zigbee"){
		NETSTACK_LLSEC.send();
		-> NETSTACK_MAC.send(sent, ptr);  
	}
	
}

void input(){

}



PROCESS_THREAD(packet_relay_process, ev, data){

}

/*---------------------------------------------------------------------------*/
const struct packet_relay_driver packet_relay = {
	"packet_relay";
	init(),
	send(),
	input(),
	on(),
	off(),

};





