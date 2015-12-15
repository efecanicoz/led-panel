#include "hal.h"
#include "ch.h"

int main(void)
{
	halInit();
	chSysInit();
    palSetPadMode(GPIOA, GPIOA_PIN5, PAL_MODE_OUTPUT_PUSHPULL);

	volatile long i,j;
	while(!0)
	{		
		palClearPad(GPIOA, GPIOA_PIN5); 
		for(i = 0; i < 10000000; i++)
			j++;
		//chThdSleepMilliseconds(100);
		palSetPad(GPIOA, GPIOA_PIN5);
		for(i = 0; i < 10000000; i++)
			j++;
		//chThdSleepMilliseconds(100);
	}
	return 0;
}
