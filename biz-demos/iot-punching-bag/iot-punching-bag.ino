//REQUIRED LIBRARIES: very important!
//These have to be added manually on the Particle IDE! 
//#include "SparkJson/SparkJson.h"
//#include "MQTT/MQTT.h"
//#include "MPU6050/MPU6050.h"


//Credentials from the developer dashboard
#define DEVICE_ID "8fb4c931-f817-4399-a0f8-69530a24dffe" 
#define MQTT_USER "8fb4c931-f817-4399-a0f8-69530a24dffe" 
#define MQTT_PASSWORD "3FGOGiAPx.sQ"
#define MQTT_CLIENTID "punching-bag" //This can be anything else
#define MQTT_SERVER "mqtt.relayr.io"


//Threshold: minimum value of the acceleration to be considered as a punch (raw values)
//Size: number of values taken in order to make the average
#define THRESHOLD 20000
#define SIZE 10


//Some definitions, including the publishing period
const int led = D7;
int ledState = LOW;
unsigned long lastPublishTime = 0;
unsigned long lastBlinkTime = 0;
bool punch_detected = false;
int publishingPeriod = 1000;


// MPU variables:
MPU6050 accelgyro;
int16_t ax, ay, az;
int16_t gx, gy, gz;


//Variables for the calculation of the peak value
int readings[SIZE];
int peak;
int peak_index;
int average;
int sum;




//-----------------------------------------------------------------//
// Initializing some stuff...                                      //
//-----------------------------------------------------------------//

void setup()
{
	//Initializing serial port
	Serial.begin(9600);
	
	//Initializing I2C devices
	Wire.begin();
	Serial.println("Initializing I2C devices...");
    
    //Initializing accelerometer, and changing the range to +/-8g
    accelgyro.initialize();
    accelgyro.setFullScaleAccelRange(MPU6050_ACCEL_FS_8);
	
    RGB.control(true);

    Serial.println("Hello there, I'm your Photon... and I'm gonna talk to the relayr Cloud!");
    //Setup our LED pin
    pinMode(led, OUTPUT);
    //200ms is the minimum publishing period
    publishingPeriod = publishingPeriod > 200 ? publishingPeriod : 200;
    mqtt_connect();
}





//-----------------------------------------------------------------//
// Callback function, necessary for the MQTT communication         //
//-----------------------------------------------------------------//

