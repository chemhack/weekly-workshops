// This #include statement was automatically added by the Particle IDE.
//Uses Jaime's version with increased packet size of 512mb
#include "MQTT.h"


// This #include statement was automatically added by the Particle IDE.
#include "SparkJson/SparkJson.h"


//Credentials from the developer dashboard
#define DEVICE_ID "f0992905-31dc-4410-a4d9-6311d6f31ea6"
#define MQTT_USER "f0992905-31dc-4410-a4d9-6311d6f31ea6"
#define MQTT_PASSWORD "WK89_VmFAqpG"
#define MQTT_CLIENTID "T8JkpBTHcRBCk2WMR1vMepg" //can be anything else
#define MQTT_TOPIC "/v1/f0992905-31dc-4410-a4d9-6311d6f31ea6/"
#define MQTT_SERVER "mqtt.relayr.io"

//Some definitions, including the publishing period
const int led = D7;
int ledState = LOW;
unsigned long lastPublishTime = 0;
unsigned long lastBlinkTime = 0;
int publishingPeriod = 500;


//defining pins and variables to hold info later
int moistureSensor = A0;
int waterSensor = D0;
bool needsWatered = false; //this will read TRUE after a certain level of the moisture sensor
bool isRaining = false; //this will read TRUE if the water sensor is shorted
int moistureLevel; //this holds the moisture level from the sensor

//------------------------------------------------------------------------------------//
// Initializing some stuff...                                                         //
//------------------------------------------------------------------------------------//

void setup()
{
    //Initializing pins
    pinMode(waterSensor, INPUT);
    pinMode(moistureSensor, INPUT);
  
    RGB.control(true);
    Serial.begin(9600);
    Serial.println("Hello there, I'm your Photon... and I'm gonna talk to the relayr Cloud!");
    //Setup our LED pin
    pinMode(led, OUTPUT);
    //200ms is the minimum publishing period
    publishingPeriod = publishingPeriod > 200 ? publishingPeriod : 200;
    mqtt_connect();
}



//------------------------------------------------------------------------------------//
// Callback function, necessary for the MQTT communication                            //
//------------------------------------------------------------------------------------//

void callback(char* topic, byte* payload, unsigned int length);
//Create our instance of MQTT object
MQTT client(MQTT_SERVER, 1883, callback);
//Implement our callback method that's called on receiving data from a subscribed topic
void callback(char* topic, byte* payload, unsigned int length)
{
  //Store the received payload and convert it to string
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  //Print the topic and the received payload
  Serial.println("topic: " + String(topic));
  Serial.println("payload: " + String(p));
  //Call our method to parse and use the received payload
  handlePayload(p);
}

//------------------------------------------------------------------------------------//
// This processes the payload; commands and configurations should be implemented here //  
//------------------------------------------------------------------------------------//

void handlePayload(char* payload)
{
  StaticJsonBuffer<200> jsonBuffer;
  //Convert payload to json
  JsonObject& json = jsonBuffer.parseObject(payload);
  
  if (!json.success())
  {
    Serial.println("json parsing failed");
    return;
  }

  //Get the value of the key "name", aka. listen to incoming commands and configurations
  const char* command = json["name"];
  Serial.println("parsed command: " + String(command));
  
  //COMMAND "color": we can change the color of the RGB to red, green or blue
  if (String(command).equals("color"))
  {
    const char* color = json["value"];
    Serial.println("parsed color: " + String(color));
    String s(color);
    
    if (s.equals("red"))
      RGB.color(255, 0, 0);

    else if (s.equals("green"))
      RGB.color(0, 255, 0);
      
    else if (s.equals("blue"))
      RGB.color(0, 0, 255);
  }
  
    //CONFIGURATION "frequency": We can change the publishing period
  if (String(command).equals("frequency"))
  {
    int frequency = json["value"];
    Serial.println("publishing period (ms): " + String(frequency));
    publishingPeriod = frequency;
  }
}

