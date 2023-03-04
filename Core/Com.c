
#include "Com.h"

#include "UART.h"
#include "GPIO.h"
#include "Wiznet/Ethernet/socket.h"

/*
 * PRIVATE DEFINITIONS
 */

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void Com_ReadSocket(void);
static void Com_ReadSerial(void);

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void Com_Init(void)
{
	GPIO_EnableOutput(LED_TX_PIN, GPIO_PIN_RESET);
	GPIO_EnableOutput(LED_RX_PIN, GPIO_PIN_RESET);
	UART_Init(SERIAL, SERIAL_BAUD, UART_Mode_Default);
}

void Com_Update(void)
{
	uint8_t status;
	getsockopt(COM_SOCKET, SO_STATUS, &status);

	switch (status)
	{
	case SOCK_CLOSED:
		socket(COM_SOCKET, Sn_MR_TCP, COM_PORT, SF_IO_NONBLOCK);
		listen(COM_SOCKET);
		break;

	case SOCK_ESTABLISHED:
	case SOCK_CLOSE_WAIT:
		Com_ReadSocket();
		break;

	default:
		break;
	}

	GPIO_Write(LED_RX_PIN, UART_ReadCount(SERIAL) > 0);

	if (status == SOCK_ESTABLISHED)
	{
		Com_ReadSerial();
	}
	else
	{
		UART_ReadFlush(SERIAL);
	}

	GPIO_Write(LED_TX_PIN, UART_WriteCount(SERIAL) > 0);
}

/*
 * PRIVATE FUNCTIONS
 */

static void Com_ReadSocket(void)
{
	uint8_t bfr[256];
	int32_t read = recv(COM_SOCKET, bfr, sizeof(bfr));

	if (read > 0)
	{
		UART_Write(SERIAL, bfr, read);
	}
}

static void Com_SendSerial(uint32_t count)
{
	uint8_t bfr[256];
	if (count > sizeof(bfr))
	{
		count = sizeof(bfr);
	}
	uint32_t read = UART_Read(SERIAL, bfr, count);
	send(COM_SOCKET, bfr, read);
}

static void Com_ReadSerial(void)
{
	uint32_t count = UART_ReadCount(SERIAL);

	if (count > 64)
	{
		// Buffer is filling up - just get data out.
		Com_SendSerial(256);
		return;
	}

	uint32_t line_size = UART_Seek(SERIAL, '\n');
	if (line_size)
	{
		Com_SendSerial(line_size);
	}
}

/*
 * INTERRUPT ROUTINES
 */

