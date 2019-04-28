#include <msp430.h>
#include <main.h>

/**
 * main.c
 */
int click_cnt1=0;
int click_cnt2=0;

const char ch1Reg = 0x09;
const char ch2Reg = 0x25;
int ocTimeOutB = 0;
int statehandler(int state,int pst,int a);
void init(void);
void init2(void);
void record_position(int pos1,int pos2);

void QEI_Init(void){
    WDTCTL = WDTPW | WDTHOLD;   // stop watchdog timer

	//Set pins as GPIO for QEI, HI edge interrupt enabled.
    P2SEL &=~ (BIT0 + BIT1 + BIT3 + BIT4);
    P2DIR &=~ (BIT0 + BIT1 + BIT3 + BIT4);
    P2OUT |= (BIT0 + BIT1 + BIT3 + BIT4);
    P2IE |= (BIT0 + BIT1 + BIT3 + BIT4); //HI EDGE ONLY INTERRUPT
    P2IES &=~ (BIT0 + BIT1 + BIT3 + BIT4);
    P2IFG &=~ (BIT0 + BIT1 + BIT3 + BIT4);
}

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
//QEI can work with just high edge interrupt. If channel A goes LO->HI, it will interrupt, state of channel B will determine
//which channel is leading.
//
//If B goes LO->HI, this will also count as a click, and the state of A will determine the direction

//Channel A interrupted on HI edge
 if(P2IFG & BIT0){
     //Channel B High -> Decrement (Channel B is Leading)
     if(P2IN & BIT1){
         click_cnt1--;
     }
     //Channel B Low -> Increment (Channel A is Leading)
     else{
         click_cnt1++;
     }
     //Clear Interrupt flag
     storeInt(click_cnt1,ch1Reg);
     P2IFG &=~ BIT0;
 }
//Channel B interrupted on HI Edge
 if(P2IFG & BIT1){
     //Channel A High -> Increment (Channel A is leading)
     if(P2IN & BIT0){
         click_cnt1++;
     }
     //Channel A Low -> Decrement (Channel B is Leading)
     else{
         click_cnt1--;
     }
     //Clear Interrupt flag
     storeInt(click_cnt1,ch1Reg);
     P2IFG &=~ BIT1;
 }



 //Repeat for QEI channel 2
 if(P2IFG & BIT3){
     //Channel B High -> Decrement (Channel B is Leading)
     if(P2IN & BIT4){
         click_cnt2--;
     }
     //Channel B Low -> Increment (Channel A is Leading)
     else{
         click_cnt2++;
     }
     //Clear Interrupt flag
     storeInt(click_cnt2,ch2Reg);
     P2IFG &=~ BIT3;
 }
//Channel B interrupted on HI Edge
 if(P2IFG & BIT4){
     //Channel A High -> Increment (Channel A is leading)
     if(P2IN & BIT3){
         click_cnt2++;
     }
     //Channel A Low -> Decrement (Channel B is Leading)
     else{
         click_cnt2--;
     }
     //Clear Interrupt flag
     storeInt(click_cnt2,ch2Reg);
     P2IFG &=~ BIT4;
 }
 //Hijack port 2 interrupt for overcurrent on Channel B
 if ((P2IFG & BIT6) || (P2IFG & BIT7)){
	 overCurrent(2, 1);
 }
 else{
	 overCurrent(2, 0);
	 ocTimeOutB++;
 }

}
