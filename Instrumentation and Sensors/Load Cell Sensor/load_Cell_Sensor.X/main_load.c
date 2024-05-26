/*
 * File:   main_load.c
 * Author: Eric Mulovhedzi
 *
 * Created on 03 November 2021, 10:10 AM
 */

#include <xc.h>
#include <pic18f4550.h>
#include "config.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include <stddef.h>
#include "lcd.h"
#include "delays.h"
#include "keypad.h"

#define HIGH 0x01

#define    ADDO_TRIS          TRISBbits.TRISB0
#define    ADDO_PORT          PORTBbits.RB0    /* If ADC Data Input is connected to RD5 */
#define    ADDO_GetValue()    PORTBbits.RB0
#define    ADDO_SetHigh()     LATBbits.LATB0 = 1
#define    ADDO_SetLow()      LATBbits.LATB0 = 0

#define    ADSCK_TRIS         TRISBbits.TRISB1
#define    ADSCK_PORT         PORTBbits.RB1
#define    ADSCK_LAT          LATBbits.LATB1    
#define    ADSCK_SetHigh()    LATBbits.LATB1 = 1
#define    ADSCK_SetLow()     LATBbits.LATB1 = 0  

#define    LED_PIN            TRISC
#define    GREEN_LED          PORTCbits.RC0
#define    YELLOW_LED         PORTCbits.RC1
#define    RED_LED            PORTCbits.RC2
#define    BUZZER             PORTCbits.RC6
#define    SWITCH_TRIS        TRISBbits.TRISB2
#define    SWITCH_PORT        PORTBbits.RB2

long OFFSET = 0;
float SCALE = 1;

void GSM_Init();
void GSM_Response();
void GSM_Msg_Read(int);
void GSM_Send_Msg(const char*);
char buff[160];                             /* buffer to store responses and messages */
volatile char status_flag;                  /* monitor to check for any new message */
volatile int a;

signed long read_count(void)      
{
    signed short long Count;  
    unsigned char i;
    
    ADDO_TRIS = 1;         
    ADSCK_TRIS = 0;
    ADSCK_PORT = 0;
    Count = 0;
    
    while (ADDO_GetValue());
    
    for (i=0; i<24; i++)
    {
        ADSCK_SetHigh();
        Count = Count << 1;
        ADSCK_SetLow();
        Count = Count | ADDO_GetValue();
    }
    
    for (i=0; i<3; i++)
    {   
        ADSCK_SetHigh();
        __delay_us(300);
        ADSCK_SetLow();
        __delay_us(300);
    }
    
    return (signed long)Count;
}   

long read_average(unsigned char times) {
	long sum = 0;
	for (unsigned char i = 0; i < times; i++) {
		sum += read_count();
		__delay_ms(0);
	}
	return sum / times;
}

double check_weight() {
    return (read_average(10)+9000)/(-335922.3301);
}



void led(int n) {
    switch(n) {
        case 1: 
            if (GREEN_LED == 0) {
                GREEN_LED = 1;
            } else {
                GREEN_LED = 0;
            }
            break;
        case 2: 
            if (YELLOW_LED == 0) {
                YELLOW_LED = 1;
            } else {
                YELLOW_LED = 0;
            }
            break;
        case 3: 
            if (RED_LED == 0) {
                RED_LED = 1;
            } else {
                RED_LED = 0;
            }
            break;
    }         
}

void buzzer(int n) 
{
    int i = 0;
    
    for (i=0;i<n;i++)
    {
        BUZZER = 1;
        delay_ms(100);
        BUZZER = 0;
        delay_ms(100);
    }
}

