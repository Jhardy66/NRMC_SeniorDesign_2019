/*
 * I2CM.h
 *
 *  Created on: Feb 23, 2019
 *      Author: Jordan Hardy
 */

#ifndef I2CM_H_
#define I2CM_H_

#define SDA BIT7 //CHANGE TO 4
#define SCL BIT6 //CHANGE TO 3
#define OUT P2OUT //CHANGE TO 3
#define IN P2IN
#define DIR P2DIR //CHANGE TO 3
#define del() __delay_cycles(100)
#define dellay() __delay_cycles(100)
#define ACK  0x00
#define NACK 0x01

void i2cm_init();

unsigned char readSCL();
unsigned char readSDA();

void clrSCL();
void clrSDA();

void startCond();
void stopCond();

void writeBit(unsigned char b);
unsigned char readBit();

void writeByte(unsigned char data);
unsigned char readByte();


void readBuff(unsigned char *Buff, unsigned char Register, unsigned char nLength);
void writeBuff(unsigned char data[], int nLength);


#endif /* I2CM_H_ */
