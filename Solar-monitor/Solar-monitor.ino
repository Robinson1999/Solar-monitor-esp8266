/*
 Name:		Solar-monitor.ino
 Created:	10/28/2021 1:22:26 PM
 Author:	robin
*/

/*TODO
QUITE A LOT...
*/



//all real data
float solar_current = 0;
float battery_output_current = 0;
float battery_input_current = 0;
float V24 = 0;
float solarV = 0;
float V12 = 0;
float V5 = 0;
float temp1 = 0;
//high voltage board measurments
float V5H = 0;
float temp2 = 0;
//mains side 1
float VAC1trms = 0;
float IAC1trms = 0;
//main side 2
float VAC2trms = 0;
float IAC2trms = 0;
//calculated values
float AP1 = 0;//aparant power
float AP2 = 0;
float P1 = 0;//real power
float P2 = 0;
float PF1 = 0;//power factor
float PF2 = 0;


//variabls for blinking an LED with Millis
const byte led = 2; // ESP8266 Pin to which onboard LED is connected
unsigned long previousMillis = 0;  // will store last time LED was updated
const long interval = 1000;  // interval at which to blink (milliseconds)
int ledState = LOW;  // ledState used to set the LED

const byte buzzer = 2;
const byte ATS = 4;

unsigned long previousMicrosslow = 0;
unsigned long previousMicrosmedium = 0;
unsigned long previousMicrosfast = 0;

const uint8_t intervalADCslow = 100;  // interval at which to read data (miliseconds)
const uint8_t intervalADCmedium = 10;  // interval at which to read data (miliseconds)
const uint16_t intervalADCfast = 500;  // interval at which to read data (microseconds)


bool alarm = false;

void setup() {
	Serial.begin(115200);
	Serial.println("Booting");
	pinMode(led, OUTPUT);
	digitalWrite(led, HIGH);

	wifiConnect(); //connect to wifi
	OTAsetup();//set up over air programming
	tbConnect();//connect to thingsboard
	ADCsetup();//self explanatory

	digitalWrite(led, LOW);
}

void loop() {
	//read adc every interval
	unsigned long currentMicrosslow = millis();
	unsigned long currentMicrosmedium = millis();
	unsigned long currentMicrosfast = micros();
	if (currentMicrosfast - previousMicrosfast >= intervalADCfast)
	{
		ADCfast();
	}
	else if (currentMicrosmedium - previousMicrosmedium >= intervalADCmedium)
	{
		ADCmedium();
	}
	else if (currentMicrosslow - previousMicrosslow >= intervalADCslow)
	{
		ADCslow();
	}
	tbCheckConnection();//check connection for thingsboard and wifi, reconnect if needed
	OTAloop();//check for over the air programming
	
	
	

	//loop to blink without delay
	unsigned long currentMillis = millis();
	if (alarm) 
	{
		if (currentMillis - previousMillis >= interval) 
		{
			// save the last time you blinked the LED
			previousMillis = currentMillis;
			// if the LED is off turn it on and vi ce-versa:
			ledState = !(ledState);
			// set the LED with the ledState of the variable:
			digitalWrite(led, ledState);
		}
	}
	else
	{
		digitalWrite(led, LOW);
	}

	

}


/*
 ___       ________  ________  ________  ________  ________  ___      ___ ___  ___  _____ ______
|\  \     |\   __  \|\   __  \|\   __  \|\   __  \|\   __  \|\  \    /  /|\  \|\  \|\   _ \  _   \
\ \  \    \ \  \|\  \ \  \|\ /\ \  \|\  \ \  \|\  \ \  \|\  \ \  \  /  / | \  \\\  \ \  \\\__\ \  \
 \ \  \    \ \   __  \ \   __  \ \   ____\ \   __  \ \   _  _\ \  \/  / / \ \  \\\  \ \  \\|__| \  \
  \ \  \____\ \  \ \  \ \  \|\  \ \  \___|\ \  \ \  \ \  \\  \\ \    / /   \ \  \\\  \ \  \    \ \  \
   \ \_______\ \__\ \__\ \_______\ \__\    \ \__\ \__\ \__\\ _\\ \__/ /     \ \_______\ \__\    \ \__\
	\|_______|\|__|\|__|\|_______|\|__|     \|__|\|__|\|__|\|__|\|__|/       \|_______|\|__|     \|__|

*/