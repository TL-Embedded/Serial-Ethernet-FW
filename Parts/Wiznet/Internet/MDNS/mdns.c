
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>

#include "../../Ethernet/socket.h"
#include "mdns.h"

/*
 * PRIVATE DEFINITIONS
 */

#define MDNS_PORT					5353
#define MDNS_IP						((uint8_t*)"\xE0\x00\x00\xFB")
#define MDNS_TTL					120

#define	MAXCNAME	   				MDNS_MAX_HOSTNAME
#define MAX_CNAME_SEGMENTS			8

#define	TYPE_A		1	   /* Host address */
#define	TYPE_NS		2	   /* Name server */
#define	TYPE_MD		3	   /* Mail destination (obsolete) */
#define	TYPE_MF		4	   /* Mail forwarder (obsolete) */
#define	TYPE_CNAME	5	   /* Canonical name */
#define	TYPE_SOA	6	   /* Start of Authority */
#define	TYPE_MB		7	   /* Mailbox name (experimental) */
#define	TYPE_MG		8	   /* Mail group member (experimental) */
#define	TYPE_MR		9	   /* Mail rename name (experimental) */
#define	TYPE_NULL	10	   /* Null (experimental) */
#define	TYPE_WKS	11	   /* Well-known sockets */
#define	TYPE_PTR	12	   /* Pointer record */
#define	TYPE_HINFO	13	   /* Host information */
#define	TYPE_MINFO	14	   /* Mailbox information (experimental)*/
#define	TYPE_MX		15	   /* Mail exchanger */
#define	TYPE_TXT	16	   /* Text strings */
#define TYPE_SRV	33	   /* Service record type */
#define	TYPE_ANY	255	   /* Matches any type */

#define	CLASS_IN				0x0001
#define CLASS_MASK				0x7FFF
#define CLASS_UNICAST_REQ		0x8000

#define FLAG_RESPONSE 			0x8000
#define FLAG_AUTHORITIVE 		0x0400

#define MDNS_HEADER_SIZE		12

enum {
	MDNS_MATCH_QUERY = 0,
	MDNS_MATCH_RESPONSE = 1,
};

/*
 * PRIVATE TYPES
 */

typedef struct {
	uint16_t questions;
	uint16_t answers;
	uint16_t flags;
} mdns_t;

typedef struct {
	char name[MAXCNAME];
	uint16_t type;
	uint16_t class;
} mdns_question_t;

typedef struct {
	char name[MAXCNAME];
	uint16_t type;
	uint16_t class;
	uint32_t ttl;
	uint16_t size;
} mdns_answer_t;

/*
 * PRIVATE PROTOTYPES
 */


static void mdns_increment_hostname(char * name);

static int mdns_recieve(mdns_t * mdns, uint8_t * msg, uint16_t max_size, uint8_t match);
static int mdns_handle_queries(uint8_t * msg, uint16_t max_size);
static int mdns_handle_answers(uint8_t * msg, uint16_t max_size);
static int mdns_send_reply(uint8_t * msg, uint16_t max_size, uint16_t type);
static int mdns_send_probe(uint8_t * msg, uint16_t max_size, const char * name);

static uint16_t get16(uint8_t * s);
static uint8_t * put16(uint8_t * s, uint16_t i);
static int parse_name(uint8_t * msg, uint8_t * compressed, char * buf, int16_t len);
static uint8_t * put_name(uint8_t * buf, const char * name);

static int mdns_parse_header(mdns_t * mdns, uint8_t * msg, uint16_t len);
static int mdns_parse_question(mdns_question_t * question, uint8_t * msg, uint16_t len, int offset);
static int mdns_parse_answer(mdns_answer_t * answer, uint8_t * msg, uint16_t len, int offset);

static uint8_t * mdns_put_header(mdns_t * mdns, uint8_t * msg);
static uint8_t * mdns_put_question(mdns_question_t * question, uint8_t * msg);
static uint8_t * mdns_put_answer(mdns_answer_t * answer, uint8_t * msg);

/*
 * PRIVATE VARIABLES
 */

static uint8_t* pMDNSMSG;       // DNS message buffer
static uint8_t  MDNS_SOCKET;    // SOCKET number for DNS
static uint8_t	mdns_state;
static char MDNS_HOST_NAME[MAXCNAME];
static uint8_t mdns_timeout;

/*
 * PUBLIC FUNCTIONS
 */

