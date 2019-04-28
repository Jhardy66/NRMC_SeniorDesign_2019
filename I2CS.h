/*
 * I2CS.h
 *
 *  Created on: Feb 23, 2019
 *      Author: jorda
 */

#ifndef I2CS_H_
#define I2CS_H_

void  i2c_slave_init(int SL_ADD,unsigned char * passptr);

extern unsigned char i2cTXData[64],i2cRXData[64];
extern volatile int i2cTXData_ptr,i2cRXData_ptr,i2crxflag;
extern volatile int i2cmode;
#endif /* I2CS_H_ */
