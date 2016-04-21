##How to change the network configuration (for instance to connect to a different WiFi hotspot):

Connect the board with an Ethernet cable to a laptop, and make sure that both are in the same network. Then navigate 
	
Open a browser an navigate to the following URL: `relayryun.local`

Log-in with the password: `arduino`

How to SSH the board: In the terminal, type `ssh root@relayryun.local`

##Device info

Account: `team@relayr.io`

Device name: `Hannover Messe (Arduino Yun)`

Credentials:

```
#define DEVICE_ID "d74d0079-897f-43f0-86ee-09a6ccacbd62"
#define MQTT_USER "d74d0079-897f-43f0-86ee-09a6ccacbd62"
#define MQTT_PASSWORD "cT1LSJBaduhF"
#define MQTT_CLIENTID "T100AeYl/Q/CG7gmmzKy9Yg"
#define MQTT_TOPIC "/v1/d74d0079-897f-43f0-86ee-09a6ccacbd62/"
#define MQTT_SERVER "mqtt.relayr.io"
```

For convenience, this device was included in the group: `Hannover Messe`

An own device model was also added. Simply navigate to "Models" and then under "Created by me" search for `Hannover Messe (Arduino Yun)`.

