#include <msp430.h> 
#include <math.h>
#include <stdlib.h>
#include <I2CM.h>
#include <I2CS.h>
#include <QEI.h>
#include <ADC.h>
#include <PWM.h>
#include "control_logic.h"
#define sc1 *(MAIN_BUFF + 0x05)
#define sc2 *(MAIN_BUFF + 0x06)



/** NRMC Signal Board 2019
 * main.c
 *
 * Created on April 19, 2019
 * Author: Jordan Hardy
 */

unsigned char MAIN_BUFF[53];
char State = 0;
char SL_State = 0;



const int controller_type = 1;
const int motorNum = 1;

//****Application Dependant Variables****
//Set Overcurrent Mode
const char Mode = 0x00;
//Set Maximum Uptime for PWM Output
int dclimit = 3200;
//Set This Device's I2C Slave Address
unsigned char my_address = 0x30;
//Set desired propotional gains for each PWM channel
int Kp = -20;
int KpB = -20;
//Set Desired Integral gains for each PWM channel
float Ki = 0.1;
float KiB = 0.1;
//Set Gains for what ratio to increase motor speed for diff control
float chA_Inc = 2.0;
float chB_Inc = 1.3;
//Set Gains for what ratio to decrease motor speed for diff control
float chA_dec = 1;
float chB_dec = 0.5;
//Specify how many motors are to be controlled
const int numMotors = 2;
//*****************************************
//Variables for Position control, depend on motor specs
int dcOffset = 24000;
int speedInt = 1024;
int desSpeed = 1024;
float speedDes = 1.0;
float desRatio = 0.0;
float tf = 1.1;
long e = 0;
long eB = 0;
int deadbandIt= 0;
int deadbandItB = 0;
long eraw = 0;
long propOut = 0;
long propOutB = 0;
long erawB = 0;
long intOut = 0;
long satError= 0;
int y = 0;
int yB = 0;
int allowB = 1;
long _posDesB = 0;
long pdtest = 0;
long clktest;
int fullstopA = 0;
int fullstopB = 0;
int allowFWD = 1;
int allowBWD = 1;
int allowFWDB = 1;
int allowBWDB = 1;
int dirA;
int dirB;
unsigned char slqeiBuff[2];
long sl_click_cnt = 0;

int sign(int x);
void initBuff();
void storeInt(int ch, char channel);
void SBint2bytes(int input,int k); //"16 bit integer to 2 bytes"
long Twobytes2int(char MSB, char LSB);
int getDesiredPosition(int channel);
void overCurrent();
long readSlavePosition();
void motorControlLoop();
void overCurrent(int channel,int TF);
int canRun(int mNum);

