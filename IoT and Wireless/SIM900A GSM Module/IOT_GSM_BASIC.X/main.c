/*
 * File:   newmain.c
 * Author: Eric Mulovhedzi
 *
 * Created on March 15, 2024, 12:36 PM
 */

// PIC18F4550 Configuration Bit Settings

//#include <xc.h>

#include <pic18f4550.h>
#include "Configuration_Header_File.h"

//#include "USART_Header_File.h"
#include "USART_Source_File.c"

//#include "GSM_Header_File.h"
//#include "GSM_Source_File.c"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define _XTAL_FREQ 8000000


void display_sequence(void);
void display_adc(void);

void main(void) 
{
    // --- --- --- --- BASICS BLINK CODE --- --- --- ---
    TRISD = 0x00;
    TRISC1 = 0x00;TRISC0 = 0x00;
    //display_sequence();
    
    
    // --- --- --- --- GSM CODE --- --- --- ---
    
    a=0;
    OSCCON =0x72;  /* set internal oscillator Freq = 8 MHz*/
    
    INTCONbits.GIE=1;  /* enable Global Interrupt */
    INTCONbits.PEIE=1;  /* enable Peripheral Interrupt */
    PIE1bits.RCIE=1;  /* enable Receive Interrupt */
    MSdelay(100);
    USART_Init(9600);  /* initialize USART communication */        
    
    //GSM_Init();  /* check GSM responses and initialize GSM */
    
   // MSdelay(300);
    //GSM_Send_Msg("+27716281997","TESTER-GSM-IOT");  /*send sms on "mobile no."*/
    //GSM_Send_Msg("+27817163886","TEST GSM IOT SMS");  /*send sms on "mobile no."*/
    //MSdelay(300);
    //GSM_Calling("+27817163886");
    //MSdelay(300);
    //USART_SendString("ERIC ERIC");
    
    //display_sequence();
    
    // --- --- --- --- ADC CODE --- --- --- ---
    
    //ADC_Init();
    //display_adc();
    
    return;
}

void display_sequence(void)
{
    while(1)
    {
        PORTDbits.RD0 = 1;PORTDbits.RD1 = 1;
        MSdelay(1000);
        PORTDbits.RD0 = 0;PORTDbits.RD1 = 0;
        MSdelay(1000);
        //USART_SendString("ERIC ERIC");
        //MSdelay(200);
    }
}


void GSM_Initx()
{   
    
    USART_SendString("\r\nAT\r\n");
	MSdelay(500);
	USART_SendString("ATE1\r\n");
	MSdelay(500);
	USART_SendString("AT+CMGF=1\r\n");
	MSdelay(500);
	USART_SendString("AT+CMGD=1,4\r\n");
	MSdelay(500);
	USART_SendString("AT+CNMI=1,1,0,1\r\n");
    MSdelay(2000);
    USART_SendString("AT+CBAND=1\r\n");
    
    USART_SendString("AT+SAPBR=3,1,\"CONTYPE\",\"GPRS\"\r\n"); // Bearer Settings for Applications Based on IP
    USART_SendString("AT+SAPBR=3,1,\"APN\",\"Airteliot.com\"\r\n"); // Bearer Settings for Applications Based on IP
    USART_SendString("AT+SAPBR=1,1\r\n");    //TO OPEN A GPRS CONTEXT
    USART_SendString("AT+SAPBR=2,1\r\n");  // TO Quary the GPRS context
    
    USART_SendString("AT+HTTPINIT\r\n");   // init http service
    USART_SendString("AT+HTTPPARA=\"CID\",1\r\n"); // set parameters for http session
    USART_SendString("AT+HTTPACTION=0\r\n");  // get session start
    USART_SendString("AT+HTTPREAD\r\n"); // read the data of http server
    USART_SendString("AT+SAPBR=0,1\r\n");  // gprs context is released by network
    USART_SendString("AT+HTTPTERM\r\n");  // terminate http service
    

}

void GSM_Init()
{   
    
    while(1)
    {   
        USART_SendString("ATE0\r");  
        MSdelay(500);
        
        //if(strcmp(strstr(buff,"OK"), "OK") == 0)
        if(strstr(buff,"OK"))
        {
            //USART_SendString("Yepee");
            GSM_Response();  
            memset(buff,0,160);
            break;
        }
        else
        {    
            //LCD_String("Error");
            //USART_SendString("Error"); 
            //MSdelay(500);
        }
        //MSdelay(500);
        //USART_SendString("looping");
    }
    MSdelay(500);

    //USART_SendString("ATE0\r");  /* send AT to check module is ready or not*/
    //GSM_Response();
	//MSdelay(500);
        
        
	USART_SendString("AT+CMGF=1\r");  /* select message format as text */
    GSM_Response();
	MSdelay(300);
    
    USART_SendString("AT+GMI\r");  /* identify manufacturer */
    GSM_Response();
	MSdelay(300);
                                                       
	USART_SendString("AT+GMM\r");  /* find model no. */
    GSM_Response();
	MSdelay(300);
    
	USART_SendString("AT+GSN\r");  /* find IMEI no. of module */
    GSM_Response();
	MSdelay(300);
        
	USART_SendString("AT+CSPN?\r");  /* find service provider name */
    GSM_Response();
	MSdelay(300);

}

