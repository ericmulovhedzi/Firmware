/*
 * File:   main.c
 * Author: Eric Mulovhedzi
 *
 * Created on April 4, 2021, 2:55 PM
 */

#include <pic18f4550.h>
#include "Configuration_Header_File.h"

//#include "USART_Header_File.h"
#include "USART_Source_File.c"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#define _XTAL_FREQ 8000000

void display_sequence(void);


/* Define Required XBee Frame Type and Responses */
#define START_DELIMITER                     0x7E
#define AT_COMMAND_FRAME                    0x08
#define TRANSMIT_REQUEST_FRAME              0x10
#define REMOTE_AT_COMMAND_FRAME             0x17
#define IO_DATA_SAMPLE_FRAME                0x92
#define AT_COMMAND_RESPONSE_FRAME           0x88
#define REMOTE_AT_COMMAND_RESPONSE_FRAME    0x97
#define RECEIVE_PACKET_FRAME                0x90
#define TRANSMIT_STATUS_FRAME               0x8B

#define FRAME_ID                            0x01
#define REMOTE_AT_COMMAND_OPT               0x02
#define TRANSMIT_REQUEST_OPT                0x00
#define TRANSMIT_REQUEST_BROADCAST_RADIUS   0x00

#define Read                                0
#define Write                               1


#define BUFFER_SIZE			    	100
#define DIGITAL_BUFFER_SIZE		    16
#define ANALOG_BUFFER_SIZE		    8

uint8_t		ReceiveBuffer[BUFFER_SIZE];
int8_t		DigitalData[DIGITAL_BUFFER_SIZE];
int16_t		AnalogData[ANALOG_BUFFER_SIZE];
uint16_t	BufferPointer	= 0,
			LastByteOfFrame	= 0; 
uint32_t	Command_Value	= 0;

bool 	_IsContainsDigital	= false,
	 	_IsContainsAnalog	= false;	


bool Is_Data_Received()		/* Check wether data received or not  */
{
	for (uint8_t i = 0; i < BUFFER_SIZE; i++)
	{
		if (ReceiveBuffer[i] != 0 && i > 0)
			return true;
	}
	return false;
}

bool Is_Checksum_Correct()	/* Check wether received data is correct */
{
	uint16_t checksum = 0;
	for(uint8_t i = 3; i < LastByteOfFrame; i++)
		checksum = checksum + ReceiveBuffer[i];
	checksum = 0xFF - checksum;
	if(ReceiveBuffer[LastByteOfFrame] == (uint8_t)checksum &&
		LastByteOfFrame > 0)
		return true;
	else
		return false;
}

/* Sample function for parsing received buffer as per frame type */
void sample()
{
    uint8_t Frame_Type, Is_Analog;
    uint16_t Length, Is_Digital, Digital_Value;
	
    _IsContainsAnalog = false;
    _IsContainsDigital = false;

    /* 2 byte Frame length is at 1st and 2nd position of frame */
    Length = ((int)ReceiveBuffer[1]<<8) + ReceiveBuffer[2];
    /* 1 byte Frame type is at 3rd position of frame */
    Frame_Type = ReceiveBuffer[3];

    switch (Frame_Type)
     {
	case (IO_DATA_SAMPLE_FRAME):	/* Parse received I/O data sample frame */
	  if(Is_Data_Received() == false || Is_Checksum_Correct() == false) break;
	  Is_Digital = ((int)ReceiveBuffer[16]<<8) + ReceiveBuffer[17];
	  Is_Analog = ReceiveBuffer[18];
	  Digital_Value = ((int)ReceiveBuffer[19]<<8) + ReceiveBuffer[20];
	
	  if(Is_Analog != 0)
	   _IsContainsAnalog = true;
	  if(Is_Digital != 0)
	   _IsContainsDigital = true;
/****** Check For whether sample contains Analog/Digital Sample *********/		

	  for (uint8_t i = 0; i < DIGITAL_BUFFER_SIZE; i++)
	   {
		if(((Is_Digital >> i) & 0x01) == 1 && ((Digital_Value>>i) & 0x01) != 0)
		 DigitalData[i] = 1;
		else if(((Is_Digital >> i) & 0x01) == 1 && ((Digital_Value>>i) & 0x01) == 0)
		 DigitalData[i] = 0;
		else
		 DigitalData[i] = -1;
	   }
			
	  for (uint8_t i = 0, j = 0; i < ANALOG_BUFFER_SIZE; i++)
	   {
	    if(((Is_Analog >> i) & 0x01) == 1)
	     {
		if(Is_Digital != 0)
		AnalogData[i] = 256 * ReceiveBuffer[21+(j*2)] + ReceiveBuffer[22+(j*2)];
		else
		AnalogData[i] = 256 * ReceiveBuffer[19+(j*2)] + ReceiveBuffer[20+(j*2)];
		j++;
	     }
	    else
	     {
		AnalogData[i] = -1;
	     }
	   }

	case (TRANSMIT_STATUS_FRAME):/* Parse received Transmit status frame */
	  if(Is_Data_Received() ==false || Is_Checksum_Correct() ==false) break;
	  break;	/* Check whether frame is correctly received or not */
	case (RECEIVE_PACKET_FRAME):
	  if(Is_Data_Received() ==false || Is_Checksum_Correct() ==false) break;
	  break;
	case (REMOTE_AT_COMMAND_RESPONSE_FRAME):
	  if(Is_Data_Received() ==false || Is_Checksum_Correct() ==false) break;
	  break;
	case (AT_COMMAND_RESPONSE_FRAME):
	  if(Is_Data_Received() ==false || Is_Checksum_Correct() ==false) break;
	  break;
	default:
	  break;
     }
}