int main(void)
{
  WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
    //Initialize I2C Master
    i2cm_init();
    //Initialize I2C Slave, input slave address here
    i2c_slave_init(my_address,MAIN_BUFF);
    //Initialize other hardware
    ADCInit();
    QEI_Init();
    //0:Input 1:Output
    PWM_Init(1);
	//Initialize motor desired speed
    initBuff();

	//Set 2.6 and 2.7 to GPIO
    P2SEL &=~ BIT6 + BIT7;
    P2SEL2 &=~ BIT6 + BIT7;
    P2DIR &=~ BIT6 + BIT7;

	
	//Enable Interrupts
	__bis_SR_register(GIE); 
	//Call main feedback loop
        motorControlLoop();


    return 0;
}
void motorControlLoop(){

    float es;
    float eBs;
    float pd;
    float pdb;
    int prevPos;
    int prevPosB;
	int dirA, dirB;
	int Up, UpB;
	float Ki = 0;// -1 * (0.2);
    int Ui,UiB;
	float DR;
	while (1){
		//Check if NOT in PS4 Controller control loop (Continuous control)
		if (!ps4flg){

			//If we are, Limit the duty cycle to 3200 (full speed) times the ratio of the desired speed (0-1024) divided by 1024
			//Ex: 
			//desSpeed = 512
			//speedDes = 512/1024 = 0.5 (50% speed)
			//dclimit = 0.5*3200 = 1600
			desSpeed = Twobytes2int(sc1, sc2);
			speedDes = (float)(desSpeed / speedInt);
			dclimit = (int)floor(speedDes * 3200);

			//Set previous desired positions to desired positions before reading in  new desired position
			//Useful for sensing when desired positions change
			prevPos = _position_desired;
			prevPosB = _posDesB;

			//Retrieve desired positions from MAIN_BUFF
			_position_desired = getDesiredPosition(1);
			_posDesB = getDesiredPosition(2);

			//Calculate error
			eraw = (_position_desired - click_cnt1);
			erawB = (_posDesB - click_cnt2);

			//Check if Desired Positions have changed
			if (prevPosB != _posDesB){

				//Cast errors (INT) to floats
				pd = (float)eraw;
				pdb = (float)erawB;

				//Find the ratio between How far each motor has to go to get to desired position
				desRatio = pd / pdb;

				//DR is used for differential control, must always be more than 1 to slow down channel B
				if (desRatio < 1){
					DR = 1 / desRatio;
				} else{
					DR = desRatio;
				}

			}

			//Calculate Proportional Output 
			Up = Kp*eraw;
			UpB = KpB*erawB;

			//Calculate Integral Output
			Ui = Ui + (int)floor(Ki*eraw);
			UiB = UiB + (int)floor(Ki*erawB);

			//set PI controller output as sum of Integral and Proportional outputs
			e = Up + Ui;
			eB = UpB + UiB;

			//Find which direction each motor needs to go
			dirA = sign(eraw);
			dirB = sign(erawB);

			//Store clicks in MAIN_BUFF
			storeInt(click_cnt2, 25);
			storeInt(click_cnt1, 9);

			//Limit PI Output to +/- dclimit (3200 for full-speed)
			if (e > dclimit) e = dclimit;

			if (e < -1 * dclimit) e = -1 * dclimit;

			//Repeat for Channel B
			if (eB > dclimit) eB = dclimit;

			if (eB < -1 * dclimit) eB = -1 * dclimit;
			//DIFFERENTIAL MOTOR CONTROL

			//Cast both PI outputs to floats for calculating
			es = (float)e;
			eBs = (float)eB;
			if (numMotors == 2){
				//If the error of channel A is larger than channel B times the ratio of how many clicks each must travel,
				//Must either speed up A or slow down B
				if (abs(eraw) > abs(erawB*desRatio)){

					//if the output of channel A is already at the limit, reduce B
					//NOTE: all of these numbers that tune the outputs of the controllers must be tuned to your specific motors
					if (abs(e) >= dclimit){
						eBs = eBs * chB_dec;
					}
					else{
						//If Channel A is not at limit, increase it's speed (Out motor for channel A is much slower than channel B, so the scaling factors are a bid disproportionate)
						es = es * chA_Inc;
					}
				}

				//If the error for channel B is larger, must either speed up B or slow down A
				else if (abs(erawB) > abs(eraw / desRatio)){
					//B at limit, slow down A (Once again since A is so much slower we don't ever want to slow it down, with more even motors this factor would be smaller than 1)
					if (abs(eB) >= dclimit){
						es = es * chA_dec;
					}
					//B is not at limit, speed it up
					else{
						eBs = eBs*chB_Inc;
					}
				}
			}
			//Cast outputs back into INTs so they can be used in the PWM registers
			eB = (int)floor(eBs);
			e = (int)floor(es);


			//Relimit outputs after doing Differential control
			if (e > dclimit) e = dclimit;

			if (e < -1 * dclimit) e = -1 * dclimit;

			//Repeat for Channel B
			if (eB > dclimit) eB = dclimit;

			if (eB < -1 * dclimit) eB = -1 * dclimit;

			//Check if We can Run for Channel A

			//If error of A is within deadband, shut off motors
			if (abs(eraw) < 20){
				deadbandIt++;
				TA1CCTL1 &= ~OUTMOD_7;
				deadbandIt = 0;


			}
		

			// StartStop bit is 0, motors can run when not in deadband
			else{
				if (!startStop) TA1CCTL1 |= OUTMOD_7;
				
			}
			//If StartStop is 1, motors must stop even if outside of deadband
			if (startStop) TA1CCTL1 &= ~OUTMOD_7;
		

			//Repeat Same Logic for Channel B
			if (abs(erawB) < 20){
				deadbandItB++;

				TA1CCTL2 &= ~OUTMOD_7;
				deadbandItB = 0;

			}
			else{
				if (!startStop) TA1CCTL2 |= OUTMOD_7;
			}

			if (startStop) TA1CCTL2 &= ~OUTMOD_7;

			//Add controller output to Duty cycle where motor is stationary (Talon SRX spec)
			y = dcOffset + e;
			yB = dcOffset + eB;
			//Set Duty Cycle Register to controller calculated output
			TA1CCR1 = y;
			TA1CCR2 = yB;
		}
		else{
			//If PS4 Controller continous control is being used

			//Set PWM ON
			TA1CCTL1 |= OUTMOD_7;
			TA1CCTL2 |= OUTMOD_7;

			//if stop flag is flagged, turn PWM off
			if (sflg){
				TA1CCTL1 &= ~OUTMOD_7;
				TA1CCTL2 &= ~OUTMOD_7;
			}

			//if desired speeds are 0, turn PWM off
			if (spd == 0){
				TA1CCTL1 &= ~OUTMOD_7;
			}
			if (spdb == 0){
				TA1CCTL2 &= ~OUTMOD_7;
			}

			//Set PWM DC registers to the deadband Talon 1.5ms DC plus the desired speed/direction of motors times the max duty cycle limit
			//NOTE: -1 is there to flip direction of motor movement to match up with controller.
			TA1CCR1 = 24000 + 3200 * spd*-1;
			TA1CCR2 = 24000 + 1600 * spdb*-1;
		}

	}
}

