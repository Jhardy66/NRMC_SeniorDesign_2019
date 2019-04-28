#include <msp430.h>
#include <main.h>
#include <math.h>
#include <QEI.h>
#include "PWM.h"
#include "I2CS.h"

#define true 1
#define false 0

int PWMSig;
int spd = 0;
int spdb = 0;
int ps4flg = 0;
int sflg = 0;
const char commandFlg = 0xA0;
int clearPosBool=0,startStop=0;
long int _position_desired = 0;
void command_handler( unsigned char * command,unsigned int k){
    char reg;
    char lb;
    int place;
    int tempBuff;
    int temp;
    int utester;

	//Set register to first byte in recieved buffer
    reg = *(command);
	//Set lb to the lowest "nibble" in that byte
    lb = (reg & 0x0F);
    place = (int) reg;
	//If High "nibble" = A, master sent command byte
    if((reg & 0xF0) == commandFlg){
        if(lb < 0x04){
			//Logic for start/stop and position clear command bits
       if(reg & BIT1){
            click_cnt1 = 0;
            click_cnt2 = 0;
            clearPosBool = 1;
            storeInt(click_cnt1,0x09);
            storeInt(click_cnt2,0x25);
            *(MAIN_BUFF + 0x07) = 0;
            *(MAIN_BUFF + 0x08) = 0;

            *(MAIN_BUFF + 23) = 0;
            *(MAIN_BUFF + 24) = 0;
        }else{
            clearPosBool = 0;
        }

        if(reg & BIT0){

            startStop = 1;
        }else{

            startStop = 0;
        }
        }else{
			//Parse Commands sent during PS4 continuous control
            if(lb == 0x0A){
                spd = 1;
                spdb = 1;
                sflg = 0;
            }
            if(lb == 0x0B){
                spd = -1;
                spdb = 1;
                sflg = 0;
            }
            if(lb == 0x0C){
                spd = 1;
                spdb = -1;
                sflg = 0;
            }
            if(lb == 0x0D){
                spd = -1;
                spdb =-1;
                sflg = 0;

            }
            if(lb == 0x0E){
                spd = 0;
                spdb = 0;
                sflg = 1;
            }

            if(lb == 0x07){
                spd = 0;
                spdb = 0;
            }
            if(lb == 0x04){
                ps4flg = 1;
            }
            if(lb == 0x05){
                ps4flg = 0;
                spd = 0;
                spdb = 0;
                sflg = 1;

            }



        }

    }else{
		//If no command byte, store data in MAIN_BUFF starting at register byte.
        *(MAIN_BUFF + reg) = *(command + 1);
        *(MAIN_BUFF + reg + 1) = *(command + 2);
    }

}