bool Get_Sample()	/* Get sample function */
{
	MSdelay(200);	/* Wait for response */
	for (uint16_t count = 0; Is_Data_Received() == false; count++)
	{
		if(count>15000)
		{
			return false;
		}
	}
    	INTCONbits.GIE=0;	/* Disable global interrupt to parse received data */
	sample();		/* Parse data in sample function */
	memset(ReceiveBuffer,0,BUFFER_SIZE);/* Clear ReceiveBuffer */
    	INTCONbits.GIE=1;	/* Enable Global Interrupt */
	return true;		/* Return success value */
}

void Remote_AT_Command(uint32_t Long_Address_MSB, uint32_t Long_Address_LSB, 
		  uint16_t Short_Address, const char* ATCommand, bool action)
{
	uint16_t Length,Checksum;
	if (action == Write)
	{
		/* Define parameter value depend frame length */
		if(Command_Value > 0x00FFFFFF) Length = 19;
		else if(Command_Value > 0x00FFFF) Length = 18;
		else if(Command_Value > 0x00FF) Length = 17;
		else Length = 16;
	}
	else
		Length = 15;

	Checksum = REMOTE_AT_COMMAND_FRAME + FRAME_ID;/* Calculate Checksum */
	for (int8_t i = 24; i >= 0; i = i-8) 
		Checksum = Checksum + (Long_Address_MSB >> i);
	for (int8_t i = 24 ; i >= 0; i = i-8) 
		Checksum = Checksum + (Long_Address_LSB >> i);
	
	Checksum = Checksum + (Short_Address >> 8) + Short_Address 
		   + REMOTE_AT_COMMAND_OPT + ATCommand[0] + ATCommand[1];
	if (action == Write)
	Checksum = Checksum + (Command_Value >> 24) + (Command_Value >> 16)
		   + (Command_Value >> 8) + Command_Value ;

	/* Subtract checksum lower byte from 0xFF to get 1 byte checksum */
	Checksum = 0xFF - Checksum;

	USART_TxChar(START_DELIMITER);	/* Send frame start with 1 byte Delimiter */
	USART_TxChar(Length >> 8);	/* Send 2 byte length */
	USART_TxChar(Length);
	USART_TxChar(REMOTE_AT_COMMAND_FRAME);	/* Send 1 byte frame type */
	USART_TxChar(FRAME_ID);			/* Send 1 byte frame ID */
	for (int8_t i = 24 ; i >= 0 ; i = i-8)	/* Send 32-bit long destin address MSB */
		USART_TxChar(Long_Address_MSB >> i);
	for (int8_t i = 24 ; i >= 0 ; i = i-8)	/* Send 32-bit long destin address LSB */
		USART_TxChar(Long_Address_LSB >> i);
	USART_TxChar(Short_Address >> 8);	/* Send 16-bit long destination address */
	USART_TxChar(Short_Address);
	USART_TxChar(REMOTE_AT_COMMAND_OPT);	/* Send Option */
	USART_SendString(ATCommand);		/* Send AT command */

	if(action == Write)
	{
		if(Length == 19)		/* Send value */
		{
			USART_TxChar(Command_Value >> 24);
			USART_TxChar(Command_Value >> 16);
			USART_TxChar(Command_Value >> 8);
		}
		if(Length == 18)
		{
			USART_TxChar(Command_Value >> 16);
			USART_TxChar(Command_Value >> 8);
		}
		if(Length == 17) 
			USART_TxChar(Command_Value >> 8);
		USART_TxChar(Command_Value);
	}
	USART_TxChar(Checksum);			/* Send Checksum */
}