void interrupt ISR_RB0_low(void) {
    if (INTCONbits.INT0IF)
    {
        int i;
        unsigned char mes;
        char missingMessage[] = "No object found!";
        
        buzzer(1);
        
        lcd_init();
        lcd_write_cmd(0x01);
        
        lcd_write_cmd(0x80);
        for (i=0;i<16;i++) {
            mes = missingMessage[i];
            lcd_write_data(mes);
        }
        
        INTCONbits.INT0IF = 0;
    }
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

void main(void) {
    int i, j;
    double result = 0, weight = 0;
    unsigned char message, message1, value, res;
    unsigned char c1, c2, c3, c4;
    char weightMessage[] = "Weight:"; 
    char newMessage[] = "New weight:";
    char welcomeMessage[] = "Welcome back!";
    char helloMessage[] = "Hello World!";
    char comMessage[] = "Completed!";
    char settingsMessage[] = "[1] Set";
    char resetMessage[] = "[1] Reset";
    char confirmMessage[] = "[1] Enter";
    char modeMessage[] = "[2] Weight";
    char returnMessage[] = "[2] Return";
    char weightValue[], weightRes[];
    
    RCONbits.IPEN =1;
    INTCONbits.GIEH =1;  
    INTCON2bits.INTEDG0 = 0;
    INTCONbits.INT0IE = 1; 
    INTCONbits.INT0IF = 0;
    
    TRISBbits.TRISB0 = 1;
    LED_PIN = 0x00;
    RED_LED = 0;
    YELLOW_LED = 0;
    GREEN_LED = 0;
    BUZZER = 0;

    lcd_init();
    //USART_Init();   /* initialize USART communication */        
    //GSM_Init();         /* check GSM responses and initialize GSM */
    delay_ms(500);
    
    while(1) {
        //check
        lcd_write_cmd(0x01);
        
        lcd_write_cmd(0x80);
        for (i=0;i<13;i++) {
            message = welcomeMessage[i];
            lcd_write_data(message);
        }
        
        lcd_write_cmd(0xC0);
        for (i=0;i<7;i++) {
            message = settingsMessage[i];
            lcd_write_data(message);
        }
        
        lcd_write_cmd(0xCA);
        for (i=0;i<10;i++) {
            message = modeMessage[i];
            lcd_write_data(message);
        }
        
        c1 = getkey();
        if (c1 == '1') {
            while(1) {
                lcd_write_cmd(0x01);
            
                lcd_write_cmd(0x80);
                for (i=0;i<7;i++) {
                    message = weightMessage[i];
                    lcd_write_data(message);
                }
                
                lcd_write_cmd(0x88);
                sprintf(weightRes, "%f", result);
                for (i=0;i<6;i++) {
                    res = weightRes[i];
                    lcd_write_data(res);
                    GSM_Send_Msg(res);
                }

                lcd_write_cmd(0xC0);
                for (i=0;i<9;i++) {
                    message = resetMessage[i];
                    lcd_write_data(message);
                    GSM_Send_Msg(message);
                }
                
                lcd_write_cmd(0xCA);
                for (i=0;i<10;i++) {
                    message = returnMessage[i];
                    lcd_write_data(message);
                    GSM_Send_Msg(message);
                }
                
                c2 = getkey();
                if (c2 == '1') {
                    while(1) {
                        lcd_write_cmd(0x01);
                        lcd_write_data('L');
                        lcd_write_data('o');
                        lcd_write_data('a');
                        lcd_write_data('d');
                        lcd_write_data('i');
                        lcd_write_data('n');
                        lcd_write_data('g');
                        lcd_write_data('.');
                        lcd_write_data('.');
                        lcd_write_data('.');
                        
                        weight = check_weight();
                        lcd_write_cmd(0x01);
                    
                        lcd_write_cmd(0x80);
                        for (i=0;i<11;i++) {
                            message = newMessage[i];
                            lcd_write_data(message);
                        }

                        lcd_write_cmd(0x8C);
                        sprintf(weightValue, "%f", weight);
                        for (i=0;i<6;i++) {
                            value = weightValue[i];
                            lcd_write_data(value);
                            GSM_Send_Msg(value);
                            GSM_Send_Msg(weightValue);
                        }
                        
                        lcd_write_cmd(0xC0);
                        for (i=0;i<9;i++) {
                            message = confirmMessage[i];
                            lcd_write_data(message);
                        }

                        lcd_write_cmd(0xCA);
                        for (i=0;i<10;i++) {
                            message = returnMessage[i];
                            lcd_write_data(message);
                        }

                        c3 = getkey();
                        if (c3 == '1') {
                            result = weight;
                            lcd_write_cmd(0x01);
                            
                            lcd_write_cmd(0x80);
                            for (i=0;i<10;i++) {
                                message = comMessage[i];
                                lcd_write_data(message);
                            }
                            
                            break;
                        } else if (c3 == '2') {
                            break;
                        }
                    }
                } else if (c2 == '2') {
                    break;
                }
            }
        } else if (c1 == '2') {
            while(1) {
                lcd_write_cmd(0x01);
                
                lcd_write_data('C');
                lcd_write_data('h');
                lcd_write_data('e');
                lcd_write_data('c');
                lcd_write_data('k');
                lcd_write_data('i');
                lcd_write_data('n');
                lcd_write_data('g');
                lcd_write_data('.');
                lcd_write_data('.');
                lcd_write_data('.');
                
                lcd_write_cmd(0xC0);
                for (i=0;i<7;i++) {
                    message = weightMessage[i];
                    lcd_write_data(message);
                }
                
                lcd_write_cmd(0xC8);
                sprintf(weightValue, "%f", weight);
                for (i=0;i<6;i++) {
                    value = weightValue[i];
                    lcd_write_data(value);
                    GSM_Send_Msg(weightValue);
                    
                }
                
                weight = check_weight();
                if (weight < (result*30/100)) {
                    led(3);
                    buzzer(5);
                    delay_ms(1000);
                    led(3);
                    delay_ms(100);
                } else if (weight < (result*50/100)) {
                    led(2);
                    delay_ms(1000);
                    led(2);
                    delay_ms(100);
                } else {
                    led(1);
                    delay_ms(1000);
                    led(1);
                    delay_ms(100);
                }

                delay_ms(300);
            }
        }
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