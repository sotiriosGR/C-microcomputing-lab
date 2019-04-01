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
#define BUT_IDLE     0
#define BUT_PRESSED  1
void FIQ_handler(void);

PIO* pioa = NULL;
AIC* aic = NULL;
TC* tc = NULL;
int temp0=0;
int temp1=0;
int temp2=0;
int temp3=0;
int temp4=0;
int temp5=0;
int op_code=0;
int op_code1=0;
int op_code2=0;
int check=0;
int check2=0;
int check3=0;

int main( int argc, const char* argv[] ){

unsigned int gen;
char tmp;

STARTUP;
tc->Channel_0.RC = 2035;
tc->Channel_0.CMR = 2084; 
tc->Channel_0.IDR = 0xFF; 
tc->Channel_0.IER = 0x10;
aic->FFER = (1<<PIOA_ID) | (1<<TC0_ID); 
aic->IECR = (1<<PIOA_ID) | (1<<TC0_ID); 
pioa->PUER = 0xC0;
pioa->ODR =0xC0;  
pioa->CODR = 0x3F; 
pioa->OER = 0x3F; 
gen = pioa->ISR; 
pioa->PER = 0xFF; 
gen = tc->Channel_0.SR; 
aic->ICCR = (1<<PIOA_ID)|(1<<TC0_ID); 
pioa->IER = 0xC0;


// Thetw thn grammh 15 sto kokkino tou F1 kai thn anavw.
// Thetw thn grammh 11 sto prasino tou F2 kai thn anavw.
// Thetw thn grammh 16 sto kitrino tou F3 alla gia twra thn krataw kleisth.
pioa->SODR=0x11;  

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
unsigned int data_in = 0;
unsigned int fiq = 0;
unsigned int data_out;


fiq = aic->IPR;

if( fiq & (1<<PIOA_ID) ){
  data_in = pioa->ISR;
  aic->ICCR = (1<<PIOA_ID);
  data_in = pioa->PDSR;
	if(!(data_in & 0x80)){       //button start
		op_code=1;                     //Auto einai gia to Start.
		}
	if(!(data_in & 0x40)){
    	op_code=0;     //Auto einai gia to Stop.
        op_code1=1;
		}	
  }

if( fiq & (1<<TC0_ID) ){
    data_out = tc->Channel_0.SR; 
    aic->ICCR = (1<<TC0_ID);     
    data_out = pioa->ODSR;
	

	if(op_code==1){  //katastash K1
           temp1++;  // Einai gia to anavosvhma tou F3. 
           pioa->SODR=0x20; // Anavei to Kitrino sto F3.
           if(temp1==2){
              temp1=0;
              pioa->CODR=0x20; // Svhvei to Kitrino sto F3.
             }
           if(temp2<40){
              temp2++;  // Einai gia thn K2.
             }
           if(temp2==40){	//katastash K2.
	                         
	         check++;
	         if(check<2){
	          pioa->CODR=0x01;
	          pioa->SODR=0x02;
	          }
              if(temp3<12){
                 temp3++;  // Einai gia thn K3.
                }
              if(temp3==12){	//katastash K3.
	            check2++;  
	            if(check2<2){
	            pioa->CODR=0x02;
	            pioa->SODR=0x04;
	            }	         
                 if(temp4<8){
                    temp4++;  // Einai gia thn K4.
                   }
                 if(temp4==8){ // Katastash K4.
                   check3++;
                   if(check3<2){
                    pioa->CODR=0x10;
	                pioa->SODR=0x08;
	                }                   
                    if(temp5<40){
                       temp5++;  // Einai gia thn K5.
                      }
                    if(temp5==40){ // Katastash K5.
                       op_code2=1;
                      }                
                   } 
                } 
             }
	}
       if(op_code2==1 && op_code1==1){ // Katastash K6.
          pioa->CODR=0x28;
	  pioa->SODR=0x10; 
          if(temp0<20){
             temp0++;  // Einai gia thn epistrofh sthn K0.
            }
          if(temp0==20){ 
             temp0=0;
             temp1=0; 
             temp2=0;
             temp3=0;
             temp4=0; 
             temp5=0;
             op_code2=0;
             op_code1=0;
             check=0;
             check2=0;
             check3=0;  
             pioa->CODR=0x3F;
	     pioa->SODR=0x11;
            }
         }  
tc->Channel_0.CCR = 0x05;
}

}	
