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
unsigned int count=0;	//������� �� ���������� ��������
unsigned int temp=0;    //������� �� ������������ ������
unsigned int sys_state = IDLE;
unsigned int button_state = BUT_IDLE;

int main( int argc, const char* argv[] )
{
    char tmp;
    STARTUP; 
    int gen;
    tc->Channel_0.RC = 4096;  //500 msec, ��� �� ����������� �� 1 sec �� �� ������������ ������
    tc->Channel_0.CMR = 2084;  //���������� �� timer
    tc->Channel_0.IDR = 0xFF; 
    tc->Channel_0.IER = 0x10; 
          
    aic->FFER = (1<<PIOA_ID) | (1<<TC0_ID); 
    aic->IECR = (1<<PIOA_ID) | (1<<TC0_ID); 
    aic->ICCR = (1<<PIOA_ID) | (1<<TC0_ID);
          
    pioa->PER = 0xFFFF;    //���������� ������� ������, 7 ��� ���� 7 segment display, 1 ��� �� ����� dp ��� ��� ��� �� button
    pioa->PUER = 0x8000;   //�������� ��� �� push button ����� ���� ��������� 10
    pioa->ODR = 0x8000;    //���������� �������
    pioa->OER = 0x7FFF;    //���������� ������
    pioa->CODR = 0x7FFF;   //������ ��� ������    
    pioa->IER = 0x8000;    //������������ �������� ��� �� push button
          
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
	data_in = pioa->PDSR;   //�������� ����� ������� ���� PDSR
   
	if( !(data_in & 0x8000)) {  //�� �������� �� push button ����
  		if(sys_state == IDLE)  //������ ������ ����������
			sys_state = HOLD;
		else  
			sys_state=IDLE;        //������ � ��������� ����� IDLE
		button_state=BUT_PRESSED;  //����������� � ��������� �� ��������
		temp=0;                    //�������������� �� temp �� 0 ��� �� ������� �� ����� ��� ������������ ������
		}
	else    
		button_state=BUT_IDLE;   //�� ��� �������� �� ������, � ��������� ��� ����� IDLE
	}

if( fiq & (1<<TC0_ID) ){
    data_out = tc->Channel_0.SR; 
    aic->ICCR = (1<<TC0_ID);   
    
    temp++;                 //����� ������� �������� ��� ������������ ������ ��� ��������
    
    if(count==1) {         //�� ������� �� ��������� ��������         
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
    
    if((button_state==BUT_PRESSED) && (temp==3)) { //�� ������ ������������ ������ > 1sec
		lsb=0x0;
        msb=0x0;
        count=0;
        temp=0;
		}
    
    if((sys_state == IDLE) && (count==1)) { //������� �� ��������� IDLE ��� �� ���������� �������� (��� ��� ���������)
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
	     	pioa->CODR =0x7FFF;              //�������� ���� ��� ���������
		pioa->SODR = data_out;           //�������� ��� ���������� ��� ���� ���� ���������� ������ data_out, ���. ������������� �� 0
		}
    else if ((sys_state==HOLD) && (temp!=0)) {//������� �� ��������� HOLD
		data_out = pioa->ODSR;                //������� ��� ��������� data_out �� ����������� (�������� ������) ��� ���������� ODSR
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





