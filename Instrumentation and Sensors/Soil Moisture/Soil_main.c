#include <pic18f4550.h>
#include "config.h"
#include "usart.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

void GSM_Init();
void GSM_Response();
void GSM_Msg_Read(int);
void GSM_Send_Msg(const char*);


#define vref 5.0               /*Reference Voltage is 5V*/
#define offset 552

//char *phone_no[3 ]={"+27817163886" ," +27725916226" ,"+27848256343"}; //array of pointers


char *compare();
char *data1,str1[100],str2[100],y[10],*data2,index;
int s,e;

char buff[160];                             /* buffer to store responses and messages */
volatile char status_flag;                  /* monitor to check for any new message */
volatile int a;

int volts;
float dB,dbc,gramms ;  
float digital,test;
int gain;

void ADC_Init();
int ADC_Read(int);
#define THRESHOLD_VALUE 600 //The threshold to turn the led on 400.00*5/1024 = 1.95v

float calibration_factor = -109525; //-106600 worked for my 40Kg max scale setup 

#define DOUT  RA0
#define CLK  RA1
//HX711 scale(DOUT, CLK);

float adc_val;
float wt;
char finaltxt[8];
float quant=5.0/1024;
float get_scale();

char Temperature[10];    
    float celsius;
    #define FLOW_SENSOR_PIN PORTBbits.RB0
//unsigned int flowValue;

const unsigned long pulseCounts[] = {100, 200, 300, 400, 500};  // Corresponding pulse counts
const float flowRates[] = {5.0, 10.0, 15.0, 20.0, 25.0};      // Corresponding flow rates in L/min
const int numCalibrationPoints = sizeof(pulseCounts) / sizeof(pulseCounts[0]);

unsigned long pulseCount = 0;

void MSdelay(unsigned int val)
{
     unsigned int i,j;
        for(i=0;i<val;i++)
            for(j=0;j<165;j++);      /*This count Provide delay of 1 ms for 8MHz Frequency */
 }
float convertToFlowRate(unsigned long pulseCount) {
    // Find the appropriate range in the lookup table
    int index;
    for (index = 0; index < numCalibrationPoints - 1; index++) {
        if (pulseCount < pulseCounts[index + 1]) {
            break;
        }
    }

    // Perform linear interpolation within the range
    float flowRateRange = flowRates[index + 1] - flowRates[index];
    unsigned long pulseCountRange = pulseCounts[index + 1] - pulseCounts[index];
    float flowRate = flowRates[index] + (float)(pulseCount - pulseCounts[index]) * (flowRateRange / pulseCountRange);

    return flowRate;
}


int main() 
{
    
     //INTCON2bits.RBPU=0;   		//To Activate the internal pull on PORTB
	//ADCON1 = 0x0F;				//To disable the all analog inputs	
	//TRISB |= (1 << FLOW_SENSOR_PIN);
	
	//TRISB = 0;				//To configure PORTB  as output
	TRISC = 0x00;				//To configure PORTC  as output
	TRISD = 0x00;				//To configure PORTD  as output	
	TRISE = 0x00;				//To configure PORTE  as output
    
    OSCCON=0x72;                /*Set internal Oscillator frequency to 8 MHz*/
    ADC_Init();                 /*Initialize 10-bit ADC*/
    USART_Init(9600);   /* initialize USART communication */        
    
    
    while(1)
    {   
       
        
        unsigned long currentPulseCount;

        // Disable interrupts while reading pulse count
        INTCONbits.GIE = 0;
        currentPulseCount = pulseCount;
        pulseCount = 0;
        INTCONbits.GIE = 1;

        // Convert pulse count to flow rate
        float flowRate = convertToFlowRate(currentPulseCount);
        
        char strr[144];
            sprintf(strr,"FlowRate: %.2f L/min", flowRate);
           GSM_Send_Msg(strr);

        // Now you can use the calculated flowRate value for your application

        // Add delay before the next reading
      
        
    }

    return 0;
}

void __interrupt() my_isr(void)
{
    if (INTCONbits.INT0F) {
        pulseCount++;
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


