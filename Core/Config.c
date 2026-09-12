
#include "Config.h"

#include "FLASH.h"

/*
 * PRIVATE DEFINITIONS
 */

#define CONFIG_PAGE		15
#define CONFIG_KEY		0xAA0055AA

/*
 * PRIVATE TYPES
 */

/*
 * PRIVATE PROTOTYPES
 */

/*
 * PRIVATE VARIABLES
 */

/*
 * PUBLIC FUNCTIONS
 */

void Config_Init(void)
{
	if (Config_Get()->key != CONFIG_KEY)
	{
		const Config_t cfg = {
			.baud = 9600,
			.flags = 0,
			.name = "serial-ethernet",
			.key = CONFIG_KEY,
		};
		Config_Set(&cfg);
	}
}

const Config_t * Config_Get(void)
{
	return (void*)FLASH_GetPage(CONFIG_PAGE);
}

void Config_Set(const Config_t * cfg)
{
	const uint32_t * addr = FLASH_GetPage(CONFIG_PAGE);
	FLASH_Erase(addr);
	FLASH_Write(addr, (void*)cfg, sizeof(Config_t));
}

/*
 * PRIVATE FUNCTIONS
 */

/*
 * INTERRUPT ROUTINES
 */