void callback(char* topic, byte* payload, unsigned int length);
//Create our instance of MQTT object
MQTT client(MQTT_SERVER, 1883, callback);
//Implement our callback method that's called on receiving data from a subscribed topic
void callback(char* topic, byte* payload, unsigned int length) {
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





//-----------------------------------------------------------------//
// This processes the payload; commands should be implemented here //  
//-----------------------------------------------------------------//

void handlePayload(char* payload) {
  StaticJsonBuffer<200> jsonBuffer;
  //Convert payload to json
  JsonObject& json = jsonBuffer.parseObject(payload);
  if (!json.success()) {
    Serial.println("json parsing failed");
    return;
  }
  //Get the value of the key "command", aka. listen to incoming commands
  const char* command = json["command"];
  Serial.println("parsed command: " + String(command));
  
  //We can send commands to change the color of the RGB
  if (String(command).equals("color"))
  {
    const char* color = json["value"];
    Serial.println("parsed color: " + String(color));
    String s(color);
    if (s.equals("red")){
      RGB.color(255, 0, 0);
    }
    else if (s.equals("green"))
      RGB.color(0, 255, 0);
    else if (s.equals("blue"))
      RGB.color(0, 0, 255);

  }
}





//-----------------------------------------------------------------//
// This function establishes the connection with the MQTT server   //
//-----------------------------------------------------------------//

void mqtt_connect() {
  Serial.println("Connecting to MQTT server...");
  if (client.connect(MQTT_CLIENTID, MQTT_USER, MQTT_PASSWORD)) {
    Serial.println("Connection successful! Subscribing to topic...");
    //This one subscribes to the topic "cmd", so we can listen to commands
    client.subscribe("/v1/"DEVICE_ID"/cmd");
  }
  else {
    Serial.println("Connection failed! Check your credentials or WiFi network");
  }
}





//-----------------------------------------------------------------//
// This is for the LED to blink                                    //
//-----------------------------------------------------------------//

void blink(int interval) {
  if (millis() - lastBlinkTime > interval) {
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





//-----------------------------------------------------------------//
// This is the MAIN LOOP, it's repeated until the end of time! :)  //
//-----------------------------------------------------------------//

void loop()
{
        
    //Get the readings from the accel/gyro
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
        
    //If the acceleration on axis exceeds the threshold: punch detected!
    if(az>THRESHOLD){
        
        //Punch detected flag
        punch_detected=true;
        //RGB.color(255, 0, 0);
        
        average=0;
        sum=0;
            
        for (int i=0; i<SIZE; i++){
            
            accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
            
            //It only considers positive values (on the right direction)
            if(az>0){
                
                readings[i] = az;
                sum = sum + readings[i];
                average = sum/SIZE;
                delay(20); //It waits some time between readings, for stabilization
            }
            
            //Otherwise, it discards them and moves the index back and takes the reading again
            else
            i--;
        }
            
        peak_index = getIndexOfMaxValue(readings,SIZE);
        peak = readings[peak_index];
        RGB.color(255, 0, 0);
        delay(500);
        // UNCOMMENT THIS LINES FOR DEBUGGING!
        // Serial.print("Peak:\t"); Serial.println(peak);
         Serial.print("Raw average value:\t"); Serial.println(average);
    }
        
        //If we're connected, we can send data...
	    if (client.isConnected()) {

        client.loop();
        //Publish within the defined publishing period
            if (millis() - lastPublishTime > publishingPeriod) {
            
                lastPublishTime = millis();
                
                //Publishing...
                
                if (punch_detected == true){
                    
                    publish();  
                    
                    //Reset the flag
                    punch_detected=false;
                }
                
            }
            
            //Blink LED  
            blink(publishingPeriod / 2);
            RGB.color(0, 0, 0);
        }
            
        else {
        //If connection is lost, then reconnect
            Serial.println("Retrying...");
            mqtt_connect();
        }
        
    

    //UNCOMMENT THE SECTION BELOW TO PRINT REAL TIME VALUES!
    //BTW, this happens way faster than the publishing method
    //-------------------------------------------------------
    // accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);
    // Serial.print("a/g:\t");
    // Serial.print(ax); Serial.print("\t");
    // Serial.print(ay); Serial.print("\t");
    // Serial.print(az); Serial.print("\t");
    // Serial.print(gx); Serial.print("\t");
    // Serial.print(gy); Serial.print("\t");
    // Serial.println(gz);

}





//-----------------------------------------------------------------//
// PUBLISH FUNCTION // What we want to send to the cloud           //
//-----------------------------------------------------------------//

void publish() {
  //Create our JsonArray
  StaticJsonBuffer<300> pubJsonBuffer;
  JsonArray& root = pubJsonBuffer.createArray();

//UNCOMMENT THIS SECTION TO PUBLISH IN THE CLOUD ALSO THE PEAK VALUE
// //-------------------------------------------------
//   //First object: peak value
//   JsonObject& leaf1 = root.createNestedObject();
//   leaf1["meaning"] = "Peak (g)";
//   leaf1["value"] = peak/4096.00; //To convert to (g), we have to divide the raw value by the range of the accelerometer (+/-8g)
// //-------------------------------------------------
  
//-------------------------------------------------  
  //Second object: average value
  JsonObject& leaf2 = root.createNestedObject();
  leaf2["meaning"] = "Average (g)";
  leaf2["value"] = average/2048.00; //To convert to (g), we have to divide the raw value by the range of the accelerometer (+/-8g)
//-------------------------------------------------
  
  char message_buff[128];
  root.printTo(message_buff, sizeof(message_buff));
  client.publish("/v1/"DEVICE_ID"/data", message_buff);
  Serial.println("Publishing " + String(message_buff));

}





//-----------------------------------------------------------------//
// CALCULATE THE PEAK VALUE // Returns the index of the peak value //
//-----------------------------------------------------------------//

int getIndexOfMaxValue(int* array, int size){
 int maxIndex = 0;
 int max = array[maxIndex];
 for (int i=0; i<size; i++){
   if (max<array[i]){
     max = array[i];
     maxIndex = i;
   }
 }
 return maxIndex;
}




