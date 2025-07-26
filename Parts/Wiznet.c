
#include "Wiznet.h"

#include "Core.h"
#include "GPIO.h"
#include "SPI.h"

#include "Wiznet/Ethernet/wizchip_conf.h"
#include "Wiznet/Internet/DHCP/dhcp.h"

/*
 * PRIVATE DEFINITIONS
 */

#define DHCP_SOCKET			0

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

static void Wiznet_SPI_Write(const uint8_t * tx, uint32_t size);
static void Wiznet_SPI_Read(uint8_t * rx, uint32_t size);
static inline void Wiznet_CS_Select(void);
static inline void Wiznet_CS_Deselect(void);

/*
 * PRIVATE VARIABLES
 */

static uint8_t gDhcpBuffer[548];

/*
 * PUBLIC FUNCTIONS
 */

void Wiznet_Init(void)
{
	GPIO_EnableOutput(W5500_RST_PIN, GPIO_PIN_SET);
	GPIO_EnableOutput(W5500_CS_PIN, GPIO_PIN_SET);
	SPI_Init(W5500_SPI, 1000000, SPI_Mode_0);

	reg_wizchip_cs_cbfunc(Wiznet_CS_Select, Wiznet_CS_Deselect);
	reg_wizchip_spi_cbfunc(Wiznet_SPI_Read, Wiznet_SPI_Write);

	wizchip_init(NULL, NULL);

	wiz_PhyConf phy_conf = {
		.by = PHY_CONFBY_HW,
		.mode = PHY_MODE_AUTONEGO,
		.speed = PHY_SPEED_100,
		.duplex = PHY_DUPLEX_FULL,
	};
	wizphy_setphyconf(&phy_conf);

	DHCP_init(DHCP_SOCKET, gDhcpBuffer);
}

void Wiznet_Deinit(void)
{
	SPI_Deinit(W5500_SPI);
	GPIO_Deinit(W5500_CS_PIN);
	GPIO_Deinit(W5500_RST_PIN);
}

void Wiznet_Update(void)
{
	static uint32_t tlast = 0;
	uint32_t now = CORE_GetTick();
	if ((now - tlast) >= 1000)
	{
		DHCP_time_handler();
	}
	DHCP_run();
}

/*
 * PRIVATE FUNCTIONS
 */

static void Wiznet_SPI_Write(const uint8_t * tx, uint32_t size)
{
	SPI_Write(W5500_SPI, tx, size);
}

static void Wiznet_SPI_Read(uint8_t * rx, uint32_t size)
{
	SPI_Read(W5500_SPI, rx, size);
}

static inline void Wiznet_CS_Select(void)
{
	GPIO_Reset(W5500_CS_PIN);
}

static inline void Wiznet_CS_Deselect(void)
{
	GPIO_Set(W5500_CS_PIN);
}

/*
 * INTERRUPT ROUTINES
 */

