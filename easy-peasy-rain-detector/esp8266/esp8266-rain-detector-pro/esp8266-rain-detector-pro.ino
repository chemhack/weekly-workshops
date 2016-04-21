#include <DHT.h>

//Libraries used: To be added manually on the Arduino IDE!
#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>


//WiFi credentials
#define SSID "Freeletics"
#define PASSWORD "unleashyourpotential"


//Credentials from the developer dashboard
#define DEVICE_ID "5e05ff87-1054-41f0-9f3d-b1e4ef9d6703"
#define MQTT_USER "5e05ff87-1054-41f0-9f3d-b1e4ef9d6703"
#define MQTT_PASSWORD "g3jULKwcd-Be"
#define MQTT_CLIENTID "TXgX/hxBUQfCfPbHk751nAw" //It can be anything else
#define MQTT_TOPIC "/v1/5e05ff87-1054-41f0-9f3d-b1e4ef9d6703/"
#define MQTT_SERVER "mqtt.relayr.io"


//This creates the WiFi client and the pub-sub client instance
WiFiClient espClient;
PubSubClient client(espClient);


//Some definitions, including the publishing period
const int led = BUILTIN_LED;
int ledState = LOW;
int analog0;
int test_counter=0;
unsigned long lastPublishTime = 0;
unsigned long lastBlinkTime = 0;
int publishingPeriod = 1000;


//Where are the modules connected?
#define WATER_SENSOR D0
#define BUZZER D3
#define DHTPIN D2


//What type of temp/hum sensor are we using?
#define DHTTYPE DHT22    // DHT 22 (AM2302)


//Function prototypes
void setup_wifi();
void mqtt_connect();
void callback(char* topic, byte* payload, unsigned int length);
void handlePayload(char* payload);
void publish();

//The values of humidity and temperature will be stored here
float h;
float t;

//And this is for the temperature and humidity sensor
DHT dht(DHTPIN, DHTTYPE);

//Setup function
void setup()
{
  //Initializing pins
  pinMode(WATER_SENSOR, INPUT);
  pinMode(BUZZER, OUTPUT);
  
  //Initialize the BUILTIN_LED pin as an output
  pinMode(led, OUTPUT);

  Serial.begin(9600);
  Serial.println("Hello there, I'm your ESP8266...");
  Serial.println("Let's talk to the relayr Cloud!");
  
  setup_wifi();
  client.setServer(MQTT_SERVER, 1883);
  client.setCallback(callback);

  //This initializes the sensor
  dht.begin();

  //200ms is the minimum publishing period
  publishingPeriod = publishingPeriod > 200 ? publishingPeriod : 200;
  mqtt_connect();
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


void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(SSID);

  WiFi.begin(SSID, PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}




//------------------------------------------------------------------------------------//
// Callback function, necessary for the MQTT communication                            //
//------------------------------------------------------------------------------------//

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
  
  
  //COMMAND "d0": We can toggle the digital output 0
  if (String(command).equals("d0"))
  {
    const char* d0 = json["value"];
    Serial.println("digital output 0: " + String(d0));
    String s(d0);
    
    if (s.equals("high"))
      digitalWrite(D0, HIGH);
    
    else if (s.equals("low"))
      digitalWrite(D0, LOW);
  }
  
  //COMMAND "d1": We can toggle the digital output 1
  if (String(command).equals("d1"))
  {
    const char* d1 = json["value"];
    Serial.println("digital output 1: " + String(d1));
    String s(d1);
    
    if (s.equals("high"))
      digitalWrite(D1, HIGH);
    
    else if (s.equals("low"))
      digitalWrite(D1, LOW);
  }
  
//  //CONFIGURATION "frequency": We can change the publishing period
//  if (String(command).equals("frequency"))
//  {
//    int frequency = json["value"];
//    Serial.println("publishing period (ms): " + String(frequency));
//    publishingPeriod = frequency;
//  }
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
  {
      Serial.println("Connection failed! Check your credentials or the WiFi network");
      //This reports the error code
      Serial.println("rc = ");
      Serial.print(client.state());
      //And it tries again in 5 seconds
      Serial.println("Retrying in 5 seconds...");
      delay(5000);
  }
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
// This is the MAIN LOOP, it's repeated until the end of time! :)                     //
//------------------------------------------------------------------------------------//

void loop()
{
  //If we're connected, we can send data...
  if (client.connected())
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

        //Is there water in the sensor?
        if(isExposedToWater()){   
              soundAlarm();   
         }
  }

  //If the connection is lost, then reconnect...
  else
  {
    Serial.println("Retrying...");
    mqtt_connect();
  }      

    // Reading temperature or humidity takes about 250 ms!
    // Sensor readings may also be up to 2s 'old' (it's a very slow sensor!)
  h = dht.readHumidity();
    // Read temperature as Celsius
  t = dht.readTemperature();
    // Read temperature as Fahrenheit
  float f = dht.readTemperature(true);
        
    // Check if any reads failed and exit early (to try again)
  if (isnan(h) || isnan(t) || isnan(f)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
        

  //This function prevents the device from crashing
  //since it allows the ESP8266 background functions to be executed
  //(WiFi, TCP/IP stack, etc.)
  yield();
  
}

//Publish function; here we add what we want to send to the cloud
void publish()
{    
  //MQTT_MAX_PACKET_SIZE is defined in "PubSubClient.h", it's 128 bytes by default
  //A modified version with 512 bytes it's available here:
  //   https://github.com/uberdriven/pubsubclient
  
  StaticJsonBuffer<MQTT_MAX_PACKET_SIZE> pubJsonBuffer;
  //Create our JsonArray
  JsonArray& root = pubJsonBuffer.createArray();

//-------------------------------------------------
//   //First object: Water sensor
//   JsonObject& leaf1 = root.createNestedObject();
//   //This is how we name what we are sending
//   leaf1["meaning"] = "w";
//   //This contains the readings of the sensor
//   leaf1["value"] = isExposedToWater();
//-------------------------------------------------
  
//-------------------------------------------------  
  //Second object: Humidity sensor
  JsonObject& leaf2 = root.createNestedObject();
  //This is how we name what we are sending
  leaf2["meaning"] = "humidity";
  //This contains the readings of the sensor
  leaf2["value"] = abs(h);
//-------------------------------------------------
  
//-------------------------------------------------  
  //Third object: Temperature sensor
  JsonObject& leaf3 = root.createNestedObject();
  //This is how we name what we are sending
  leaf3["meaning"] = "temperature";
  //This contains the readings of the sensor
  leaf3["value"] = abs(t);
//-------------------------------------------------
  
  char message_buff[MQTT_MAX_PACKET_SIZE];
  root.printTo(message_buff, sizeof(message_buff));
  client.publish("/v1/"DEVICE_ID"/data", message_buff);
  Serial.println("Publishing " + String(message_buff));
}

