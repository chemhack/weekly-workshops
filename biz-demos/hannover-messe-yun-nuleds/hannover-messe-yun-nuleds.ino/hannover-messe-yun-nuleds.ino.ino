#include <Ultrasonic.h>
#include <YunClient.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <SoftwareSerial.h>

//========= define our variables ======================/

//DEVICE: relayryun
//ACCOUNT: jaime@relayr.io
//#define DEVICE_ID "a5de1928-757e-4605-b421-60cea868f401"
//#define MQTT_USER "a5de1928-757e-4605-b421-60cea868f401"
//#define MQTT_PASSWORD "CwEzjf4SvaJc"
//#define MQTT_CLIENTID "Tpd4ZKHV+RgW0IWDOqGj0AQ"
//#define MQTT_TOPIC "/v1/a5de1928-757e-4605-b421-60cea868f401/"
//#define MQTT_SERVER "mqtt.relayr.io"

//DEVICE: Hannover Messe (Arduino Yun)
//ACCOUNT: team@relayr.io
#define DEVICE_ID "d74d0079-897f-43f0-86ee-09a6ccacbd62"
#define MQTT_USER "d74d0079-897f-43f0-86ee-09a6ccacbd62"
#define MQTT_PASSWORD "cT1LSJBaduhF"
#define MQTT_CLIENTID "T100AeYl/Q/CG7gmmzKy9Yg"
#define MQTT_TOPIC "/v1/d74d0079-897f-43f0-86ee-09a6ccacbd62/"
#define MQTT_SERVER "mqtt.relayr.io"

YunClient yun;
const int led = 13;
int ledState = LOW;

//char message_buff[100];
int publishingPeriod = 500;
unsigned long lastPublishTime = 0;
unsigned long lastBlinkTime = 0;

//=================== declare all methods ======================//

void callback(char* topic, byte* payload, unsigned int length);
void mqtt_connect();
char* dataToJson(String meaning, int value);
void publish();

//create our mqtt client object, params are server, port, callback and wifi client
PubSubClient client(MQTT_SERVER, 1883, callback, yun);

//The ranger is connected on pin D7 (connector "D7" using the baseshield)
Ultrasonic ultrasonic(7);
int distance;
#define MAXDISTANCE 30

//CO level
float COvol;

//CO2 sensor stuff
const int pinRx = 9;
const int pinTx = 8;
 
SoftwareSerial sensor(pinTx,pinRx);
 
const unsigned char cmd_get_sensor[] =
{
    0xff, 0x01, 0x86, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x79
};
unsigned char dataRevice[9];
int temperature;
int CO2PPM;




//======================= our application logic ===========================//

void setup() {

  Bridge.begin();
  sensor.begin(9600);
  Serial.begin(9600);
  Serial.println("Hello There, I'm your relayr-enabled Yun!");
  pinMode(led, OUTPUT);
  //set 200ms as minimum publishing period else server will kick us out
  publishingPeriod = publishingPeriod > 200 ? publishingPeriod : 200;
  mqtt_connect();
}

void loop() {
  if (client.connected()) {
    client.loop();
    //publish within publishing period
    if (millis() - lastPublishTime > publishingPeriod) {
      lastPublishTime = millis();
      
      distance = ultrasonic.MeasureInCentimeters();
      
      if (distance >= MAXDISTANCE)
        distance = MAXDISTANCE;

      //Retrieve value from the CO sensor (as a coefficient between 0 and 1)
      COvol=((float)analogRead(A0)/1024)*5.0;
//      Serial.print("A0 = ");
//      Serial.println(analogRead(A0));
//      Serial.print("COvol = ");
//      Serial.println(COvol);

      //Retrieve value from the CO sensor (as a coefficient between 0 and 1)
      if(dataReceive())
      {
        Serial.print("Temperature: ");
        Serial.print(temperature);
        Serial.print("  CO2: ");
        Serial.print(CO2PPM);
        Serial.println("");
       }
      
      publish();
      
    }
    blink(publishingPeriod / 2);
  } else {
    //if connection lost, reconnect
    Serial.println("retrying to connect..");
    mqtt_connect();
    //wait two seconds before trying to reconnect.
    delay(2000);
  }
}

