#include "hal.h"
#include "ch.h"

#define A1TO9 0b101111111U

int main(void)
{
	halInit();
	chSysInit();
	uint16_t portSample = 0;
    palSetPadMode(GPIOA, GPIOA_PIN0, PAL_MODE_OUTPUT_PUSHPULL);
    palSetGroupMode(GPIOA, A1TO9, 1, PAL_MODE_INPUT_PULLDOWN);
	

	volatile long i,j;
	while(!0)
	{
		portSample = palReadGroup(GPIOA, A1TO9, 1);
		if(portSample )
		{
			palClearPad(GPIOA, GPIOA_PIN0);
			for(i = 0; i < 100000; i++)
				j++;
		}
		else
		{
			palSetPad(GPIOA, GPIOA_PIN0);
				for(i = 0; i < 100000; i++)
					j++;
		}
		
		//chThdSleepMilliseconds(100);
	}
	return 0;
}
