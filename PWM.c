#include <msp430.h>
#include "main.h"
#include <math.h>
#include "control_logic.h"
/*	NRMC Signal Board 2019
 * PWM.c
 *
 *  Created on: Feb 24, 2019
 *      Author: Jordan Hardy
 */

long outputT = 320000;

void PWM_Init(int outOrNotIn){
    //Init PWM as OUTPUT
		//Set 2.2,2.5 as outputs, enable PWM
        P2DIR |= (BIT2 + BIT5);
        P2SEL |= (BIT2 + BIT5);

		//Set periood to 20ms, uptime to 1.5ms
		//(Talon SRX specs, 1.5ms uptime = no movement)
		//CCR1: 2.2
		//CCR2: 2.5
        TA1CCR0 = outputT;
        TA1CCR1 = 24000;
		TA1CCR1 = 24000;
		//Set both PWM channels to ON
        TA1CCTL1 |= OUTMOD_7;
		TA1CCTL2 |= OUTMOD_7;
		//Setup PWM hardware
        TA1CTL = TASSEL_2 + MC_1;

}
