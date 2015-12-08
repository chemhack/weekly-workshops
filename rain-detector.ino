// This #include statement was automatically added by the Particle IDE.
#include "SparkJson/SparkJson.h"

// This #include statement was automatically added by the Particle IDE.
#include "MQTT/MQTT.h"

//Where are the modules connected?
#define WATER_SENSOR 0
#define BUZZER 3

//Credentials from the developer dashboard
#define DEVICE_ID "e91ac158-9881-4d07-bad1-65d29205734e" 
#define MQTT_USER "e91ac158-9881-4d07-bad1-65d29205734e" 
#define MQTT_PASSWORD "mS.rEmz.h2R0"

#define MQTT_CLIENTID "rain_detector" //This can be anything else
#define MQTT_SERVER "mqtt.relayr.io"

//Some definitions, including the publishing period
const int led = D7;
int ledState = LOW;
unsigned long lastPublishTime = 0;
unsigned long lastBlinkTime = 0;
int publishingPeriod = 400;

//Initializing some stuff...
void setup()
{
	pins_init();
	
    RGB.control(true);
    Serial.begin(9600);
    Serial.println("Hello there, I'm your Photon!");
    //setup our LED pin
    pinMode(led, OUTPUT);
    //set 200ms as minimum publishing period
    publishingPeriod = publishingPeriod > 200 ? publishingPeriod : 200;
    mqtt_connect();
	
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
	if(digitalRead(WATER_SENSOR) == LOW)
		return true;
            		    
	else return false;
}

/////////////////////////////////////
void callback(char* topic, byte* payload, unsigned int length);
//create our instance of MQTT object
MQTT client(MQTT_SERVER, 1883, callback);
//implement our callback method thats called on receiving data from a subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
  //store the received payload and convert it to string
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  //print the topic and the payload received
  Serial.println("topic: " + String(topic));
  Serial.println("payload: " + String(p));
  //call our method to parse and use the payload received
  handlePayload(p);
}

void handlePayload(char* payload) {
  StaticJsonBuffer<200> jsonBuffer;
  //convert payload to json
  JsonObject& json = jsonBuffer.parseObject(payload);
  if (!json.success()) {
    Serial.println("json parsing failed");
    return;
  }
  //get value of the key "command"
  const char* command = json["command"];
  Serial.println("parsed command: " + String(command));
  if (String(command).equals("color"))
  {
    const char* color = json["value"];
    Serial.println("parsed color: " + String(color));
    String s(color);
    if (s.equals("red")){
      RGB.color(255, 0, 0);
    }
    else if (s.equals("blue"))
      RGB.color(0, 0, 255);
    else if (s.equals("green"))
      RGB.color(0, 255, 0);
  }
}
//////////////////////////////////

//////////////////////////////////
void mqtt_connect() {
  Serial.println("Connecting to mqtt server...");
  if (client.connect(MQTT_CLIENTID, MQTT_USER, MQTT_PASSWORD)) {
    Serial.println("Connection successful! Subscribing to topic...");
    //subscribe to a topic
    client.subscribe("/v1/"DEVICE_ID"/cmd");
  }
  else {
    Serial.println("Connection failed! Check your credentials or WiFi network");
  }
}
//////////////////////////////////

void blink(int interval) {
  if (millis() - lastBlinkTime > interval) {
    // save the last time you blinked the LED
    lastBlinkTime = millis();
    if (ledState == LOW)
      ledState = HIGH;
    else
      ledState = LOW;
    // set the LED with the ledState of the variable:
    digitalWrite(led, ledState);
  }
}

//This is the main loop, it's repeated until the end of time :)
void loop()
{
	
	//If we're connected, we can send data...
	if (client.isConnected()) {

    client.loop();
    //Publish within the defined publishing period
        if (millis() - lastPublishTime > publishingPeriod) {
            
            lastPublishTime = millis();
            
            //Finally publishing...
            publish();
            
            //Is there water in the sensor?
            if(isExposedToWater()){
             	
             	soundAlarm();   
            }
            
        }
            
        //Blink LED  
        blink(publishingPeriod / 2);
    }
    else {
    //If connection is lost, then reconnect
        Serial.println("Retrying...");
        mqtt_connect();
    }
    //Serial.println(z);
    //Serial.println(Kalman);
    //Serial.println(digitalRead(M1));
    
}



void publish() {
  //Create our JsonArray
  StaticJsonBuffer<300> pubJsonBuffer;
  JsonArray& root = pubJsonBuffer.createArray();

  //First object: Water sensor
  JsonObject& leaf1 = root.createNestedObject();
  //This is how we name what we are sending
  leaf1["meaning"] = "Water";
  //This contains the readings of the sensor
  leaf1["value"] = isExposedToWater();

  char message_buff[128];
  root.printTo(message_buff, sizeof(message_buff));
  client.publish("/v1/"DEVICE_ID"/data", message_buff);
  Serial.println("Publishing " + String(message_buff));
}


