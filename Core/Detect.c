
#include "Detect.h"

#include "Wiznet/Ethernet/wizchip_conf.h"
#include "Wiznet/Ethernet/socket.h"

#include <stdio.h>

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

static uint32_t Detect_MakeReply(uint8_t * bfr, uint32_t size);

/*
 * PUBLIC FUNCTIONS
 */

void Detect_Init(void)
{
	socket(DETECT_SOCKET, Sn_MR_UDP, DETECT_PORT, SF_IO_NONBLOCK);
}

void Detect_Update(void)
{
	uint8_t ip[4];
	uint16_t port;
	uint8_t bfr[256];
	int32_t read = recvfrom(DETECT_SOCKET, bfr, sizeof(bfr), ip, &port);

	if (read > 0 && (strncmp((char*)bfr, DETECT_STRING, strlen(DETECT_STRING)) == 0))
	{
		uint32_t written = Detect_MakeReply(bfr, sizeof(bfr));
		sendto(DETECT_SOCKET, bfr, written, ip, port);
	}
}

/*
 * PRIVATE FUNCTIONS
 */

static uint32_t Detect_MakeReply(uint8_t * bfr, uint32_t size)
{
	char * str = (char *)bfr;
	char * end = str + size;

	wiz_NetInfo info;
	wizchip_getnetinfo(&info);

	str += snprintf(str, end - str, "%d.%d.%d.%d\n", info.ip[0], info.ip[1], info.ip[2], info.ip[3]);
	str += snprintf(str, end - str, "%02X-%02X-%02X-%02X-%02X-%02X\n", info.mac[0], info.mac[1], info.mac[2], info.mac[3], info.mac[4], info.mac[5]);
	str += snprintf(str, end - str, "%d\n", COM_PORT);

	return (uint8_t*)str - bfr;
}

/*
 * INTERRUPT ROUTINES
 */