void AT_Command(const char* ATCommand, bool action)
{
	uint16_t Length,Checksum;

	if (action == Write)
	{
		if(Command_Value > 0x00FFFFFF) Length = 8;
		else if(Command_Value > 0x00FFFF) Length = 7;
		else if(Command_Value > 0x00FF) Length = 6;
		else Length = 5;
	}
	else
		Length = 4;

	Checksum = AT_COMMAND_FRAME + FRAME_ID + ATCommand[0] + ATCommand[1];
	if (action == Write)
	Checksum = Checksum + (Command_Value >> 24) + (Command_Value >> 16)
		   + (Command_Value >> 8) + Command_Value ;
	Checksum = 0xFF - Checksum;

	USART_TxChar(START_DELIMITER);
	USART_TxChar(Length >> 8);
	USART_TxChar(Length);
	USART_TxChar(AT_COMMAND_FRAME);
	USART_TxChar(FRAME_ID);
	USART_SendString(ATCommand);

	if(action == Write)
	{
		if(Length == 8)		/* Send value */
		{
			USART_TxChar(Command_Value >> 24);
			USART_TxChar(Command_Value >> 16);
			USART_TxChar(Command_Value >> 8);
		}
		if(Length == 7)
		{
			USART_TxChar(Command_Value >> 16);
			USART_TxChar(Command_Value >> 8);
		}
		if(Length == 6) 
			USART_TxChar(Command_Value >> 8);
		USART_TxChar(Command_Value);
	}
	USART_TxChar(Checksum);
}

void Transmit_Request(uint32_t Long_Address_MSB, uint32_t Long_Address_LSB,
		      uint16_t Short_Address, char* str)
{
	uint16_t Length,Checksum;
	Length = 14 + strlen(str);

	Checksum = TRANSMIT_REQUEST_FRAME + FRAME_ID;/* Calculate Checksum */
	for (int8_t i = 24; i >= 0; i = i-8) 
		Checksum = Checksum + (Long_Address_MSB >> i);
	for (int8_t i = 24 ; i >= 0; i = i-8) 
		Checksum = Checksum + (Long_Address_LSB >> i);
	Checksum = Checksum + (Short_Address >> 8) + Short_Address;
	for (int8_t i=0;str[i]!=0;i++)
		Checksum = Checksum + str[i];
	Checksum = 0xFF - Checksum;

	USART_TxChar(START_DELIMITER);
	USART_TxChar(Length >> 8);
	USART_TxChar(Length);
	USART_TxChar(TRANSMIT_REQUEST_FRAME);
	USART_TxChar(FRAME_ID);
	for (int8_t i = 24 ; i >= 0 ; i = i-8)
		USART_TxChar(Long_Address_MSB >> i);
	for (int8_t i = 24 ; i >= 0 ; i = i-8)
		USART_TxChar(Long_Address_LSB >> i);
	USART_TxChar(Short_Address >> 8);
	USART_TxChar(Short_Address);
	USART_TxChar(TRANSMIT_REQUEST_BROADCAST_RADIUS);
	USART_TxChar(TRANSMIT_REQUEST_OPT);
	USART_SendString(str);
	USART_TxChar(Checksum);
}

void Write_AT_Command(char* ATCommand, uint32_t _CommandValue)
{
	Command_Value = _CommandValue;
	AT_Command(ATCommand, Write);
}

void Read_AT_Command(char* ATCommand)
{
	AT_Command(ATCommand, Read);
}

void Write_Remote_AT_Command(uint32_t Long_Address_MSB, uint32_t Long_Address_LSB,
		  uint16_t Short_Address, char* ATCommand, uint32_t _CommandValue)
{
	Command_Value = _CommandValue;
	Remote_AT_Command(Long_Address_MSB, Long_Address_LSB, Short_Address, ATCommand, Write);
}

void Read_Remote_AT_Command(uint32_t Long_Address_MSB, uint32_t Long_Address_LSB,
		  uint16_t Short_Address, char* ATCommand, uint32_t _CommandValue)
{
	Remote_AT_Command(Long_Address_MSB, Long_Address_LSB, Short_Address, ATCommand, Read);
}

