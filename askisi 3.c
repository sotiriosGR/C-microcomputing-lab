#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <header.h>

#define PIOA_ID 2
#define TC0_ID 17
#define BUTTON_IDLE 0
#define BUTTON_PRESSED 1
#define CLOCK_IDLE 0
#define CLOCK_HOLD 1
#define LED_OFF 0
#define LED_ON 1

void FIQ_handler(void);
PIO* pioa = NULL;
AIC* aic = NULL;
TC* tc = NULL;

unsigned int segment1 = 0;
unsigned int segment2 = 0;
unsigned int count_one_second = 0;
unsigned int count_one_second_pressed = 0;
unsigned int clock_state = CLOCK_IDLE;
unsigned int button_state = BUTTON_IDLE;
unsigned int led_state = LED_OFF;
unsigned int tmp;

int main(int argc, const char* argv[])
{
	STARTUP; 

	tc->Channel_0.RC = 2048;  
	tc->Channel_0.CMR = 2084; 
	tc->Channel_0.IDR = 0xFF; 
	tc->Channel_0.IER = 0x10;
	tmp = tc->Channel_0.SR;
          
	aic->FFER = (1<<PIOA_ID) | (1<<TC0_ID);
	aic->IECR = (1<<PIOA_ID) | (1<<TC0_ID);
	aic->ICCR = (1<<PIOA_ID) | (1<<TC0_ID);
          
	pioa->PER = 0x3FF;
	pioa->PUER = 0x200;   
	pioa->ODR = 0x200;
	pioa->OER = 0x1FF;
	pioa->CODR = 0x1FF;
	pioa->IER = 0x200;      
	tmp = pioa->ISR;

	tc->Channel_0.CCR = 0x05;

	while(getchar() != 'e') {}
          
	aic->IDCR = (1<<PIOA_ID) | (1<<TC0_ID);
	tc->Channel_0.CCR = 0x02;
	CLEANUP;
	return 0;
}
          
void FIQ_handler(void)
{
	unsigned int fiq = 0;
	unsigned int data_in = 0;
	unsigned int data_out= 0;
	fiq = aic->IPR;

	if(fiq & (1<<PIOA_ID))
	{ 
		data_in = pioa->ISR;
		aic->ICCR = (1<<PIOA_ID);
		data_in = pioa->PDSR;
		count_one_second_pressed = 0;
   
		if(data_in & 0x200)
		{    
			if(button_state == BUTTON_IDLE)
			{
				button_state = BUTTON_PRESSED;

				if(clock_state == CLOCK_IDLE)
				{
					clock_state = CLOCK_HOLD;
					pioa->SODR = 0x100;
					led_state = LED_ON;
					tc->Channel_0.CCR = 0x05;
				}
				else if(clock_state == CLOCK_HOLD)
				{
					clock_state = CLOCK_IDLE;
					pioa->CODR = 0x100;
					led_state = LED_OFF;
					tc->Channel_0.CCR = 0x05;
				}
			}
		}
		else
		{
			if(button_state == BUTTON_PRESSED)
				button_state = BUTTON_IDLE;
		}       
	}

	if(fiq & (1<<TC0_ID))
	{
		data_out = tc->Channel_0.SR;
		aic->ICCR = (1<<TC0_ID);
		data_out = pioa->ODSR;

		if(clock_state == CLOCK_IDLE)
		{
			count_one_second++;
			if(count_one_second == 4)
			{
				count_one_second = 0;
				segment1++;
				if(segment1 == 10)
				{
					segment1 = 0;
					segment2++;
					if(segment2 == 6)
						segment2 = 0;
				}
				pioa->CODR = 0xFF;
				pioa->SODR = segment1 + (segment2<<4);
			}
		}
		else if(clock_state == CLOCK_HOLD)
		{
			if(led_state == LED_ON)
			{
				led_state = LED_OFF;
				pioa->CODR = 0x100;
			}
			else if(led_state == LED_OFF)
			{
				led_state = LED_ON;
				pioa->SODR = 0x100;
			}
		}

		if(button_state == BUTTON_PRESSED)
		{
			count_one_second_pressed++;
			if(count_one_second_pressed == 4)
			{
				count_one_second_pressed = 0;
				pioa->CODR = 0xFF;
				segment1 = 0;
				segment2 = 0;
				count_one_second = 0;
			}
		}

		tc->Channel_0.CCR = 0x05;
	}
}