void GSM_Response()
{
    unsigned int timeout=0;
    int CRLF_Found=0;
    char CRLF_buff[2];
    int Response_Length=0;
    while(1)
    {
        if(timeout>=60000)  /*if timeout occur then return */
            return;
        Response_Length = strlen(buff);		
        if(Response_Length)
        {
            MSdelay(1);
            timeout++;
            if(Response_Length==strlen(buff))	
            {
                for(int i=0;i<Response_Length;i++)
                {
                    memmove(CRLF_buff,CRLF_buff+1,1);
                    CRLF_buff[1]=buff[i];
                    if(strncmp(CRLF_buff,"\r\n",2))
                    {
                        if(CRLF_Found++==2)	/* search for \r\n in string */
                        {
                            GSM_Response_Display();
                            return;
                        }
                    }

                }
                CRLF_Found =0;
            }
    }
    MSdelay(1);
    timeout++;
 }
}

void GSM_Response_Display()
{
    a=0;
    while(1)
    {
        if(buff[a]==0x0d || buff[a]==0x0a)
        {
            a++;
        }
        else
            break;
    }
    //LCD_String_xy(1,0,"                   ");
    //USART_SendString("                   ");  
    //LCD_Command(0xc0);
    while(buff[a]!=0x0d)
    {  
        //LCD_Char(buff[a]);
        //USART_SendString(buff[a]);
        USART_TxChar(buff[a]);
        a++;
        //if(a==15)
            //LCD_Command(0x80);
    }
    a=0;
        memset(buff,0,strlen(buff));
        
}

void GSM_Send_Msg(const char *num,const char *sms)
{
    int i;
    char sms_buffer[35];
    a=0;
    sprintf(sms_buffer,"AT+CMGS=\"%s\"\r",num);
    USART_SendString(sms_buffer);  /*send command AT+CMGS="Mobile No."\r */
    MSdelay(200);
    while(1)
    {
        if(buff[a]==0x3e)  /* wait for '>' character*/
        {
            a=0;
            memset(buff,0,strlen(buff));
            USART_SendString(sms);  /* send msg to given no. */
            USART_TxChar(0x1a);  /* send Ctrl+Z then only message will transmit*/
            break;
        }  
        a++;
    }        
    MSdelay(300);
    a=0;
    memset(buff,0,strlen(buff));
    memset(sms_buffer,0,strlen(sms_buffer));
}

void GSM_Calling(char *mobile)
{
    char call[20];
    sprintf(call,"ATD%s;\r",mobile);  /* send command ATD8007xxxxxx; for calling*/
    USART_SendString(call);    
}

void __interrupt () ISR(void)
{
    //PORTCbits.RC1 = 1;MSdelay(100);PORTCbits.RC1 = 0;MSdelay(100); // -- print out
    
    //if (INTCONbits.INT0F)
    //{
        //pulseCount++;
    //    INTCONbits.INT0F = 0;
    //}
    
    if(RCIF)
    {
        buff[a] = RCREG;  // read received byte from serial buffer
        a++;
        
        if(RCSTAbits.OERR)  // check if any overrun occur due to continuous reception
        {           
            CREN = 0;
            NOP();
            CREN=1;
        }
        status_flag=1;  // use for new message arrival 
    }

}
/*
void __interrupt() my_isr(void)
{
    if (INTCONbits.INT0F) {
        //pulseCount++;
        INTCONbits.INT0F = 0;
    }
    if(RCIF)
    {
        buff[a] = RCREG;                
        a++;
        
        if(RCSTAbits.OERR)              
        {           
            CREN = 0;
            NOP();
            CREN=1;
        }
        status_flag=1;                  
    }

}

*/


// --- --- --- ADC Code --- --- ---

void display_adc(void)
{
    char data_01[10];char data_02[10];    
    int digital_01; int digital_02;  
    float voltage_01;float voltage_02;
    USART_SendString("Voltage is...\r\n\r\n");
    while(1)
    {
        digital_01=ADC_Read(0);
        digital_02=ADC_Read(1);
        
        /*Convert digital value into analog voltage*/
        voltage_01= digital_01*((float)vref/(float)1023);
        voltage_02= digital_02*((float)vref/(float)1023);   

        /*It is used to convert integer value to ASCII string*/
        sprintf(data_01,"%.2f",voltage_01);
        sprintf(data_02,"%.2f",voltage_02);

        strcat(data_01," V\r\n");	/*Concatenate result and unit to print*/
        MSdelay(300);
        strcat(data_02," A\r\n");	/*Concatenate result and unit to print*/
        //LCD_String_xy(2,4,data);/*Send string data for printing*/ ;
        USART_SendString(data_01);
        //USART_SendString(data_02);c
    }
}






