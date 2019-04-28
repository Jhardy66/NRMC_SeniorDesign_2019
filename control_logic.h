/*
 * control_logic.h
 *
 *  Created on: Feb 24, 2019
 *      Author: Jordan Hardy
 */

#ifndef CONTROL_LOGIC_H_
#define CONTROL_LOGIC_H_
void command_handler(unsigned char *command,unsigned int k);
extern int PWMSig,spd,spdb,ps4flg;
extern int spd;
extern int spdb;
extern int ps4flg;
extern int sflg;
extern long int _position_desired;
extern int clearPosBool,startStop;
#endif /* CONTROL_LOGIC_H_ */