void MDNS_init(uint8_t s, uint8_t * buf, const char * hostname)
{
	MDNS_SOCKET = s;
	pMDNSMSG = buf;

	snprintf(MDNS_HOST_NAME, sizeof(MDNS_HOST_NAME), "%s.local", hostname);

	uint8_t dhar[6] = {0x01, 0x00, 0x5E, 0x00, 0x00, 0xFB};
	setSn_DHAR(MDNS_SOCKET, dhar);

	uint16_t port = MDNS_PORT;
	uint8_t ttl = 255;
	setsockopt(MDNS_SOCKET, SO_DESTIP, MDNS_IP);
	setsockopt(MDNS_SOCKET, SO_DESTPORT, &port);
	setsockopt(MDNS_SOCKET, SO_TTL, &ttl);
	socket(MDNS_SOCKET, Sn_MR_UDP, MDNS_PORT, SF_MULTI_ENABLE | SF_IGMP_VER2 | SF_IO_NONBLOCK);

	mdns_timeout = 10;
	mdns_state = MDNS_WAIT_IP;
}

void MDNS_stop(void)
{
	mdns_state = MDNS_SHUTDOWN;
	close(MDNS_SOCKET);
}

uint8_t MDNS_run(void)
{
	switch (mdns_state)
	{
	default:
	case MDNS_SHUTDOWN:
		break;

	case MDNS_WAIT_IP:
		// If we recieve bytes, we can probably assume we are connected to the multicast group
		if (getSn_RX_RSR(MDNS_SOCKET) > 0 || mdns_timeout == 0)
		{
			mdns_state = MDNS_PROBE;
		}
		break;

	case MDNS_PROBE:
#ifdef _MDNS_DEBUG_
		printf("Sending probe for %s\r\n", MDNS_HOST_NAME);
#endif
		mdns_send_probe(pMDNSMSG, MDNS_MAX_BUF_SIZE, MDNS_HOST_NAME);
		mdns_state = MDNS_WAIT_PROBE;
		mdns_timeout = 2;
		break;

	case MDNS_WAIT_PROBE:
		if (mdns_handle_answers(pMDNSMSG, MDNS_MAX_BUF_SIZE))
		{
			mdns_state = MDNS_PROBE;
			mdns_increment_hostname(MDNS_HOST_NAME);
			break;
		}
		else if (mdns_timeout == 0)
		{
#ifdef _MDNS_DEBUG_
			printf("Probe timeout. Hostname %s accepted.\r\n", MDNS_HOST_NAME);
#endif
			mdns_state = MDNS_ASSIGN_HOST;
		}
		break;

	case MDNS_ASSIGN_HOST:
		mdns_state = MDNS_READY;
		// Intentional fallthrough

	case MDNS_READY:
		int match = mdns_handle_queries(pMDNSMSG, MDNS_MAX_BUF_SIZE);
		if (match > 0)
		{
#ifdef _MDNS_DEBUG_
			printf("Replying to query type 0x%04X\r\n", match);
#endif
			mdns_send_reply(pMDNSMSG, sizeof(pMDNSMSG), match);
		}
		break;
	}
	return mdns_state;
}

void MDNS_time_handler(void)
{
	if (mdns_timeout > 0)
	{
		mdns_timeout--;
	}
}

bool MDNS_get_host(char * bfr, int size)
{
	if (mdns_state >= MDNS_ASSIGN_HOST)
	{
		*bfr = 0;
		strncat(bfr, MDNS_HOST_NAME, size);
		return true;
	}
	return false;
}

/*
 * PRIVATE FUNCTIONS
 */

static void mdns_increment_hostname(char * name)
{
    char * end = name + MAXCNAME;

    char * suffix = name + strlen(name) - 6; // Char prior to ".local"
    char * ch = suffix - 1;
    while (ch > name && (*ch >= '0' && *ch <= '9')) {
        ch--; // Find the start of the number (if any)
    }

    if (*ch == '-') {
        ch++;
        // we found a number. Increment it.
        int num = atoi(ch) + 1;
        snprintf(ch, end - ch, "%d.local", num); // Increment number
    } else {
        // No number. Add one.
        snprintf(suffix, end - ch, "-2.local");
    }
}

