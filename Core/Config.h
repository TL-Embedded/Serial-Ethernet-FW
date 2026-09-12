#ifndef CONFIG_H
#define CONFIG_H

#include "STM32X.h"

/*
 * PUBLIC DEFINITIONS
 */

/*
 * PUBLIC TYPES
 */

typedef struct __attribute((aligned(8))) {
	uint32_t baud;
	uint32_t flags;
	char name[64];
	uint32_t key;
} Config_t;

/*
 * PUBLIC FUNCTIONS
 */

void Config_Init(void);
const Config_t * Config_Get(void);
void Config_Set(const Config_t * cfg);

/*
 * EXTERN DECLARATIONS
 */

#endif //CONFIG_H
