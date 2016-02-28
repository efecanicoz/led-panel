#include "hal.h"
#include "ch.h"

//a4,a5,a6,a7,a9,a10
#define A4TO10 0b11011110000U
#define MASKA4A7 0b11110000U
#define MASKA9A10 0b11000000000U

/*Macros*/
#define SOH 0x9A
#define EOT 0xD5
//#define ID  0xF0

/*Prototypes*/
static void sleep(void);
static void prepareForSend(uint8_t *, uint8_t);

/* Wake up callback.*/
static void extcb2(EXTDriver *extp, expchannel_t channel) 
{
	(void)extp;
	(void)channel;
	
	chSysLockFromISR();
	/* we must reinit clocks after waking up ESPECIALLY if use HSE or HSI+PLL */
	stm32_clock_init();

	chSysUnlockFromISR();
}

static const EXTConfig extcfg = {
  {
   {EXT_CH_MODE_DISABLED, NULL},
   {EXT_CH_MODE_DISABLED, NULL},
   {EXT_CH_MODE_DISABLED, NULL},
   {EXT_CH_MODE_DISABLED, NULL},
   {EXT_CH_MODE_RISING_EDGE | EXT_MODE_GPIOA, extcb2},
   {EXT_CH_MODE_RISING_EDGE | EXT_MODE_GPIOA, extcb2},
   {EXT_CH_MODE_RISING_EDGE | EXT_MODE_GPIOA, extcb2},
   {EXT_CH_MODE_RISING_EDGE | EXT_MODE_GPIOA, extcb2},
   {EXT_CH_MODE_DISABLED, NULL},
   {EXT_CH_MODE_RISING_EDGE | EXT_MODE_GPIOA, extcb2},
   {EXT_CH_MODE_RISING_EDGE | EXT_MODE_GPIOA, extcb2},
   {EXT_CH_MODE_DISABLED, NULL},
   {EXT_CH_MODE_DISABLED, NULL},
   {EXT_CH_MODE_DISABLED, NULL},
   {EXT_CH_MODE_DISABLED, NULL},
   {EXT_CH_MODE_DISABLED, NULL},
  }
};

/*
 * Packet description:
 * 1 Byte Start Byte
 * 4 bit source id ,3 bit Data, 1 bit parity
 * 1 Byte Stop Byte
 */
static void prepareForSend(uint8_t *packet, uint8_t msg)
{
	uint8_t encoded;
	//logarithm(Math.h) or 8 if, another solution for encode ?
	if(msg == 0x04)
		encoded = 2;
	else if(msg == 0x08)
		encoded = 3;
	else if(msg == 0x10)
		encoded = 4;
	else if(msg == 0x20)
		encoded = 5;
	else if(msg == 0x40)
		encoded = 6;
	else if(msg == 0x80)
		encoded = 7;
	else
		encoded = 0;
		
	//device id
	packet[1] = ID;
	
	//add message
	packet[1] |= encoded<<1;
//	packet[1] = msg;//overwriting packet for debug purpose
	return;
}

static void sleep(void)
{
	/*Stop mode*/
	PWR->CR |= (PWR_CR_LPDS | PWR_CR_CSBF | PWR_CR_CWUF);
	PWR->CR &= ~PWR_CR_PDDS;
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
	//wait for interrupt (enter sleep)
	__WFI();
}


int main(void)
{
	halInit();
	chSysInit();
	
  /*
   * Activates the EXT driver 1.
   */
	extStart(&EXTD1, &extcfg);
	extChannelEnable(&EXTD1, 4);
	extChannelEnable(&EXTD1, 5);
	extChannelEnable(&EXTD1, 6);
	extChannelEnable(&EXTD1, 7);
	extChannelEnable(&EXTD1, 9);
	extChannelEnable(&EXTD1, 10);
	
	const uint8_t calibration[9] = {0x55,0x55,0x55,0x00,0x00,0x00,0xFF,0xFF,0xFF};
	uint16_t rawSample = 0;
	uint8_t portSample;
	uint8_t packet[3];
	
	palClearPad(GPIOF, 0);
	palSetPadMode(GPIOF, 0, PAL_MODE_OUTPUT_PUSHPULL);
    palSetGroupMode(GPIOA, A4TO10, 0, PAL_MODE_INPUT_PULLDOWN);
	palSetPadMode(GPIOA, GPIOA_USART_TX, PAL_MODE_ALTERNATE(1)); // used function :  USART1_TX
	
	palSetPadMode(GPIOA, GPIOA_USART_RX, PAL_MODE_UNCONNECTED); // used function : NOT USART1_RX
	palSetPadMode(GPIOF, GPIOF_OSC_OUT, PAL_MODE_UNCONNECTED);
	palSetPadMode(GPIOB, GPIOB_PIN1, PAL_MODE_UNCONNECTED);
	palSetPadMode(GPIOA, GPIOA_PIN0, PAL_MODE_UNCONNECTED);
	palSetPadMode(GPIOA, GPIOA_LED_GREEN, PAL_MODE_UNCONNECTED);
	
	//prepare constant packet bytes
	packet[0] = SOH;
	packet[2] = EOT;

	while(!0)
	{
		sleep();
		portSample = 0;
		rawSample = palReadGroup(GPIOA, A4TO10, 0);
		portSample |= (MASKA4A7 & rawSample) >> 2;
		portSample |= (MASKA9A10 & rawSample) >> 3;
		
		prepareForSend(packet, portSample);
		
		palSetPad(GPIOF, 0);
		sdStart(&SD1, NULL);
		
		sdWrite(&SD1, calibration, 9);
		sdWrite(&SD1, packet, 3);
		
		chThdSleepMilliseconds(500);
		sdStop(&SD1);
		palClearPad(GPIOF, 0);
	}
	
	return 0;
}