//FUNCTION: takes a 9-16 bit integer and stores it in MAIN_BUFF as 2 bytes
//Inputs: input: integer to be stored
//        k: place in MAIN_BUFF for first byte to go
void SBint2bytes(int input,int k){
    char lsb,msb;
    lsb = (input & 0xFF);
    msb = ((input >> 8)  & 0xFF);
    *(MAIN_BUFF + k) = msb;
    *(MAIN_BUFF + k + 1) = lsb;
}


//FUNCTION: takes 2 bytes and converts them to an integer
//Inputs:   MSB: Most significant byte
//          LSB: Least significant byte
//Outputs:  output: 9-16 bit integer from 2 bytes passed in
long Twobytes2int(char MSB, char LSB){
    int output = 0;

    output = LSB | (MSB << 8);

    return output;

}


//FUNCTION: Reads desired position from main buffer, converts to INT, returns that value
//Inputs:   Channel: 1-2, which Motor to read for.
//          
//Outputs:  output: Integer value of desired position

int getDesiredPosition(int channel){
    char reg;
    int desired;
    if(channel == 1){
        reg = 0x07;
    }
    else if(channel == 2){
        reg = 23;
    }
    desired = Twobytes2int(*(MAIN_BUFF + reg),*(MAIN_BUFF + reg + 1));

    return desired;

}
//FUNCTION: Processes overcurrent position
//Inputs:   Mode: Which overcurrent mode is selected
//			Channel: which channel overcurrent happened on.
//          
//Outputs:  N/A
void overCurrent(int channel,int TF){
	if (TF){
		if (channel == 1){
			switch (Mode){
				//Clear Position, stop motors
			case 0x00:
				startStop = 1;
				break;
				//Only allow motors to turn way opposite of overcurrent direction
			case 0x01:
				if (dirA > 0){
					allowFWD = 0;
				}
				else{
					allowBWD = 0;
				}
				break;
				//Wait for timeout
			case 0x02:
				if (ocTimeOut < 5000){
					startStop = 1;
					ocTimeOut++;
				}
				else{
					startStop = 0;
				}
				
				break;
				//Ignore Overcurrent
			case 0x04:

				break;
			}
		}
		//Same for channel B
		else{
			switch (Mode){
			case 0x00:
				startStop = 1;
				break;
			case 0x01:
				if (dirB > 0){
					allowB = 1;
				}
				else{
					allowB = -1;
				}
				break;
			case 0x02:

				if (ocTimeOutB < 5000){
					startStop = 1;
					
				}
				else{
					startStop = 0;
					ocTimeOutB = 0;
				}

				break;

			case 0x04:

				break;
			}
		}
	}
	else{
		startStop = 0;
	}

}

//Function: Stores integer into MAIN_BUFF
void storeInt(int ch,char channel){
    SBint2bytes(ch,channel);

}

//Function: Initialized desired speed to full speed
void initBuff(){
    SBint2bytes(1024,5);
}

//Function: returns the sign of an integer.
int sign(int x) {
    return (x > 0) - (x < 0);
}
