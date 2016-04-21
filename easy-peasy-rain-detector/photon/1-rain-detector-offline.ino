//Where are the modules connected?
#define WATER_SENSOR 0
#define BUZZER 3

//Some definitions, including the publishing period
const int led = D7;
int ledState = LOW;

///Initializing some stuff...
void setup()
{
	pins_init();
	
    Serial.begin(9600);
    Serial.println("Hello there, I'm your Photon!");
    //Setup our LED pin
    pinMode(led, OUTPUT);
	
}

//This one is called up there; it initializes some pins
void pins_init()
{
	pinMode(WATER_SENSOR, INPUT);
	pinMode(BUZZER, OUTPUT);
}

//This function makes the buzzer beep shortly
void soundAlarm()
{
	for(uint8_t i = 0;i < 4;i ++)
	{
		digitalWrite(BUZZER, HIGH);
		delay(50);
		digitalWrite(BUZZER, LOW);
		delay(50);
	}
}

//This function checks if there's water in the sensor
boolean isExposedToWater()
{
	if(digitalRead(WATER_SENSOR) == LOW){
        Serial.println("Raindrops are falling on my... sensor...");
        return true;
	}
            		    
	else return false;
}


//This is the main loop, it's repeated until the end of time :)
void loop()
{
            //Is there water in the sensor?
            if(isExposedToWater()){
             	
             	soundAlarm();   
            }
            
}



