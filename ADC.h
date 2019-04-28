/*
 * ADC.h
 *
 *  Created on: Feb 23, 2019
 *      Author: Jordan Hardy
 */

#ifndef ADC_H_
#define ADC_H_

void analog_signal_cond(unsigned int * adc);
void ADCInit(void);
extern unsigned int rCurrent;
extern int ocTimeOutB;
extern int ocTimeOut;
#endif /* ADC_H_ */
