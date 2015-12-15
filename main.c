#include "hal.h"
#include "ch.h"
#include "crc.h"

//a0,a1,a4,a5,a6,a7,a9,a10
#define A1TO10 0b11011110011U
#define MASKA0A1 0b11U
#define MASKA4A7 0b11110000U
#define MASKA9A10 0b11000000000U

/*Macros*/
#define SOH 0x01
#define EOT 0x04
#define ID  0xF0
#define TO	0x10


static void prepareForSend(uint8_t *packet, uint8_t msg)
{
	uint16_t crcRes = crcFast(packet, 4);
	packet[0] = SOH;
	packet[1] = ID;
	packet[2] = TO;
	packet[3] = msg;
	packet[4] = crcRes >> 8;//high byte
	packet[5] = crcRes;//low byte
	packet[6] = EOT;
	return;
}
int main(void)
{
	halInit();
	chSysInit();
	uint16_t rawSample = 0;
	uint8_t portSample;
	uint8_t packet[7];
    palSetGroupMode(GPIOA, A1TO10, 0, PAL_MODE_INPUT_PULLDOWN);
	palSetPadMode(GPIOA, GPIOA_USART_TX, PAL_MODE_ALTERNATE(1)); // used function : USART1_TX
	palSetPadMode(GPIOA, GPIOA_USART_RX, PAL_MODE_ALTERNATE(1)); // used function : USART1_RX
	sdStart(&SD1, NULL);

	volatile long i,j;
	while(!0)
	{
		portSample = 0;
		rawSample = palReadGroup(GPIOA, A1TO10, 0);
		portSample |= (MASKA0A1 & rawSample);
		portSample |= (MASKA4A7 & rawSample) >> 2;
		portSample |= (MASKA9A10 & rawSample) >> 3;
		if(portSample != 0x00)//if its different send
		{
			prepareForSend(packet, portSample);
			sdWrite(&SD1, (uint8_t *)packet, 7);
			//chThdSleepMilliseconds(100);
			for(i = 0; i < 100000; i++)
				j++;
		}
	}
	return 0;
}