void publish()
{    
  //MQTT_MAX_PACKET_SIZE is defined in "PubSubClient.h", it's 128 bytes by default
  //A modified version with 512 bytes it's available here:
  //   https://github.com/uberdriven/pubsubclient
  StaticJsonBuffer<256> pubJsonBuffer;
  //Create our JsonArray
  JsonArray& root = pubJsonBuffer.createArray();

//-------------------------------------------------
  //First object: proximity
  JsonObject& leaf1 = root.createNestedObject();
  leaf1["meaning"] = "proximity";
  leaf1["value"] = distance;
//-------------------------------------------------

//-------------------------------------------------  
  //Second object: temperature
  JsonObject& leaf2 = root.createNestedObject();
  leaf2["meaning"] = "temperature";
  leaf2["value"] = temperature;
//-------------------------------------------------
  
//-------------------------------------------------  
  //Third object: CO2 volume
  JsonObject& leaf3 = root.createNestedObject();
  leaf3["meaning"] = "CO2";
  leaf3["value"] = CO2PPM;
//-------------------------------------------------

////-------------------------------------------------  
//  //Fourth object: CO volume
//  JsonObject& leaf4 = root.createNestedObject();
//  leaf4["meaning"] = "CO";
//  leaf4["value"] = COvol;
////-------------------------------------------------
  
  char message_buff[256];
  root.printTo(message_buff, sizeof(message_buff));
  client.publish("/v1/"DEVICE_ID"/data", message_buff);
  Serial.println("Publishing " + String(message_buff));
}

//void publish()
//{  
//  //create our json payload as {"meaning":"moisture","value":sensorvalue}
//  //dataToJson("distance", distance);
//  dataToJson("CO", COvol);
//  client.publish("/v1/"DEVICE_ID"/data", message_buff);
//  Serial.println("Publishing " + String(message_buff));
//}



void mqtt_connect()
{
  Serial.println("Connecting to mqtt server");
  if (!client.connect(MQTT_CLIENTID, MQTT_USER, MQTT_PASSWORD))
    Serial.println("Connection failed, check your credentials or network status");
}

// implement our callback method thats called on receiving data
void callback(char* topic, byte* payload, unsigned int length) {
  //store the received payload and convert it to string
  char p[length + 1];
  memcpy(p, payload, length);
  p[length] = NULL;
  String message(p);
}

////format our data as a json object, and convert to a string
//char* dataToJson(String meaning, float value) {
//  StaticJsonBuffer<100> pubJsonBuffer;
//  JsonObject& pubJson = pubJsonBuffer.createObject();
//  //define a meaning for what we're sending
//  pubJson["meaning"] = meaning;
//  //set our value key to the sensor's reading
//  pubJson["value"] = value;
//  //copy our json object as a char array and publish it via mqtt
//  pubJson.printTo(message_buff, sizeof(message_buff));
//  return message_buff;
//}




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



bool dataReceive(void)
{
    byte data[9];
    int i = 0;
 
    //transmit command data
    for(i=0; i<sizeof(cmd_get_sensor); i++)
    {
        sensor.write(cmd_get_sensor[i]);
    }
    delay(10);
    //begin reveiceing data
    if(sensor.available())
    {
        while(sensor.available())
        {
            for(int i=0;i<9; i++)
            {
                data[i] = sensor.read();
            }
        }
    }
    else
      Serial.println("CO2 sensor not available!");
 
    if((i != 9) || (1 + (0xFF ^ (byte)(data[1] + data[2] + data[3]
    + data[4] + data[5] + data[6] + data[7]))) != data[8])
    {
        return false;
    }
    CO2PPM = (int)data[2] * 256 + (int)data[3];
    temperature = (int)data[4] - 40;
 
    return true;
}


