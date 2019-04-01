#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>
#include <header.h>

#define PIOA_ID	        2
#define TC0_ID	        17

#define BUTTON_IDLE	    0
#define BUTTON_PRESSED	1

void FIQ_handler(void);

PIO* pioa = NULL;
AIC* aic = NULL;
TC* tc = NULL;

unsigned int counter = 0;
unsigned int BUTTON_1_State = BUTTON_IDLE;
unsigned int BUTTON_2_State = BUTTON_IDLE;

int main(void){
unsigned int gen;

STARTUP;                    //Initialize System

//Inialize Timer
tc->Channel_0.RC = 1024;	//Period 8Hz
tc->Channel_0.CMR = 0x2084;	
tc->Channel_0.IDR = 0xFF;
tc->Channel_0.IER = 0x10;	//Interrupt Enable
gen = tc->Channel_0.SR;	    //Clear any pending timer IRQ

//Initialize Interrupts
aic->FFER = (1 << PIOA_ID) | (1 << TC0_ID);	//Interrupts in line 2, 17 are FIQ
aic->IECR = (1 << PIOA_ID) | (1 << TC0_ID);	//Enable Interrupts in line 2, 17
aic->ICCR = (1 << PIOA_ID) | (1 << TC0_ID);	//Clear Interrupts in line 2, 17

//Initialize Peripherals
pioa->PER = 0xff;	//General Purpose Enable
pioa->PUER = 0xc0;	//Pull up Enable
pioa->OER = 0x3f;	//Output Enable
pioa->ODR = 0xc0;	//Output Disable
pioa->IER = 0xc0;	//Interrupt Enable
gen = pioa->ISR;
pioa->CODR = 0x3f;	//Clear all outputs

pioa->SODR = 0x11;  //Initial State

char tmp;
while ( (tmp = getchar() != 'e') )
{
   
}

aic->IDCR = (1<<PIOA_ID)|(1<<TC0_ID);
tc->Channel_0.CCR = 0x02;
CLEANUP;
return 0;
}

void FIQ_handler(void){

unsigned int fiq = 0;
unsigned int data_in = 0;
unsigned int data_out = 0;

fiq = aic->IPR;	//Read the IRQ sourse

if (fiq & (1 << PIOA_ID))	//PIO Check
{
	data_in = pioa->ISR;	
	aic->ICCR = (1 << PIOA_ID);
	data_in = pioa->PDSR;
	if (data_in & 0x80)
	{
		if (BUTTON_1_State == BUTTON_IDLE)
		{
			BUTTON_1_State = BUTTON_PRESSED;
			tc->Channel_0.CCR = 0x05;	//Enable Timer
		}	
	}
	else 
	{
		if (BUTTON_1_State == BUTTON_PRESSED)
		BUTTON_1_State = BUTTON_IDLE;
	}
	if (data_in & 0x40)
	{
		if (BUTTON_2_State == BUTTON_IDLE)
		{
			BUTTON_2_State = BUTTON_PRESSED;
			tc->Channel_0.CCR = 0x05;	//Enable Timer
		}	
	}
	else 
	{
		if (BUTTON_2_State == BUTTON_PRESSED)
		BUTTON_2_State = BUTTON_IDLE;
	}
}

if (fiq & (1 << TC0_ID))	//TC0 Check
{
	data_out = tc->Channel_0.SR;
	aic->ICCR = (1 << TC0_ID);	//Clear Interrupts in line 17
	counter +=1;
	
	if (BUTTON_1_State == BUTTON_PRESSED)
	{
        if ((counter % 2) == 0)
        {
                     pioa->CODR = 0x3f;
                     pioa->SODR = 0x11;
                     if ((counter % 4) == 0)
                     pioa->SODR = 0x20;
                     else
                     pioa->CODR = 0x20;
                     if ((counter % 80) == 0)
                     {
                                pioa->CODR = 0x3f;
                                pioa->SODR = 0x12;
                                if ((counter % 4) == 0)
                                pioa->SODR = 0x20;
                                else
                                pioa->CODR = 0x20;
                                if ((counter % 24) == 0)
                                {
                                             pioa->CODR = 0x3f;
                                             pioa->SODR = 0x14;
                                             if ((counter % 4) == 0)
                                             pioa->SODR = 0x20;
                                             else
                                             pioa->CODR = 0x20;
                                             if ((counter % 16) == 0)
                                             {
                                                          pioa->CODR = 0x3f;
                                                          pioa->SODR = 0x0c;
                                                          if ((counter % 4) == 0)
                                                          pioa->SODR = 0x20;
                                                          else
                                                          pioa->CODR = 0x20;
                                                          if ((counter % 80) == 0)
                                                          {
                                                                       pioa->CODR = 0x3f;
                                                                       pioa->SODR = 0x0c;
                                                                       if ((counter % 4) == 0)
                                                                       pioa->SODR = 0x20;
                                                                       else
                                                                       pioa->CODR = 0x20;
                                                                       if (BUTTON_2_State == BUTTON_PRESSED)
	                                                                   {
                                                                                          if ((counter % 2) == 0)
                                                                                          {
                                                                                                       pioa->CODR = 0x3f;
                                                                                                       pioa->SODR = 0x14;
                                                                                                       if ((counter % 40) == 0)
                                                                                                       {
                                                                                                                    pioa->CODR = 0x3f;
                                                                                                                    pioa->SODR = 0x11;
                                                                                                       }
                                                                                          }
                                                                    }
                                                          }
                                             }
                                }
                     }
        }
 }
                     
	tc->Channel_0.CCR = 0x05;	//Enable Timer
}