static int mdns_recieve(mdns_t * mdns, uint8_t * msg, uint16_t max_size, uint8_t match)
{
	uint16_t len = getSn_RX_RSR(MDNS_SOCKET);
	if (len > 0)
	{
		// Port is not set if reciving an oversized packet.
		uint16_t port = 0;
		uint8_t ip[4];
		len = recvfrom(MDNS_SOCKET, msg, MDNS_HEADER_SIZE, ip, &port);

		uint16_t remainder;
		getsockopt(MDNS_SOCKET, SO_REMAINSIZE, &remainder);

		bool accepted =  remainder <= (max_size - MDNS_HEADER_SIZE)
					 && mdns_parse_header(mdns, msg, len) > 0
					 && (mdns->flags & FLAG_RESPONSE) == (match == MDNS_MATCH_RESPONSE ? FLAG_RESPONSE : 0);

		if (accepted)
		{
			len += recvfrom(MDNS_SOCKET, msg + MDNS_HEADER_SIZE, remainder, ip, &port);
#ifdef _MDNS_DEBUG_
			printf("Receive MDNS message from %d.%d.%d.%d:%d. len = %d\r\n", ip[0], ip[1], ip[2], ip[3], port, len);
#endif
			return len;
		}
		else
		{
			int discard = recvdiscard(MDNS_SOCKET);
#ifdef _MDNS_DEBUG_
			printf("Discarding %d bytes\r\n", discard);
#endif
		}
	}
	return 0;
}

static int mdns_handle_queries(uint8_t * msg, uint16_t max_size)
{
	mdns_t mdns;
	int len = mdns_recieve(&mdns, msg, max_size, MDNS_MATCH_QUERY);

	if (len <= 0)
		return 0;

	int offset = MDNS_HEADER_SIZE;

	for (int i = 0; i < mdns.questions; i++)
	{
		mdns_question_t question;

		offset = mdns_parse_question(&question, msg, len, offset);
		if (offset < 0)
			return 0;

		if ((question.class & CLASS_MASK) == CLASS_IN)
		{
			switch (question.type)
			{
			case TYPE_A:
				if (strcmp(question.name, MDNS_HOST_NAME) == 0)
					return TYPE_A;
				break;
			default:
				break;
			}
		}
	}

	return 0;
}

static int mdns_handle_answers(uint8_t * msg, uint16_t max_size)
{
	mdns_t mdns;
	int len = mdns_recieve(&mdns, msg, max_size, MDNS_MATCH_RESPONSE);

	if (len <= 0 || mdns.questions > 0)
		// Not interested in queries.
		return 0;

	int offset = MDNS_HEADER_SIZE;

	for (int i = 0; i < mdns.answers; i++)
	{
		mdns_answer_t answer;

		offset = mdns_parse_answer(&answer, msg, len, offset);
		if (offset < 0)
			return 0;

		if ( (answer.class & CLASS_MASK) == CLASS_IN)
		{
			if (answer.type == TYPE_A && strcmp(answer.name, MDNS_HOST_NAME) == 0)
			{
				// Thats a reply to our probe. Bummer.
				return TYPE_A;
			}
		}
	}

	return 0;
}

static int mdns_send_reply(uint8_t * msg, uint16_t max_size, uint16_t type)
{
	uint8_t * cp = msg;

	mdns_t mdns = {
		.answers = 1,
		.questions = 0,
		.flags = FLAG_RESPONSE | FLAG_AUTHORITIVE,
	};
	cp = mdns_put_header(&mdns, cp);

	switch (type)
	{
	case TYPE_A:
		mdns_answer_t answer = {
			.type = TYPE_A,
			.class = CLASS_IN,
			.ttl = MDNS_TTL,
			.size = 4,
		};
		strcpy(answer.name, MDNS_HOST_NAME);
		cp = mdns_put_answer(&answer, cp);
		getSIPR(cp);
		cp += answer.size;
		break;

	default:
		return -1;
	}

	int len = cp - msg;
	return sendto(MDNS_SOCKET, pMDNSMSG, len, MDNS_IP, MDNS_PORT);
}

static int mdns_send_probe(uint8_t * msg, uint16_t max_size, const char * name)
{
	uint8_t * cp = msg;

	mdns_t mdns = {
		.answers = 0,
		.questions = 1,
		.flags = 0,
	};
	cp = mdns_put_header(&mdns, cp);

	mdns_question_t question = {
		.class = CLASS_IN,
		.type = TYPE_A,
	};
	strcpy(question.name, name);
	cp = mdns_put_question(&question, cp);

	int len = cp - msg;
	return sendto(MDNS_SOCKET, pMDNSMSG, len, MDNS_IP, MDNS_PORT);
}

/* converts uint16_t from network buffer to a host byte order integer. */
static uint16_t get16(uint8_t * s)
{
	uint16_t i;
	i = *s++ << 8;
	i = i + *s;
	return i;
}

/* copies uint16_t to the network buffer with network byte order. */
static uint8_t * put16(uint8_t * s, uint16_t i)
{
	*s++ = i >> 8;
	*s++ = i;
	return s;
}

/*
 *              CONVERT A DOMAIN NAME TO THE HUMAN-READABLE FORM
 *
 * Description : This function converts a compressed domain name to the human-readable form
 * Arguments   : msg        - is a pointer to the reply message
 *               compressed - is a pointer to the domain name in reply message.
 *               buf        - is a pointer to the buffer for the human-readable form name.
 *               len        - is the MAX. size of buffer.
 * Returns     : the length of compressed message
 */
