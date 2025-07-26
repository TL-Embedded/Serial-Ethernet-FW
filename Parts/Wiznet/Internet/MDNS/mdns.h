#ifndef	_MDNS_H_
#define	_MDNS_H_

#include <stdint.h>
#include <stdbool.h>

/*
 * PUBLIC DEFINITIONS
 */

#define	MDNS_MAX_BUF_SIZE			256
#define MDNS_MAX_HOSTNAME			64

/*
 * PUBLIC TYPES
 */

typedef enum {
	MDNS_SHUTDOWN = 0,
	MDNS_WAIT_IP = 1,
	MDNS_PROBE = 2,
	MDNS_WAIT_PROBE = 3,
	MDNS_ASSIGN_HOST = 4,
	MDNS_READY = 5,
} MDNS_STATUS_t;

/*
 * PUBLIC FUNCTIONS
 */

/*
 * Initializes the MDNS process
 * This should only be started once the link is up, otherwise name conflicts may not be detected.
 *
 * s: Socket number for the process.
 * buf: Allocated buffer for the packet. This is assumed to be MDNS_MAX_BUF_SIZE in size.
 * hostname: A string for the desired hostname. Do not include the ".local" suffix.
 */
void MDNS_init(uint8_t s, uint8_t * buf, const char * hostname);

/*
 * Stops the MDNS process and closes the used sockets.
 */
void MDNS_stop(void);

/*
 * MDNS process runner. Put this in your loop or on a timer.
 * Return code as per MDNS_STATUS_t
 */
uint8_t MDNS_run(void);

/*
 * MDNS timer handler.
 * Call this at 1 second intevals.
 */
void MDNS_time_handler(void);

/*
 * Copies the current host name into the supplied buffer
 * Returns the string size. Returns 0 if the hostname has not yet been validated.
 */
bool MDNS_get_host(char * bfr, int size);


#endif	// _MDNS_H_