//void interrupt ISR()		/* Receive ISR routine */
void __interrupt () ISR(void)
{
    char received_char;
    if(RCIF==1){
        received_char = RCREG;
	/* check if any overrun occur due to continuous reception */
        if(RCSTAbits.OERR)
        {           
            CREN = 0;
            NOP();
            CREN=1;
        }
        if (received_char == START_DELIMITER)
        {
            LastByteOfFrame = BufferPointer;
            BufferPointer = 0;
            ReceiveBuffer[BufferPointer] = received_char;
        }
        else
        {
            BufferPointer++;
            ReceiveBuffer[BufferPointer] = received_char;
        }
    }
}

void SetTo_Broadcast()
{
    Write_AT_Command("+++", 0x0000FFFF);
	MSdelay(500);
	Write_AT_Command("DH", 0x00000000);
	MSdelay(500);
	Write_AT_Command("DL", 0x0000FFFF);
	MSdelay(500);
	Read_AT_Command("WR");
	MSdelay(1000);	
}

int main()
{
    // --- --- --- --- BASICS BLINK CODE --- --- --- ---
    TRISD = 0x00;
    //TRISC1 = 0x00;TRISC0 = 0x00;
    
    //display_sequence();
    
    char _buffer[25];
    double Temperature;
    uint32_t Remote_Address_DH = 0x0013A200;
    uint32_t Remote_Address_DL = 0x41241CB2;

    OSCCON=0x72;		/* set internal clock to 8MHz */
    USART_Init(9600);		/* Initiate USART with 9600 baud rate */
    //USART_Init(9600);
    //LCD_Init();			/* Initialize LCD */
    //LCD_String_xy(1, 0, "X-Bee Network ");
	USART_SendString( "\r\n\r\n");
    USART_SendString( "X-Bee Network \r\n");
    //LCD_String_xy(2, 0, "Demo..!!");
    USART_SendString( "Demo..!!\r\n");
    MSdelay(200);
    //LCD_Clear();
    INTCONbits.GIE=1;		/* enable Global Interrupt */
    INTCONbits.PEIE=1;		/* enable Peripheral Interrupt */
    PIE1bits.RCIE=1;		/* enable Receive Interrupt */	

    //LCD_String_xy(1, 0, "Setting X-Bee to");
    USART_SendString( "Setting X-Bee to\r\n");
    //LCD_String_xy(2, 0, "Broadcast mode  ");
    USART_SendString( "Broadcast mode  \r\n\r\n");
    SetTo_Broadcast();		/* Set XBee coordinator to broadcast mode */
    //LCD_Clear();
	USART_SendString( "\r\n");
    //LCD_String_xy(1, 0, "Request Samples ");
    USART_SendString( "Request Samples\r\n");
    USART_SendString(strcat("DH:", Remote_Address_DH));
	USART_SendString( "\r\n");
    USART_SendString(strcat("DL:", Remote_Address_DL));
	USART_SendString( "\r\n");
    /* Request Samples from remote X-Bee device at 100ms Sample rate */
    Write_Remote_AT_Command(Remote_Address_DH, Remote_Address_DL, 0xFFFE, "IR", 100);
    MSdelay(150);
    //LCD_Clear();

    while (1)
	{
		Get_Sample();
		if (_IsContainsDigital)
		{
			if(DigitalData[2] >= 0)/* Switch status on DIO2 pin */
			{
			  sprintf(_buffer, "Switch = %d   \r\n", DigitalData[2]);
			  //LCD_String_xy(1, 0, _buffer);	/* print on 1st row */
              USART_SendString(_buffer);
			  memset(_buffer, 0, 25);	/* Clear Buffer */
			}
		}
		if (_IsContainsAnalog)
		{
            		Temperature = (double)AnalogData[1] * 0.1171875;
			if(AnalogData[1] >= 0)/* LM35 value on AIN1 pin */
			{
			  sprintf(_buffer, "Temp = %0.1f C  \r\n", Temperature);
			  //LCD_String_xy(2, 0, _buffer);/* print on 2nd row */
              USART_SendString(_buffer);
			  memset(_buffer, 0, 25);	/* Clear Buffer */
			}
		}
	}
    
    display_sequence();
    
    return 0;
}

void display_sequence(void)
{
    while(1)
    {
        PORTDbits.RD0 = 1;PORTDbits.RD1 = 1;
        MSdelay(100);
        PORTDbits.RD0 = 0;PORTDbits.RD1 = 0;
        MSdelay(100);
        //USART_SendString("ERIC ERIC");
        //MSdelay(200);
    }
}
