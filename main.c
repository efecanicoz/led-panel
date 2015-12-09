#include "hal.h"
#include "ch.h"
#include "crc.h"

/*Macros*/
#define SOH 0x01
#define EOT 0x04
#define ID  0xF0
#define TO	0x10


void prepareForSend(uint8_t *packet, uint8_t msg)
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
	
	/*Set input pullup mode first 8 ports of gpiod*/
	palSetGroupMode(GPIOA, PAL_GROUP_MASK(8), 0, PAL_MODE_INPUT_PULLUP);
	
	palSetPadMode(GPIOB, 10, PAL_MODE_ALTERNATE(7)); // used function : USART3_TX
	palSetPadMode(GPIOB, 11, PAL_MODE_ALTERNATE(7)); // used function : USART3_RX
	/* Start the serial driver(change it's state to ready) pointed by arg1 with configurations in arg2
     * if arg2 is NULL then use default configuration in halconf.h*/
	sdStart(&SD3, NULL);
	
	/*Read first 8 pin of gipod*/
	while(!0)
	{
		portSample = palReadGroup(GPIOA, PAL_GROUP_MASK(8), 0);
		if(portSample != 0xFF)//if it's non zero
		{
			prepareForSend(packet, portSample);
			/*packet[0] = portSample;
			packet[1] = 0U;
			packet[2] = 0U;
			packet[3] = 0U;
			packet[4] = 0U;//high byte
			packet[5] = 0U;//low byte
			packet[6] = 0U;*/
			sdWrite(&SD3, (uint8_t *)packet, 7);
		}
		chThdSleepMilliseconds(5);
	}
	return 0;
}
