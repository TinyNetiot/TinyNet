/*
 * Copyright (c) 2017, Graz University of Technology
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/**
 * \file
 *    A simple IPv6-over-BLE UDP-client.
 *
 * \author
 *    Michael Spoerk <michael.spoerk@tugraz.at>
 */
/*---------------------------------------------------------------------------*/
#include "contiki.h"
#include "contiki-net.h"
#include "contiki-lib.h"

#define DEBUG DEBUG_FULL
#include "net/ip/uip-debug.h"
#include "net/ip/uiplib.h"
#include "net/ipv6/uip-icmp6.h"

#include <string.h>
/*---------------------------------------------------------------------------*/

#define SERVER_IP              "fe80::2671:89ff:fe1a:2a82"
#define PING_TIMEOUT              (CLOCK_SECOND / 4)
#define CLIENT_SEND_INTERVAL      (CLOCK_SECOND * 1)


#define CLIENT_PORT           61617
#define SERVER_PORT           61616

#define UDP_LEN_MAX           255
/*---------------------------------------------------------------------------*/
#define UIP_IP_BUF    ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
static struct uip_udp_conn *server_conn;

static char buf[UDP_LEN_MAX];
static uint16_t packet_counter;


/*---------------------------------------------------------------------------*/

//client mode

static uip_ipaddr_t server_addr;
static struct uip_icmp6_echo_reply_notification icmp_notification;
static uint8_t echo_received;
static struct uip_udp_conn *conn;

static struct etimer timer;
static char buf[UDP_LEN_MAX];
//WMY
static char buf_send[UDP_LEN_MAX];
static char buf_rec[UDP_LEN_MAX];
//
static uint16_t packet_counter;

//cgl add for modifying the states.
int ISMASTER = 0;
int previous_role = 0;
/*---------------------------------------------------------------------------*/

PROCESS(ipv6_ble_server_process, "IPv6 over BLE - server process");
AUTOSTART_PROCESSES(&ipv6_ble_server_process);
/*---------------------------------------------------------------------------*/
static void
tcpip_serverhandler(void)
{
  if(uip_newdata()) {
    /* process received message */
    strncpy(buf_rec, uip_appdata, uip_datalen());
    buf_rec[uip_datalen()] = '\0';
    PRINTF("relay_server rec. message: <%s>\n", buf_rec);

    /* send response message */
    uip_ipaddr_copy(&server_conn->ripaddr, &UIP_IP_BUF->srcipaddr);
//    sprintf(buf_send, "Hello client %04u!", packet_counter);
    PRINTF("relay_server send message: <%s>\n", buf_send);
    uip_udp_packet_send(server_conn, buf_send, strlen(buf_send));
    packet_counter++;

    memset(&server_conn->ripaddr, 0, sizeof(server_conn->ripaddr));
  }
}
/*---------------------------------------------------------------------------*/
void
icmp_reply_handler(uip_ipaddr_t *source, uint8_t ttl,
                   uint8_t *data, uint16_t datalen)
{
  PRINTF("echo response received\n");
  echo_received = 1;
}
/*---------------------------------------------------------------------------*/
static void
tcpip_clienthandle(void)
{
 // char data[UDP_LEN_MAX];
  if(uip_newdata()) {
    strncpy(buf_send, uip_appdata, uip_datalen());
 //   data[uip_datalen()] = '\0';
    buf_send[uip_datalen()] = (char)packet_counter;
    buf_send[uip_datalen() + 1] = '\0';
    PRINTF("relay_client rec. message: <%s>\n", buf_send);
    strncpy(buf_send, uip_appdata, uip_datalen());
  }
}
/*---------------------------------------------------------------------------*/
static void
timeout_handler(void)
{
  sprintf(buf, "Hello server %04u!", packet_counter);
  PRINTF("relay_client send message: <%s>\n", buf);
  uip_udp_packet_send(conn, buf, strlen(buf));
  packet_counter++;
}

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(ipv6_ble_server_process, ev, data)
{

 
 PROCESS_BEGIN();
 
while(1){


if(ISMASTER){
  PRINTF("CC26XX-IPv6-over-BLE server started\n");

  server_conn = udp_new(NULL, UIP_HTONS(CLIENT_PORT), NULL);
  udp_bind(server_conn, UIP_HTONS(SERVER_PORT));
  printf("server.c(): udp_bind() finished!\n");

  while(ISMASTER) {
    PROCESS_WAIT_EVENT();
    if(ev == tcpip_event) {
      tcpip_serverhandler();
      ISMASTER = 0;
      
    }
  }
}

  // there is to perform the client process.
if(!ISMASTER){
uiplib_ipaddrconv(SERVER_IP, &server_addr);
  uip_icmp6_echo_reply_callback_add(&icmp_notification, icmp_reply_handler);

  echo_received = 0;

//  printf("server.c():uip_icmp6_echo_reply_callback_add() finished!\n");
  PRINTF("pinging the IPv6-over-BLE server: ");
  PRINT6ADDR(&server_addr);
  PRINTF("\n");
printf("server.c: ISMASTER = %d, previous_role = %d\n", ISMASTER,previous_role);
  do {
    etimer_set(&timer, PING_TIMEOUT);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));
    uip_icmp6_send(&server_addr, ICMP6_ECHO_REQUEST, 0, 20);
   // printf("uip_icmp6_send\n");
  } while(!echo_received);

  conn = udp_new(&server_addr, UIP_HTONS(SERVER_PORT), NULL);
  udp_bind(conn, UIP_HTONS(CLIENT_PORT));

  etimer_set(&timer, CLIENT_SEND_INTERVAL);

  while(!ISMASTER) {
    PROCESS_YIELD();
    if((ev == PROCESS_EVENT_TIMER) && (data == &timer)) {
            timeout_handler();
      etimer_set(&timer, CLIENT_SEND_INTERVAL);
    } else if(ev == tcpip_event) {
      tcpip_clienthandle();
      ISMASTER = 1;
    }
  }
}

}
  PROCESS_END();
}
/*---------------------------------------------------------------------------*/
