#include "contiki.h"
#include "cpu.h"
#include "dev/leds.h"
#include "dev/watchdog.h"
#include "dev/sys-ctrl.h"
#include "pwm.h"
#include "lpm.h"
#include "dev/ioc.h"
#include "contiki-net.h"
#include "net/ipv6/uip.h"
#include "net/packetbuf.h"

#include "coap-engine.h"

/*---------------------------------------------------------------------------*/
#define DEBUG DEBUG_NONE
#include "net/ipv6/uip-debug.h"
/*---------------------------------------------------------------------------*/


#define UDP_SERVER_PORT	6000	//LISTENING ON THIS UDP DEST PORT FOR MULTICAST

static struct uip_udp_conn *server_conn;

#if !NETSTACK_CONF_WITH_IPV6 || !UIP_CONF_ROUTER || !UIP_IPV6_MULTICAST || !UIP_CONF_IPV6_RPL
#error "This example can not work with the current contiki configuration"
#error "Check the values of: NETSTACK_CONF_WITH_IPV6, UIP_CONF_ROUTER, UIP_CONF_IPV6_RPL"
#endif


extern coap_resource_t pwm_light;
void pwm_init_handler();
void pwm_update(uint16_t newdutycycle);

PROCESS(pwm_multicast, "pwm multicast");
AUTOSTART_PROCESSES(&pwm_multicast);
/*---------------------------------------------------------------------------*/


static void
tcpip_handler(void)
{
	PRINTF("RECEIVED MULTICAST FROM: ");
	PRINT6ADDR(&UIP_IP_BUF->srcipaddr);
	PRINTF("\n");

  if(uip_newdata()) {

	uint16_t* newdutycycle = (uint16_t *)uip_appdata; //We use promille instead of %

	if(*newdutycycle <= 1000) {
		pwm_update(*newdutycycle);
		PRINTF("Set PWM to duty cycle %u promille\n", *newdutycycle);
	}

  } 
}


/*---------------------------------------------------------------------------*/


static uip_ds6_maddr_t *
join_mcast_group(void)
{
  uip_ipaddr_t addr;
  uip_ds6_maddr_t *rv;

  uip_ip6addr(&addr, 0xFF0E,0,0,0,0,0,0,0xBABE);
  rv = uip_ds6_maddr_add(&addr);
  if(rv) {
    PRINTF("Joined multicast group ");
    PRINT6ADDR(&uip_ds6_maddr_lookup(&addr)->ipaddr);
    PRINTF("\n");
  }
  return rv;
}
/*---------------------------------------------------------------------------*/
//static void
//unjoin_mcast_group(void){
//  uip_ipaddr_t addr;
//  uip_ds6_maddr_t *mcast_addrs;
//  uip_ip6addr(&addr, 0xFF0E,0,0,0,0,0,0,0xBABE);
//  mcast_addrs = uip_ds6_maddr_lookup(&addr);
//  uip_ds6_maddr_rm(mcast_addrs);
//  PRINTF("Unsubscribing from multicast group\n");
//}
/*---------------------------------------------------------------------------*/


PROCESS_THREAD(pwm_multicast, ev, data)
{
  PROCESS_BEGIN();


  if(join_mcast_group() == NULL) {
    PRINTF("Failed to join multicast group\n");
    PROCESS_EXIT();
  }

  server_conn = udp_new(NULL, 0, NULL);
  if(server_conn == NULL) {
    PRINTF("No UDP connection available, exiting the process!\n");
    PROCESS_EXIT();
  }
  udp_bind(server_conn, UIP_HTONS(UDP_SERVER_PORT));

  PRINTF("Created a server connection with remote address ");
  PRINT6ADDR(&server_conn->ripaddr);
  PRINTF(" local/remote port %u/%u\n", UIP_HTONS(server_conn->lport),
         UIP_HTONS(server_conn->rport));



  //CoAP stuff here
  coap_activate_resource(&pwm_light, "lamp/dimming");
  pwm_init_handler();


  while(1) {
    PROCESS_YIELD();
    if(ev == tcpip_event) {
      tcpip_handler();
    }
  }

  PROCESS_END();

}
