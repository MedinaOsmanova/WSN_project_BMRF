#ifndef PROJECT_CONF_H_
#define PROJECT_CONF_H_

#define LOG_LEVEL_APP LOG_LEVEL_DBG

//#define LOG_CONF_LEVEL_RPL LOG_LEVEL_DBG
//#define LOG_CONF_LEVEL_IPV6 LOG_LEVEL_DBG

#include "net/ipv6/multicast/uip-mcast6-engines.h"
#define UIP_MCAST6_CONF_ENGINE	UIP_MCAST6_ENGINE_BMRF
#define BMRF_CONF_MODE	BMRF_UNICAST_MODE

#define LPM_CONF_ENABLE 0  //Absolute must for enabling 32KB RAM!



#endif /* PROJECT_CONF_H_ */

/** @} */
