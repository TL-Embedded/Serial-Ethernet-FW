
#include "Core.h"
#include "Wiznet.h"
#include "Com.h"
#include "Config.h"


int main(void)
{
	CORE_Init();
	CORE_Delay(100);
	Config_Init();
	Wiznet_Init();
	Com_Init();

	while(1)
	{
		Wiznet_Update();
		Com_Update();
		CORE_Idle();
	}
}

