
#include "Core.h"
#include "GPIO.h"
#include "Wiznet.h"

#include "Detect.h"
#include "Com.h"

#include <stdio.h>



int main(void)
{
	CORE_Init();
	CORE_Delay(100);
	Wiznet_Init();
	Detect_Init();
	Com_Init();

	while(1)
	{
		Wiznet_Update();
		Detect_Update();
		Com_Update();
		CORE_Idle();
	}
}

