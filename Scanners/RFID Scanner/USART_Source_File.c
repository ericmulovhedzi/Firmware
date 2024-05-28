

#include "USART_Header_File.h"

/*****************************USART Initialization*******************************/
void USART_Init(long baud_rate)
{
    float temp;
    TRISC6=0;                       /*Make Tx pin as output*/
    TRISC7=1;                       /*Make Rx pin as input*/
    temp=Baud_value;     
    SPBRG=(int)temp;                /*baud rate=9600, SPBRG = (F_CPU /(64*9600))-1*/
    TXSTA=0x20;                     /*Transmit Enable(TX) enable*/ 
    RCSTA=0x90;                     /*Receive Enable(RX) enable and serial port enable */
}
/******************TRANSMIT FUNCTION*****************************************/ 
void USART_TransmitChar(char out)
{        
        while(TXIF==0);            /*wait for transmit interrupt flag*/
        TXREG=out;                 /*wait for transmit interrupt flag to set which indicates TXREG is ready
                                    for another transmission*/    
}
/*******************RECEIVE FUNCTION*****************************************/
char USART_ReceiveChar()
{

    while(RCIF==0);                 /*wait for receive interrupt flag*/
    return(RCREG);                  /*receive data is stored in RCREG register and return to main program */
}

/*
void USART_SendString(const char *out)
{
   while(*out!='\0')
   {            
        USART_TransmitChar(*out);
        out++;
   }
}*/


void USART_SendString(const char *out)
{
   unsigned char s=0;
	
	while (out[s]!=0){
		
		USART_TxChar (out[s]);
		s++;
	}
}


/*********************************Delay Function********************************/
void MSdelay(unsigned int val)
{
     unsigned int i,j;
        for(i=0;i<=val;i++)
            for(j=0;j<81;j++);      /*This count Provide delay of 1 ms for 8MHz Frequency */
 }


/******************TRANSMIT FUNCTION*****************************************/ 
void USART_TxChar(char out)
{        
        while(TXIF==0);            /*wait for transmit interrupt flag*/
        TXREG=out;    
}
/*******************RECEIVE FUNCTION*****************************************/
char USART_RxChar()
{

    while(RCIF==0);       /*wait for receive interrupt flag*/
    if(RCSTAbits.OERR)
    {           
        CREN = 0;
        NOP();
        CREN=1;
    }
    return(RCREG);   /*receive data is stored in RCREG register and return */
}


/*
void GSM_Init()
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
    //sprintf(url,"AT+HTTPPARA=\"URL\",\%s%d\r\n",http,b++); // 
    //USART_SendString(url);
    USART_SendString("AT+HTTPACTION=0\r\n");  // get session start
    USART_SendString("AT+HTTPREAD\r\n"); // read the data of http server
    USART_SendString("AT+SAPBR=0,1\r\n");  // gprs context is released by network
    USART_SendString("AT+HTTPTERM\r\n");  // terminate http service
    

}*/


void ADC_Init()
{    
    TRISA = 0xff;		/*Set as input port*/
    ADCON1 = 0x0e;  		/*Ref vtg is VDD & Configure pin as analog pin*/    
    ADCON2 = 0x92;  		/*Right Justified, 4Tad and Fosc/32. */
    ADRESH=0;  			/*Flush ADC output Register*/
    ADRESL=0;   
}

int ADC_Read(int channel)
{
    int digital;
    ADCON0 =(ADCON0 & 0b11000011)|((channel<<2) & 0b00111100);

    /*channel 0 is selected i.e.(CHS3CHS2CHS1CHS0=0000)& ADC is disabled*/
    ADCON0 |= ((1<<ADON)|(1<<GO));/*Enable ADC and start conversion*/

    /*wait for End of conversion i.e. Go/done'=0 conversion completed*/
    while(ADCON0bits.GO_nDONE==1);

    digital = (ADRESH*256) | (ADRESL);/*Combine 8-bit LSB and 2-bit MSB*/
    return(digital);
}



