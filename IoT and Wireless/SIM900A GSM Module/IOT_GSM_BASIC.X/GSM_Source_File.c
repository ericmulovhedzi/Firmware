

#include "USART_Header_File.h"
#include "GSM_Header_File.h"

void GSM_Init()
{   
    while(1)
    {   
        //LCD_Command(0xc0);
        USART_SendString("ATE0\r");  /* send AT to check module is ready or not*/
        MSdelay(500);
        if(strstr(buff,"OK"))
        {
            GSM_Response();  /* find response and display it on LCD16x2 */
            memset(buff,0,160);
            break;
        }
        else
        {    
            //LCD_String("Error");
        }
    }
    MSdelay(2000);

    //LCD_Clear();
	//LCD_String_xy(0,0,"Text Mode"); 
    //LCD_Command(0xc0);
	USART_SendString("AT+CMGF=1\r");  /* select message format as text */
    GSM_Response();
	MSdelay(3000);
    
    //LCD_Clear();
	//LCD_String_xy(0,0,"Mfd name");  
    //LCD_Command(0xc0);
	USART_SendString("AT+GMI\r");  /* identify manufacturer */
    GSM_Response();
	MSdelay(3000);
    
    //LCD_Clear();
	//LCD_String_xy(0,0," Model No.");
    //LCD_Command(0xc0);
	USART_SendString("AT+GMM\r");  /* find model no. */
    GSM_Response();
	MSdelay(3000);
    
	//LCD_Clear();
	//LCD_String_xy(0,0,"  IMEI No. ");
    //LCD_Command(0xc0);
	USART_SendString("AT+GSN\r");  /* find IMEI no. of module */
    GSM_Response();
	MSdelay(3000);
        
    //LCD_Clear();
	//LCD_String_xy(0,0,"Service Provider");
    //LCD_Command(0xc0);
	USART_SendString("AT+CSPN?\r");  /* find service provider name */
    GSM_Response();
	MSdelay(3000);

}

void GSM_Msg_Delete(unsigned int position)
{
    
    a=0;
    char delete_cmd[20];    
    sprintf(delete_cmd,"AT+CMGD=%d\r",position);  /* delete message at specified position */
    USART_SendString(delete_cmd);
    MSdelay(100);
    memset(buff,0,strlen(buff));
}
void GSM_Wait_for_Msg()
{
    char i,val[4];
    int position;
    //LCD_Clear();
    a=0;
    while(1)
    {
        if(buff[a]==0x0d || buff[a]==0x0a)  /*eliminate "\r \n" which is start of string */
        {
            a++;
        }
        else
            break;
    }
    
    if(strstr(buff,"CMTI:"))  /* check if any new message received */
    {   
        while(buff[a]!=',')
        {
            a++;
        }
        a++;
        
        i=0;
        while(buff[a]!=0x0d)
        {
            val[i]=buff[a];
            a++;
            i++;
        } 
    position = atoi(val);
    if(position>20)
    {
        //LCD_String_xy(0,0,"Msg mem full");
        memset(buff,0,strlen(buff));
        return;
    }
    memset(buff,0,strlen(buff));
    a=0;
    GSM_Msg_Read(position);  /* read message which is recently arrived from position */
    }    
}

void __interrupt() my_isr(void)
{
    if (INTCONbits.INT0F) {
        //pulseCount++;
        INTCONbits.INT0F = 0;
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
/*
void __interrupt ISR(void)
{
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

void GSM_HangCall()
{
    USART_SendString("ATH\r");  /*send command ATH\r to hang call*/
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
    //LCD_Command(0xc0);
    while(buff[a]!=0x0d)
    {  
        //LCD_Char(buff[a]);
        a++;
        if(a==15)
            //LCD_Command(0x80);
    }
    a=0;
        memset(buff,0,strlen(buff));
        
}

void GSM_Msg_Read(int position)
{
    int i,k;
    char flag,read_cmd[10];
    i=0;
    sprintf(read_cmd,"AT+CMGR=%d\r",position);
    USART_SendString(read_cmd);                 
    MSdelay(1000);
    GSM_Msg_Display();
}

void GSM_Msg_Display()
{
    
    if(!(strstr(buff,"+CMGR")))  /*check for +CMGR response */
    {
            //LCD_String_xy(0,0,"No message");   
    }   
    else
    {   
        a=0;
        
        while(1)
        {
            if(buff[a]==0x0d || buff[a]==0x0a)  /*wait till \r\n not over*/
            {
                a++;
            }
            else
                break;
        }
        while(buff[a]!=0x3a)  /*wait till string not equal to ':' */
        {
            a++;
        }
     do
        {        
            a++;
        }while(buff[a-1]!=0x0a);
        
        //LCD_Command(0xC0);
        int i=0;
            while(buff[a]!=0x0d && i<31)
            {
                    //LCD_Char(buff[a]);
                    a++;
                    i++;
                    if(i==16)  /* if received message is greater than 16(for display message)*/
                        //LCD_Command(0x80);  /* resume display of message from 1st line */
                    
            }
        
            a=0;
            memset(buff,0,strlen(buff));
    }
    status_flag = 0;
}