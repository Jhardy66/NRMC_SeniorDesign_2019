/*
 * main.h
 *
 *  Created on: Feb 23, 2019
 *      Author: Jordan Hardy
 */

#ifndef MAIN_H_
#define MAIN_H_

extern unsigned char MAIN_BUFF[53];
extern const int MotorNum;
extern const int controller_type;
extern int desPos;
void SBint2bytes(int input,int k);
long Twobytes2int(char MSB, char LSB);
void storeInt(int ch,char channel);
void overCurrent(int channel,int TF);
#endif /* MAIN_H_ */