//------------------------------------------------------------------------------------//
// This function establishes the connection with the MQTT server                      //
//------------------------------------------------------------------------------------//

void mqtt_connect()
{
  Serial.println("Connecting to MQTT server...");
  
  if (client.connect(MQTT_CLIENTID, MQTT_USER, MQTT_PASSWORD))
  {
    Serial.println("Connection successful! Subscribing to topic...");
    //Subscribing to the topic "cmd", so we can listen to commands
    client.subscribe("/v1/"DEVICE_ID"/cmd");
    //And to "config" as well, for the configurations
    client.subscribe("/v1/"DEVICE_ID"/config");
  }
  
  else
    Serial.println("Connection failed! Check your credentials or the WiFi network");
}




//------------------------------------------------------------------------------------//
// This is for the LED to blink                                                       //
//------------------------------------------------------------------------------------//

void blink(int interval)
{
  if (millis() - lastBlinkTime > interval)
  {
    //Save the last time you blinked the LED
    lastBlinkTime = millis();
    
    if (ledState == LOW)
      ledState = HIGH;
      
    else
      ledState = LOW;
      
    //Set the LED with the ledState of the variable:
    digitalWrite(led, ledState);
  }
}

//------------------------------------------------------------------------------------//
// Publish function: What we want to send to the relayr Cloud                         //
//------------------------------------------------------------------------------------//

void publish()
{
  //MQTT_MAX_PACKET_SIZE is defined in "MQTT.h"
  //If the default size doesn't suffice, a modified version (512 bytes) can be found at:
  //   https://github.com/relayr/weekly-workshops/tree/master/MQTT-library-mod-buffer
  //If used, the header file and the library should be added manually in the Particle IDE
  StaticJsonBuffer<MQTT_MAX_PACKET_SIZE> pubJsonBuffer;
  //Create our JsonArray
  JsonArray& root = pubJsonBuffer.createArray();

//-------------------------------------------------
  //First object: analog input 0
  JsonObject& leaf1 = root.createNestedObject();
  //This is how we name what we are sending
  leaf1["meaning"] = "humidity";
  //This contains the readings of the port
  leaf1["value"] = moistureLevel;
//-------------------------------------------------
  
//-------------------------------------------------  
  //Second object: analog input 1
  JsonObject& leaf2 = root.createNestedObject();
  //This is how we name what we are sending
  leaf2["meaning"] = "isRaining";
  //This contains the readings of the port
  leaf2["value"] = isRaining;
//-------------------------------------------------

//-------------------------------------------------  
  //third object: analysis of analog1
  JsonObject& leaf3 = root.createNestedObject();
  //This is how we name what we are sending
  leaf3["meaning"] = "needsWatered";
  //This contains the readings of the port
  leaf3["value"] = needsWatered;
//-------------------------------------------------
  
  char message_buff[MQTT_MAX_PACKET_SIZE];
  root.printTo(message_buff, sizeof(message_buff));
  client.publish("/v1/"DEVICE_ID"/data", message_buff);
  Serial.println("Publishing " + String(message_buff));
}

//------------------------------------------------------------------------------------//
// measurement loop                                                                   //
//------------------------------------------------------------------------------------//
void loop()
{
  //If we're connected, we can send data...
  if (client.isConnected())
  {
    client.loop();
    //Publish within the defined publishing period
        if (millis() - lastPublishTime > publishingPeriod)
        {
            lastPublishTime = millis();
            //Publishing...
            publish();
        }
        //Blink LED  
        blink(publishingPeriod / 2);
  }
  
  else
  {
    //If the connection is lost, then reconnect...
    Serial.println("Retrying...");
    mqtt_connect();
  }      
        
  // Read the values of analog inputs 0 and 1, and store them in variables
    if(digitalRead(waterSensor) == LOW)
		{isRaining = true;}
	else {isRaining = false;}
	
	moistureLevel = analogRead(moistureSensor)/25;  
	
	 if(moistureLevel > 20)
		{needsWatered = false;}
	else {needsWatered = true;}
	
  
}
