
#include <MCP_ADC.h>

MCP3208 mcp1(12, 13, 14); //(dataIn, dataOut, clock)
MCP3208 mcp2(12, 13, 14);

const byte CS1 = 15; //select adc 1c
const byte CS2 = 5;	//select adc 2

#define spiSpeed 2000000




/*
ADC configuration
*/
//array size can be edited for difrent signals acording to speed and memory requirments
#define LSA 10 //array size for low speed signals like voltage rails and temperature
#define MSA 50 //medium speed signal array size 
#define HSA 200 //high speed signal array size

//ADC solar and battery/inverter
//	!!! if a signal speed requirement is changed also change it in the ADC section !!!
//  !!! Best to keep it alone
uint16_t solar_input_current_array[MSA]; //CH0
uint16_t battery_output_current_array[MSA]; //CH1
uint16_t battery_input_current_array[MSA]; //CH2
uint16_t V24_array[LSA]; //CH3
uint16_t solarV_array[MSA]; //CH4
uint16_t V12_array[LSA]; //CH5
uint16_t V5_array[LSA]; //CH6
uint16_t temp1_array[LSA]; //CH7

//ADC 230V measurement
uint16_t V5H_array[LSA]; //CH0
uint16_t temp2_array[LSA]; //CH1

int16_t IAC2_array[HSA]; //CH2
int16_t VAC22_array[HSA]; //CH3
int16_t VAC21_array[HSA]; //CH4

int16_t VAC11_array[HSA]; //CH5
int16_t VAC12_array[HSA]; //CH6
int16_t IAC1_array[HSA]; //CH7

//to keep track of current array position
uint16_t LSAL = 0;
uint16_t MSAL = 0;
uint16_t HSAL = 0;

void ADCsetup()
{
	//ADC set select pins
	mcp1.begin(CS1);
	mcp1.setSPIspeed(spiSpeed);

	mcp2.begin(CS2);
	mcp2.setSPIspeed(spiSpeed);
}

void ADCslow()
{
	if (LSAL == LSA - 1) { LSAL = 0; }

	V24_array[LSAL] = mcp1.analogRead(3);

	V12_array[LSAL] = mcp1.analogRead(5);
	V5_array[LSAL] = mcp1.analogRead(6);
	temp1_array[LSAL] = mcp1.analogRead(7);


	V5H_array[LSAL] = mcp2.analogRead(0);
	temp2_array[LSAL] = mcp2.analogRead(1);

	LSAL++;
}

void ADCmedium()
{
	if (MSAL == MSA - 1) { MSAL = 0; }

	solar_input_current_array[MSAL] = mcp1.analogRead(0);
	battery_output_current_array[MSAL] = mcp1.analogRead(1);
	battery_input_current_array[MSAL] = mcp1.analogRead(2);

	solarV_array[MSAL] = mcp1.analogRead(4);

	MSAL++;
}

void ADCfast()
{
	if (HSAL == HSA - 1) { HSAL = 0; }

	IAC2_array[HSAL] = mcp2.analogRead(2);
	VAC22_array[HSAL] = mcp2.analogRead(3);
	VAC21_array[HSAL] = mcp2.analogRead(4);
	VAC11_array[HSAL] = mcp2.analogRead(5);
	VAC12_array[HSAL] = mcp2.analogRead(6);
	IAC1_array[HSAL] = mcp2.analogRead(7);

	HSAL++;
}

//true rms values
uint16_t IAC1trms;
uint16_t VAC1trms;

uint16_t IAC2trms;
uint16_t VAC2trms;

//calc diff of 2 arrays from 1 side of mains and then calc true rms voltage and current
void calcTrueRMS()
{
	IAC1trms = truerms(IAC1_array);
	IAC2trms = truerms(IAC2_array);
	
	//diffrence between 2 whole arrays
	int16_t VAC1_array[HSA];
	for (uint16_t i = 0; i < (sizeof(VAC12_array) / sizeof(VAC12_array[0])); i++)
	{
		VAC1_array[i] = VAC11_array[i] - VAC12_array[i];
	}
	VAC1trms = truerms(VAC1_array);

	int16_t VAC2_array[HSA];
	for (uint16_t i = 0; i < (sizeof(VAC22_array) / sizeof(VAC22_array[0])); i++)
	{
		VAC2_array[i] = VAC21_array[i] - VAC22_array[i];
	}
	VAC2trms = truerms(VAC2_array);
}

//convert and send all data
void sendAllData()
{


}

//calculate true rms from an array
int16_t truerms(int16_t array[])
{
	uint32_t arraySum = 0;
	uint16_t truerms = 0;
	for (uint16_t i = 0; i < (sizeof(array)/sizeof(array[0]) ); i++ )
	{
		arraySum += array[i];
	}
	truerms = (sqrt(arraySum) / (sizeof(array) / sizeof(array[0])) );
	return truerms;
}

//calculate avarage of array en return result
uint16_t arrayAvarage(uint16_t array[])
{
	uint16_t avarage = 0;
	for (uint16_t i = 0; i < (sizeof(array) / sizeof(array[0])); i++)
	{
		avarage += array[i];
	}
	avarage = avarage / (sizeof(array) / sizeof(array[0]));
	return avarage;
}

