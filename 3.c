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

#define BUT_IDLE 0
#define BUT_PRESSED 1

#define IDLE 0
#define HOLD 1

void FIQ_handler(void);

PIO* pioa = NULL;
AIC* aic = NULL;
TC* tc = NULL;

unsigned int lsb=0x0;
unsigned int msb=0x0;
unsigned int count=0;	//ελέγχει τη δυνατότητα μέτρησης
unsigned int temp=0;    //ελέγχει το παρατεταμένο πάτημα
unsigned int sys_state = IDLE;
unsigned int button_state = BUT_IDLE;

int main( int argc, const char* argv[] )
{
    char tmp;
    STARTUP; 
    int gen;
    tc->Channel_0.RC = 4096;  //500 msec, για να ξεπεράσουμε το 1 sec με το παρατεταμένο πάτημα
    tc->Channel_0.CMR = 2084;  //λειτουργία ως timer
    tc->Channel_0.IDR = 0xFF; 
    tc->Channel_0.IER = 0x10; 
          
    aic->FFER = (1<<PIOA_ID) | (1<<TC0_ID); 
    aic->IECR = (1<<PIOA_ID) | (1<<TC0_ID); 
    aic->ICCR = (1<<PIOA_ID) | (1<<TC0_ID);
          
    pioa->PER = 0xFFFF;    //ακροδέκτες γενικού σκοπού, 7 για κάθε 7 segment display, 1 για το κοινό dp και ένα για το button
    pioa->PUER = 0x8000;   //θεωρούμε ότι το push button είναι στον ακροδέκτη 10
    pioa->ODR = 0x8000;    //ακροδέκτης εισόδου
    pioa->OER = 0x7FFF;    //ακροδέκτες εξόδου
    pioa->CODR = 0x7FFF;   //αρχικά όλα σβηστά    
    pioa->IER = 0x8000;    //ενεργοποίηση διακοπών από το push button
          
    gen = pioa->ISR; 
    gen = tc->Channel_0.SR;

    tc->Channel_0.CCR = 0x05;
          
    while( (tmp = getchar()) != 'e')
       {          
             
          }
      
    aic->IDCR = (1<<PIOA_ID) | (1<<TC0_ID); 
    tc->Channel_0.CCR = 0x02;      
    CLEANUP;
    return 0;
}
          
void FIQ_handler(void)
{
unsigned int data_in = 0x0;
unsigned int fiq = 0;
unsigned int data_out=0x0;
unsigned int a=0;
unsigned int b=0;

fiq = aic->IPR;   

if( fiq & (1<<PIOA_ID) ) {
	data_in = pioa->ISR; 
	aic->ICCR = (1<<PIOA_ID); 
	data_in = pioa->PDSR;   //ανάγνωση τιμών εισόδου στον PDSR
   
	if( !(data_in & 0x8000)) {  //αν πατήθηκε το push button τότε
  		if(sys_state == IDLE)  //έχουμε αλλαγή κατάστασης
			sys_state = HOLD;
		else  
			sys_state=IDLE;        //αλλιώς η κατάσταση είναι IDLE
		button_state=BUT_PRESSED;  //σημειώνεται η κατάσταση ως πατημένη
		temp=0;                    //αρχικοποιείται το temp με 0 για να αρχίσει να μετρά για παρατεταμένο πάτημα
		}
	else    
		button_state=BUT_IDLE;   //αν δεν πατήθηκε το κουμπί, η κατάστασή του είναι IDLE
	}

if( fiq & (1<<TC0_ID) ){
    data_out = tc->Channel_0.SR; 
    aic->ICCR = (1<<TC0_ID);   
    
    temp++;                 //μετρά χρονικό διάστημα για παρατεταμένο πάτημα του κουμπιού
    
    if(count==1) {         //αν είμαστε σε κατάσταση μέτρησης         
        if (lsb==0x09) {
			lsb=0; 
			if(msb<0x05) 
				msb++;
			else 
				msb=0x0; 
        }
        else
        lsb++;
                        
    }
    
    if((button_state==BUT_PRESSED) && (temp==3)) { //αν έχουμε παρατεταμένο πατημα > 1sec
		lsb=0x0;
        msb=0x0;
        count=0;
        temp=0;
		}
    
    if((sys_state == IDLE) && (count==1)) { //είμαστε σε κατάσταση IDLE και σε δυνατότητα μέτρησης (και όχι παγώματος)
		pioa->CODR =0X080;  
	     	if(lsb==0x00) a=0x01;
	     	if(lsb==0x01) a=0x4F;
	     	if(lsb==0x02) a=0x12;
	     	if(lsb==0x03) a=0x06;
	     	if(lsb==0x04) a=0x4C;
	     	if(lsb==0x05) a=0x24;
	     	if(lsb==0x06) a=0x20;
	     	if(lsb==0x07) a=0x0F;
	     	if(lsb==0x08) a=0x00;
	     	if(lsb==0x09) a=0x04;
	     	if(msb==0x00) b=0x01;
	     	if(msb==0x01) b=0x4F;
	     	if(msb==0x02) b=0x12;
	     	if(msb==0x03) b=0x06;
	     	if(msb==0x04) b=0x4C;
	     	if(msb==0x05) b=0x24;
	     	data_out=(a<<8)|(b);
	     	pioa->CODR =0x7FFF;              //σβήνουμε όλες τις ενδείξεις
		pioa->SODR = data_out;           //ανάβουμε ότι περιέχεται από πριν στον καταχωρητή εξόδου data_out, δηλ. απεικονίζουμε το 0
		}
    else if ((sys_state==HOLD) && (temp!=0)) {//είμαστε σε κατάσταση HOLD
		data_out = pioa->ODSR;                //βάζουμε στη μεταβλητή data_out τα περιεχόμενα (δεδομένα εξόδου) του καταχωρητή ODSR
        if (data_out & 0x080) 
			pioa->CODR =0x080;
        else 
			pioa->SODR =0x080;
		}
    
    if (count==1) 
		count=0;
    else 
		count++;

	tc->Channel_0.CCR = 0x05;
	}

}





