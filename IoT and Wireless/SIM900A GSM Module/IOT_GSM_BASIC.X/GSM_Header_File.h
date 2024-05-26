

// This is a guard condition so that contents of this file are not included
// more than once.  
#ifndef GSM_HEADER_FILE_H
#define	GSM_HEADER_FILE_H

#include <pic18f4550.h>
#include <xc.h>

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

#endif	/* GSM_HEADER_FILE_H */

