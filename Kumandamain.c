#include "hal.h"
#include "ch.h"
#include "crc.h"

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
	crcInit();
	
	uint8_t portSample;
	uint8_t packet[7];
	
	/*Set input pullup mode first 8 ports of gpioa*/
	palSetGroupMode(GPIOA, PAL_GROUP_MASK(8), 0, PAL_MODE_INPUT_PULLUP);
	
	palSetPadMode(GPIOA, 9, PAL_MODE_ALTERNATE(7)); // used function : USART1_TX
	palSetPadMode(GPIOA, 10, PAL_MODE_ALTERNATE(7)); // used function : USART1_RX
	
	sdStart(&SD1, NULL);
	
	while(!0)
	{
		/*Read first 8 pin of gipoa*/
		portSample = palReadGroup(GPIOA, PAL_GROUP_MASK(8), 0);
		if(portSample != 0xFF)//if it's non zero
		{
			prepareForSend(packet, portSample);
			sdWrite(&SD1, (uint8_t *)packet, 7);
		}
		chThdSleepMilliseconds(5);
	}
	return 0;
}
