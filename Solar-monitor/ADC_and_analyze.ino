/*
TODO
	MAKE ARRAYS OF FLOATING POINTS FOR VOLTAGE AND CURRENT TO PROPERLY CALC POWER
*/


#include <MCP_ADC.h>

//can be changed to mcp3008 without other code rework I think :) havent tested
MCP3208 mcp1(12, 13, 14); //(dataIn, dataOut, clock)
MCP3208 mcp2(12, 13, 14);

const byte CS1 = 15; //select adc 1c
const byte CS2 = 5;	//select adc 2

#define spiSpeed 2000000 //spi clock frequency in hertz

float Vref = 5.00;

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
//diferential arrays
int16_t VAC1_array[HSA];
int16_t VAC2_array[HSA];

//multipliers
byte solar_current_mult = 27;
byte battery_output_current_mult = 27;
byte battery_input_current_mult = 27;
byte V24_mult = 7;
byte solarV_mult = 31;
byte V12_mult = 3;
byte V5_mult = 2;
//byte temp1_mult = 0; formula needed V-509/2.64=T (in milivolts) for mcp3208
//high voltage board measurments
byte V5H_mult = 2;
//byte temp2_mult = 0; formula needed V-509/2.64=T (in milivolts) for mcp3208
//mains side 1
byte VAC1rms_mult = 160;
byte IAC1rms_mult = 160;
//main side 2
byte VAC2rms_mult = 160;
byte IAC2rms_mult = 160;

//true rms values
uint16_t IAC1trms_b;
uint16_t VAC1trms_b;

uint16_t IAC2trms_b;
uint16_t VAC2trms_b;

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

//convert and send all data
void convertToFloat()
{
	uint16_t bitsize1 = mcp1.maxValue();
	uint16_t bitsize2 = mcp2.maxValue();

	solar_current = (arrayAvarage(solar_input_current_array) * (Vref / bitsize1))*solar_current_mult;
	battery_output_current = (arrayAvarage(battery_output_current_array) * (Vref / bitsize1))*battery_output_current_mult;
	battery_input_current = (arrayAvarage(battery_input_current_array) * (Vref / bitsize1))*battery_input_current_mult;
	V24 = (arrayAvarage(V24_array) * (Vref / bitsize1))*V24_mult;
	solarV = (arrayAvarage(solarV_array) * (Vref / bitsize1))*solarV_mult;
	V12 = (arrayAvarage(V12_array) * (Vref / bitsize1))*V12_mult;
	V5 = (arrayAvarage(V5_array) * (Vref / bitsize1))*V5H_mult;

	temp1 = (arrayAvarage(temp1_array) * (Vref / bitsize1));

	//Mains board
	V5H = (arrayAvarage(V5H_array) * (Vref / bitsize2))*V5H_mult;

	temp1 = (arrayAvarage(temp1_array) * (Vref / bitsize2));

	VAC1trms = (VAC1trms_b * (Vref / bitsize2) * VAC1rms_mult);
	IAC1trms = (IAC1trms_b * (Vref / bitsize2) * IAC1rms_mult);

	VAC2trms = (VAC2trms_b * (Vref / bitsize2) * VAC2rms_mult);
	IAC2trms = (IAC2trms_b * (Vref / bitsize2) * IAC2rms_mult);

	//aparant power
	AP1 = VAC1trms * IAC1trms;
	AP2 = VAC2trms * IAC2trms;

	//true power
	int16_t addedArray[HSA];
	for (uint16_t i = 0; i < (sizeof(addedArray) / sizeof(addedArray[0])); i++)
	{
		addedArray[i] = VAC1_array[i] + IAC1_array[i];
	}
	P1 = truerms(addedArray)

}

//calc diff of 2 arrays from 1 side of mains and then calc true rms voltage and current
void calcTrueRMS()
{
	IAC1trms_b = truerms(IAC1_array);
	IAC2trms_b = truerms(IAC2_array);

	//diffrence between 2 whole arrays
	for (uint16_t i = 0; i < (sizeof(VAC12_array) / sizeof(VAC12_array[0])); i++)
	{
		VAC1_array[i] = VAC11_array[i] - VAC12_array[i];
	}
	VAC1trms_b = truerms(VAC1_array);

	for (uint16_t i = 0; i < (sizeof(VAC22_array) / sizeof(VAC22_array[0])); i++)
	{
		VAC2_array[i] = VAC21_array[i] - VAC22_array[i];
	}
	VAC2trms_b = truerms(VAC2_array);
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

