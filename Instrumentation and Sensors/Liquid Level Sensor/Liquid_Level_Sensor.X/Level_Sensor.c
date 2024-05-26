/*
 * File:   Level_Sensor.c
 * Author: Eric Mulovhedzi
 *
 * Created on 21 07 2021, 10:24 AM
 */


#include <pic18f4550.h>
#include "Configuration_Header_File.h"
#include "USART_Header_File.h"
#include "xc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void ADC_Init();
int ADC_Read(int);

void GSM_Init();
void GSM_Response();
void GSM_Msg_Read(int);
//void GSM_Send_Msg(const char* , const char*);
void GSM_Send_Msg(const char*);
void MSdelay(unsigned int);

char *compare();
char *data1,str1[100],str2[100],y[10],*data2,index;
int s,e;

char buff[160];                             /* buffer to store responses and messages */
volatile char status_flag;                  /* monitor to check for any new message */
volatile int a;
int digital;  
float voltage,depth;



#define Tunk 20000 // Depth measuring range 5000mm (for water)
#define WATER 1 // Pure water density normalized to 1
#define PRINT_INTERVAL 1000



    char data[10];    
   
    


#define vref 5.00               /*Reference Voltage is 5V*/


void MSdelay(unsigned int val)
{
     unsigned int i,j;
        for(i=0;i<=val;i++)
            for(j=0;j<81;j++);      /*This count Provide delay of 1 ms for 8MHz Frequency */
 }

void main()
{    
    
    TRISB = 0x00;				//To configure PORTB  as output
	TRISC = 0x00;				//To configure PORTC  as output
	TRISD = 0x00;				//To configure PORTD  as output	
	TRISE = 0x00;				//To configure PORTE  as output
    PORTAbits.RA0 = 1;
    
    char data[10];  
    USART_Init(9600);
   // int digital,depth;  
    //float voltage;
    OSCCON=0x72;                /*Set internal Oscillator frequency to 8 MHz*/
    ADC_Init();                 /*Initialize 10-bit ADC*/

    while(1)
    {        
        digital = ADC_Read(0);
        voltage = ((float)vref/(float)1023);    /*Convert digital value into analog voltage*/  
        
        
        char strr[20];
        sprintf(strr,"Water Level = %d", digital);
        GSM_Send_Msg(strr);
        
        if (voltage > 00.4 && voltage < 00.6){
        PORTBbits.RB0 = 1;
        }else{PORTBbits.RB0 = 0;}
        if (voltage >= 00.6 && voltage < 00.8){
        PORTBbits.RB1 = 1;
        }else{PORTBbits.RB1 = 0;}
        if (voltage >= 00.7 && voltage < 00.9){
        PORTBbits.RB2 = 1;
        }else{PORTBbits.RB2 = 0;}
		MSdelay(500);
    }
    
}

void __interrupt() my_isr(void)
{
    if(RCIF)
    {
        buff[a] = RCREG;                /* read received byte from serial buffer */
        a++;
        
        if(RCSTAbits.OERR)              /* check if any overrun occur due to continuous reception */
        {           
            CREN = 0;
            NOP();
            CREN=1;
        }
        status_flag=1;                  /* use for new message arrival */
    }

}

void GSM_Send_Msg(const char *sms)
{
    int i;
    char sms_buffer[35];
    a=0;
    sprintf(sms_buffer,"\r");
    USART_SendString(sms_buffer);               /*send command AT+CMGS="Mobile No."\r */
    MSdelay(200);
    while(1)
    {
        if(buff[a]==0x3e)                       /* wait for '>' character*/
        {
            a=0;
            memset(buff,0,strlen(buff));
            USART_SendString(sms);              /* send msg to given no. */
            USART_TxChar(0x1a);                 /* send Ctrl+Z then only message will transmit*/
            break;
        }  
        a++;
    }        
    MSdelay(300);
    a=0;
    memset(buff,0,strlen(buff));
    memset(sms_buffer,0,strlen(sms_buffer));
}

void ADC_Init()
{    
    TRISA = 0xff;       /*set as input port*/
    ADCON1 = 0x0e;      /*ref vtg is VDD and Configure pin as analog pin*/    
    ADCON2 = 0x92;      /*Right Justified, 4Tad and Fosc/32. */
    ADRESH = 0;           /*Flush ADC output Register*/
    ADRESL = 0;   
}

int ADC_Read(int channel)
{
    //int digital;
    ADCON0 =(ADCON0 & 0b11000011)|((channel<<2) & 0b00111100);      /*channel 0 is selected i.e (CHS3CHS2CHS1CHS0=0000) 
                                                                      and ADC is disabled i.e ADON=0*/
    ADCON0 |= ((1<<ADON)|(1<<GO));                   /*Enable ADC and start conversion*/
    while(ADCON0bits.GO_nDONE==1);                  /*wait for End of conversion i.e. Go/done'=0 conversion completed*/        
    digital = (ADRESH*256) | (ADRESL);              /*Combine 8-bit LSB and 2-bit MSB*/
    return(digital);
}
    



        