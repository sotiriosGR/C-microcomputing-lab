#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <stdio.h>
#include <stdlib.h>

#include <header.h>

#define TC0_ID			17

#define BUT_PLAYER_1	0x800 
#define BUT_PLAYER_2	0x400 

#define BALL_GOING_NOWHERE	0
#define BALL_GOING_RIGHT	1
#define BALL_GOING_LEFT		2
#define BALL_SHOW_SCORE		3

void FIQ_handler(void);

PIO *	pioa	= NULL;
AIC *	aic		= NULL;
TC  *	tc		= NULL;

unsigned int ball_position;
unsigned int ball_direction;
unsigned int start_timer;
unsigned int timer;
unsigned int timer_decrease;
unsigned int player_1_score;
unsigned int player_2_score;
unsigned int xtipimata;

int main( int argc, const char* argv[] ) {
	unsigned int gen, tmp;
	STARTUP;							
	
	// аявийопоигсг летабкгтым
	start_timer = 8192;
	timer_decrease = 200;
	ball_direction = BALL_GOING_NOWHERE;
	player_1_score = 0;
	player_2_score = 0;
	xtipimata = 0;
	
	// яокои
	tc->Channel_0.RC	= start_timer;
	tc->Channel_0.CMR	= 2084;		
	tc->Channel_0.IDR	= 0xFF;		
	tc->Channel_0.IER	= 0x10;		

	// диайопес
	aic->FFER	= (1<<TC0_ID);		
	aic->ICCR	= (1<<TC0_ID);		
	aic->IECR	= (1<<TC0_ID);		

	pioa->PER	= 0xFFF;			
        // енодос
	pioa->OER	= 0x3FF;			
		
	
	while (1) {
		if (ball_direction == BALL_GOING_NOWHERE) {
			pioa->SODR = 0x3FF;
			tmp = pioa->PDSR;
			if (tmp & BUT_PLAYER_1 == 0) {
				ball_direction = BALL_GOING_RIGHT;
				ball_position = 0x200;
    		  		pioa->CODR = 0x3FF;
				pioa->SODR = ball_position;
				xtipimata = 0;
				
				timer = start_timer;
				tc->Channel_0.RC = timer;
				tc->Channel_0.CCR = 0x05;
			}
			else if (tmp & BUT_PLAYER_2 == 0) {
				ball_direction = BALL_GOING_LEFT;
				ball_position = 0x001;
    				pioa->CODR = 0x3FF;
				pioa->SODR = ball_position;
				xtipimata = 0;
				
				timer = start_timer;
				tc->Channel_0.RC = timer;
				tc->Channel_0.CCR = 0x05;
			}
		}
		
		tmp = getchar();
		if (tmp == 'e')
			break;
	}
	
	aic->IDCR	= (1<<TC0_ID);		
	tc->Channel_0.CCR = 0x02;		

	CLEANUP;						
	return 0;
}
void FIQ_handler(void) {
	unsigned int data_in = 0;
	unsigned int fiq = 0;
	unsigned int tmp;
	
	fiq = aic->IPR;				
	if( fiq & (1<<TC0_ID) ) {
        
		tmp			= tc->Channel_0.SR;		
		aic->ICCR	= (1<<TC0_ID); 			
		tmp = pioa->PDSR;
		pioa->CODR = 0x3FF;
		
		
        if (ball_direction == BALL_SHOW_SCORE) {
           if (player_1_score >= 3 || player_2_score >= 3) {
              player_1_score = 0;
              player_2_score = 0;
           }
           ball_direction = BALL_GOING_NOWHERE;
        }




        	else if (ball_direction == BALL_GOING_RIGHT) {
			if (ball_position == 0x001) {
				if (tmp & BUT_PLAYER_2 == 0) {
					ball_direction = BALL_GOING_LEFT;
					ball_position = ball_position<<1;
					pioa->SODR = ball_position;
					xtipimata++;

					if (xtipimata > 10) {
                       			xtipimata = 0;
                      				 timer -= timer_decrease;
                			}
					tc->Channel_0.RC = timer;
					tc->Channel_0.CCR = 0x05;
				}

				else {
					player_1_score++;
					ball_direction = BALL_SHOW_SCORE;
				}
			}
			else {
				ball_position = ball_position>>1;
				pioa->SODR = ball_position;
				tc->Channel_0.CCR = 0x05;
			}
		}
		else if (ball_direction == BALL_GOING_LEFT) {
			if (ball_position == 0x200) {
				if (tmp & BUT_PLAYER_1 == 0) {
					ball_direction = BALL_GOING_RIGHT;
					ball_position = ball_position>>1;
					pioa->SODR = ball_position;
					xtipimata++;
					
					if (xtipimata > 10) {
			                       xtipimata = 0;
        			              	timer -= timer_decrease;
               			}
					tc->Channel_0.RC = timer;
					tc->Channel_0.CCR = 0x05;
				}
				else {
					player_2_score++;
					ball_direction = BALL_SHOW_SCORE;
				}
			}
			else {
				ball_position = ball_position<<1;
				pioa->SODR = ball_position;
				tc->Channel_0.CCR = 0x05;
			}
		}

	        if (ball_direction == BALL_SHOW_SCORE) {
	           pioa->CODR = 0x3FF;
	           if (player_1_score >= 1)
	              pioa->SODR = 0x200;
	           if (player_1_score >= 2)
	              pioa->SODR = 0x200>>1;
	           if (player_1_score >= 3)
	              pioa->SODR = 0x200>>2;
	                           
	           if (player_2_score >= 1)
	              pioa->SODR = 0x001;
	           if (player_2_score >= 2)
	              pioa->SODR = 0x001<<1;
	           if (player_2_score >= 3)
	              pioa->SODR = 0x001<<2;
	
	           tc->Channel_0.RC = 8192;
		    tc->Channel_0.CCR = 0x05;
        	}
     	} // if( fiq & (1<<TC0_ID) )
}


