#include <msp430.h>
#include <main.h>
#include "I2CM.h"
/*
 * ADC.c
 *
 *  Created on: Feb 23, 2019
 *      Author: jorda
 */
#define Motor_Current MAIN_BUFF[0];
unsigned int rcurrent; //current p1.0
unsigned int rfb1; // feedback p1.1
unsigned int rfb2; // feedback 1.2
unsigned int rfb3; // feedback 1.5
unsigned int rmp;  // motor + 1.3
unsigned int rmm;  // motor - 1.4
unsigned int adcval[6];
unsigned char sladcbuf[12];
unsigned int sl_current = 0;
int test = 0;
int k;
int ocTimeOut = 0;


void analog_signal_cond(unsigned int * adc);

void ADCInit(void){
	//Set P1.0-1.5 as inputs
    P1DIR &=~ (BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5);
	//Select 1.0-1.5 for ADC
    P1SEL |= (BIT0 + BIT1 + BIT2 + BIT3 + BIT4 + BIT5);
	//ADC10N interrupt enabled
    ADC10CTL0= ADC10SHT_2 + ADC10ON;        
	//Start from P1.0
    ADC10CTL1 = INCH_0; 
	//P1.0-1.5 ADC enable
    ADC10AE0 = (BIT5 + BIT4 + BIT3 + BIT2 + BIT1 + BIT0);
	//Timer Interrupt Initialization
    CCTL0 = CCIE;
	//320k cclk clock cycles -> 50Hz sample rate
    CCR0 = 320000;
    TACTL = TASSEL_2 + MC_2;
}

#pragma vector=TIMER0_A0_VECTOR
__interrupt void Timer_A (void)
{
	//Loop thru P1.0-1.5
    for (k=0;k<6;k++){
		//Setup CTL0 Register
        ADC10CTL0 &= 0xFD;
		//Select ADC channel (Pin) to read from
        ADC10CTL1 = (k*0x1000);
		//Start ADC conversion
        ADC10CTL0 |= ENC + ADC10SC;
		//wait for ADC to finish
        while (ADC10CTL1 & ADC10BUSY);

		//ADC value for pin in same spot in buffer
        adcval[k]=ADC10MEM;

    }
	//store analog values into MAIN_BUFF
    analog_signal_cond(adcval);

}

#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR(void)
{
   
}

void analog_signal_cond(unsigned int * adc){
    int currentInt;
    int fb1Int;
    int fb2Int;
    int fb3Int;
    int mpInt;
    int mmInt;

    //Set variables to ADC readings (useful for calling if needed)
    currentInt = *(adc);
    fb1Int = *(adc + 1);
    fb2Int = *(adc + 2);
    fb3Int = *(adc + 5);
    mpInt = *(adc + 3);
    mmInt = *(adc + 4);


	//Convert 10 Bit INT to 2 bytes, store in MAIN_BUFF
    SBint2bytes(currentInt,11);
    SBint2bytes(fb1Int,13);
    SBint2bytes(fb2Int,15);
    SBint2bytes(fb3Int,21);
    SBint2bytes(mpInt,17);
    SBint2bytes(mmInt,19);
    int i,j;
  
	//If read current exceeds limitations, flag overcurrent
	if ((currentInt < 10) || (currentInt > 1014)){
		overCurrent(1,1);
		ocTimeOut++;
	}
	else{
		//If current goes back within limitations, Reset OC
		overCurrent(1, 0);
	}
	//do the same for I2C slave analog values
    for(i =0;i<12;i++){
        *(MAIN_BUFF + 29 + i) = sladcbuf[i];
    }
    sl_current = Twobytes2int(sladcbuf[0],sladcbuf[1]);

  



}
