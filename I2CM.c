#include <msp430.h>
#define SDA BIT4 //CHANGE TO 4
#define SCL BIT3 //CHANGE TO 3
#define OUT P3OUT //CHANGE TO 3
#define IN P3IN
#define DIR P3DIR //CHANGE TO 3
#define del() __delay_cycles(100)
#define delay() __delay_cycles(100)
#define ACK  0x00
#define NACK 0x01

unsigned char Read_SCL(void);
unsigned char Read_SDA(void);
unsigned char slAdd = 0x30;

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

/**NRMC Signal Board 2019
 * I2CM.c
 * Created on: Fed 24, 2019
 * Authour: Jordan Hardy
 */


//Initialize I2C master
void i2cm_init(){
	P3SEL &= ~(SCL + SDA);
	P3SEL2 &= ~(SCL + SDA);
	DIR &= ~(SCL + SDA);
	OUT &= ~(SCL + SDA);
}

//Read Clock line
unsigned char readSCL(){
	DIR &= ~SCL;
	return((IN & SCL) != 0);
}

//Read data line


unsigned char readSDA(){
	DIR &= ~SDA;
	return((IN & SDA) != 0);
}

// SCL line to output

void clrSCL(){
	DIR |= SCL;
}

//SDA line to output


void clrSDA(){
	DIR |= SDA;
}



//I2C start condition

void startCond(){
	readSDA();
	delay();
	clrSDA();
	delay();
	clrSCL();
}
//I2C stop condition
void stopCond(){
	clearSDA();
	delay();
	readSDA();
	delay();
	readSDA();
}
//Write a Single bit of data to slave
void writeBit(unsigned char b){
	
	if (b){ readSDA(); }
	else{ clrSDA(); }

	delay();
	readSCL();
	delay();
	clrSCL();
}
//Read single bit of data from slave
unsigned char readBit(){
	unsigned char b;

	readSDA();
	delay();
	readSCL();
	delay();
	b = readSDA();
	delay();
	clrSCL();
	return b;
}

//Write 1 Byte to slave
void writeByte(unsigned char data){
	unsigned char n;

	for (n = 0; n < 8; n++){
		writeBit((data & 0x80) != 0);
		data <<= 1;
	}
	readBit();
}
//Read 1 Byte from slave
unsigned char readByte(){
	unsigned char buff = 0;
	unsigned char n;

	for (n = 0; n < 8; n++){
		buff = (buff << 1) | readBit();
	}

	return buff;
}

//Read array of length nLength from slave starting at Register Adresss

void readBuff(unsigned char *Buff, unsigned char Register, unsigned char nLength){
	unsigned char n;

	startCond();
	writeByte(slAdd << 1);
	writeByte(Register);
	stopCond();

	del();
	startCond();
	del();
	writeByte((slAdd << 1));
		for (n = 0; n < nLength; n++){
			*(Buff + n) = readByte();
			if (n < nLength - 1) writeBit(ACK);
		}
}
//Write Array with length n to slave

void writeBuff(unsigned char data[], int nLength){
	unsigned char n;
	startCond();
	writeByte(slAdd << 1);
	for (n = 0; n < nLength; n++){
		writeByte(*(data + n));
	}
	readBit();
	stopCond();
}
