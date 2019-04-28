

/* NRMC Signal Board 2019
 * I2CS.c
 *
 *  Created on: Feb 23, 2019
 *      Author: Jordan Hardy
 */
#include "msp430.h"
#include <control_logic.h>
#include "main.h"


//I2C mode. 0=slave, 1=master
int i2cmode = 0;            
//Slave Address for this MCU. Can be set to any number (Usually between 0x10 and 0x40)
int address = 0x30;         

//Declare various buffers for I2C comms
unsigned char i2cTXData[30],i2cRXData[12];          
unsigned char i2c_command[12];
unsigned char check[12];
volatile int i2cTXData_ptr=0,i2cRXData_ptr=0,i2crxflag=0,i2coffset; 
unsigned char * pMain;
unsigned char p1;
unsigned char p2;


void i2c_slave_init(int address,char * passptr){
    //This function initialized all the hardware settings for I2C slave.
    BCSCTL1 = CALBC1_16MHZ;                    // Set DCO
    DCOCTL = CALDCO_16MHZ;
    P1SEL |= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0
    P1SEL2|= BIT6 + BIT7;                     // Assign I2C pins to USCI_B0

    UCB0CTL1 |= UCSWRST;                      // Enable SW reset
    UCB0CTL0 = UCMODE_3 + UCSYNC;             // I2C Slave, synchronous mode
    UCB0I2COA = address;                      // Own Address is input
    UCB0CTL1 &= ~UCSWRST;                     // Clear SW reset, resume operation
    UCB0I2CIE |= UCSTTIE + UCSTPIE;           // Enable STT interrupt
    IE2 |= (UCB0TXIE+UCB0RXIE);               // Enable TX interrupt
    i2cmode = 0;
    pMain = MAIN_BUFF;
}

#pragma vector=USCIAB0TX_VECTOR
__interrupt void USCI0TX_ISR(void)
{

    if (i2cmode){
		//Master sends register byte, use that as position to start TXing from MAIN_BUFF
        i2coffset = UCB0RXBUF;
       //Set TXbuffer to desired place in buffer
        UCB0TXBUF = *(pMain  + i2coffset + i2cTXData_ptr);
        i2cTXData_ptr++;
        i2crxflag++;
        UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);
        IFG2 &=~ (UCB0TXIFG + UCB0RXIFG);

    }

    else{
        i2coffset = UCB0RXBUF;
        //i2coffset = UCB0RXBUF;
		//Store recieved data into array
        *(i2cRXData + i2cRXData_ptr)=UCB0RXBUF;                         // rx data on i2c slave bus : We're receiving data
        i2cRXData_ptr++;
        i2crxflag++;
    }

}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
   //i2coffset = UCB0RXBUF;
    if(IFG2 & UCB0TXIFG){                           // detect beginning of i2c in slave-master mode
        i2cmode=1;

        if (UCB0STAT&UCSTTIFG){
            i2coffset = UCB0RXBUF;
                UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);
                i2cTXData_ptr=0;
        }   // Increment data
    }
    if(IFG2&UCB0RXIFG){                             // detect beginning of i2c in master-slave mode
        i2cmode=0;
        //i2coffset = UCB0RXBUF;
        if (UCB0STAT&UCSTTIFG){
           // i2coffset = UCB0RXBUF;
            UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);       // Clear interrupt flags

            i2cRXData_ptr=0;
        }   // Increment data


    }
    //IFG2 &=~ UCB0TXIFG;

    if (UCB0STAT&UCSTPIFG){
        i2cmode=0;
       // IFG2 &=~ (UCB0TXIFG + UCB0RXIFG);
        //i2coffset = UCB0RXBUF;
        UCB0STAT &= ~(UCSTPIFG + UCSTTIFG);// I2C Stop condition
		//IF stop condition flags, handle data recieved from master.
        command_handler(i2cRXData,i2cRXData_ptr);
        i2cRXData_ptr=0;

    }   // Increment data




}
