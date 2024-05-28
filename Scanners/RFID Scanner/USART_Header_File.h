

// This is a guard condition so that contents of this file are not included
// more than once.  
//#ifndef USART_HEADER_FILE_H
//#define	USART_HEADER_FILE_H

#ifndef __usart_feader_file_h__
#define __usart_feader_file_h__

//#include <pic18f4550.h>
//#include <xc.h>

void USART_Init(long);
void USART_TransmitChar(char);
void USART_SendString(const char *);
void MSdelay(unsigned int val);
char USART_ReceiveChar();

void USART_TxChar(char);
char USART_RxChar();

#define F_CPU 8000000/64
//#define Baud_value(baud_rate) (((float)(F_CPU)/(float)baud_rate)-1)
#define Baud_value (((float)(F_CPU)/(float)baud_rate)-1)

#define vref 5.00		/* Reference Voltage is 5V*/

//#define URL	


void ADC_Init();
int ADC_Read(int);

void GSM_Init();
void GSM_Calling(char *);
void GSM_HangCall();
void GSM_Response();
void GSM_Response_Display();
void GSM_Msg_Read(int);
void GSM_Wait_for_Msg();
void GSM_Msg_Display();
void GSM_Msg_Delete(unsigned int);
void GSM_Send_Msg(const char* , const char*);

char buff[160];  /* buffer to store responses and messages */
volatile char status_flag;  /* monitor to check for any new message */
volatile int a;

//char http[] = "http://api.thingspeak.com/update"; // ADD YOUR URL 
//char url[150];
//int b=1;

#endif	/* USART_HEADER_FILE_H */

