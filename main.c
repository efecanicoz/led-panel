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

	extChannelDisableI(&EXTD1, 0);
	extChannelDisableI(&EXTD1, 1);
	extChannelDisableI(&EXTD1, 4);
	extChannelDisableI(&EXTD1, 5);
	extChannelDisableI(&EXTD1, 6);
	extChannelDisableI(&EXTD1, 7);
	extChannelDisableI(&EXTD1, 9);
	extChannelDisableI(&EXTD1, 10);
	chSysUnlockFromISR();
}

static const EXTConfig extcfg = {
  {
   {EXT_CH_MODE_RISING_EDGE | EXT_MODE_GPIOA, extcb2},
   {EXT_CH_MODE_RISING_EDGE | EXT_MODE_GPIOA, extcb2},
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

static void prepareForSend(uint8_t *packet, uint8_t msg)
{
	packet[0] = SOH;
	packet[1] = ID;
	packet[2] = TO;
	packet[3] = msg;
	uint16_t crcRes = crcSlow(packet, 4);
	packet[4] = crcRes >> 8;//high byte
	packet[5] = crcRes;//low byte
	packet[6] = EOT;
	return;
}

static void sleep(void)
{
	extChannelEnable(&EXTD1, 0);
	extChannelEnable(&EXTD1, 1);
	extChannelEnable(&EXTD1, 4);
	extChannelEnable(&EXTD1, 5);
	extChannelEnable(&EXTD1, 6);
	extChannelEnable(&EXTD1, 7);
	extChannelEnable(&EXTD1, 9);
	extChannelEnable(&EXTD1, 10);

	PWR->CR |= (PWR_CR_LPDS | PWR_CR_CSBF | PWR_CR_CWUF);
	PWR->CR &= ~PWR_CR_PDDS;
	SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
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
	uint16_t rawSample = 0;
	uint8_t portSample;
	uint8_t packet[7];
    palSetGroupMode(GPIOA, A1TO10, 0, PAL_MODE_INPUT_PULLDOWN);
	palSetPadMode(GPIOA, GPIOA_USART_TX, PAL_MODE_ALTERNATE(1)); // used function : USART1_TX
	palSetPadMode(GPIOA, GPIOA_USART_RX, PAL_MODE_ALTERNATE(1)); // used function : USART1_RX
	sdStart(&SD1, NULL);

	volatile long j;
	uint8_t i;
	while(!0)
	{
		sdStart(&SD1,NULL);
		for(i = 0; i < 50U; i++)/*Send button five times, why five ?*/
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
				for(j = 0; j < 100000; j++)
					;
			}
		}
		sdStop(&SD1);
		sleep();
	}
	return 0;
}