static int parse_name(uint8_t * msg, uint8_t * compressed, char * buf, int16_t len)
{
	uint16_t slen;		/* Length of current segment */
	uint8_t * cp;
	int clen = 0;		/* Total length of compressed name */
	bool indirect = false;	/* Set if indirection encountered */

	cp = compressed;
	char * end = buf + len;

	for (uint32_t segment = 0;; segment++)
	{
		slen = *cp++;

		if (!indirect) clen++;

		// Check for message indirection
		if ((slen & 0xc0) == 0xc0)
		{
			if (!indirect)
				clen++;
			indirect = true;
			uint16_t dst = ((slen & 0x3f)<<8) + *cp;
			// Uh... should we bounds check this?
			// This feels unsafe, considering dst could be any 14 bits!
			cp = &msg[dst];
			slen = *cp++;
		}

		if (slen == 0) break;

		if (segment > MAX_CNAME_SEGMENTS || buf + slen + 1 >= end)
			return -1;

		if (!indirect) clen += slen;

		if (segment != 0)
			*buf++ = '.';

		while (slen-- != 0) *buf++ = (char)*cp++;
	}

	*buf = '\0';
	return clen;
}

static uint8_t * put_name(uint8_t * buf, const char * name)
{
	// Warning this will happily overflow a buffer!

	int remainder = strlen(name);

	while (1)
	{
		char * segment_end = strchr(name, '.');
		int length = (segment_end == NULL)
				? remainder
				: segment_end - name;

		*buf++ = length;
		strncpy((char*)buf, name, length);
		buf += length;

		if (segment_end == NULL)
		{
			*buf++ = 0;
			break;
		}

		// Skip over the segment and the next dot.
		name += length + 1;
		remainder -= length + 1;
	}
	return buf;
}

static int mdns_parse_header(mdns_t * mdns, uint8_t * msg, uint16_t len)
{
	if (len >= 12)
	{
		uint16_t id = get16(&msg[0]);

		if (id != 0)
			return -1;

		mdns->flags = get16(&msg[2]);
		mdns->questions = get16(&msg[4]);
		mdns->answers = get16(&msg[6]);
		//get16(&msg[8]); // ns count
		//get16(&msg[10]); // ar count
		return 12;
	}
	return -1;
}

static uint8_t * mdns_put_header(mdns_t * mdns, uint8_t * msg)
{
	uint8_t * cp = msg;
	cp = put16(cp, 0); // id
	cp = put16(cp, mdns->flags);
	cp = put16(cp, mdns->questions);
	cp = put16(cp, mdns->answers);
	cp = put16(cp, 0); // ns count
	cp = put16(cp, 0); // ar count
	return cp;
}

static int mdns_parse_question(mdns_question_t * question, uint8_t * msg, uint16_t len, int offset)
{
	int nlen = parse_name(msg, msg + offset, question->name, sizeof(question->name));

	if (nlen < 0 || offset + nlen + 4 > len)
		return -1;
	offset += nlen;

	question->type = get16( msg + offset );
	offset += 2;
	question->class = get16( msg + offset );
	offset += 2;
	return offset;
}

static uint8_t * mdns_put_question(mdns_question_t * question, uint8_t * msg)
{
	uint8_t * cp = msg;
	cp = put_name(cp, question->name);
	cp = put16(cp, question->type);
	cp = put16(cp, question->class);
	return cp;
}

static int mdns_parse_answer(mdns_answer_t * answer, uint8_t * msg, uint16_t len, int offset)
{
	int nlen = parse_name(msg, msg + offset, answer->name, sizeof(answer->name));

	if (nlen < 0 || offset + nlen + 12 > len)
		return -1;
	offset += nlen;

	answer->type = get16( msg + offset );
	offset += 2;
	answer->class = get16( msg + offset );
	offset += 2;
	answer->ttl = get16( msg + offset ) << 16;
	answer->ttl |= get16( msg + offset + 2 );
	offset += 4;
	answer->size = get16( msg + offset );
	offset += 2;
	return offset;
}

static uint8_t * mdns_put_answer(mdns_answer_t * answer, uint8_t * msg)
{
	uint8_t * cp = msg;
	cp = put_name(cp, answer->name);
	cp = put16(cp, answer->type);
	cp = put16(cp, answer->class);
	cp = put16(cp, answer->ttl >> 16);
	cp = put16(cp, answer->ttl);
	cp = put16(cp, answer->size);
	return cp;
}

/*
 * INTERRUPT ROUTINES
 */


