*Please be aware that this documentation is a* ***work in progress!*** *I'll be updating it as time allows. Word :)*

--

#IoT Rain Detector

###Build your first cloud-enabled prototype in minutes!

In this workshop we will be working with a very simple prototype that allows you to measure the temperature and humidity in a remote location, and will let you know if it's raining or not.  

--

The **HARDWARE SETTING** is a piece of cake:

* 1x Photon (Arduino-compatible WiFi devkit)
* 1x Grove water sensor
* 1x Grove temperature and humidity sensor pro
* 1x Grove buzzer
* 3x Grove cables
* Assorted prototyping wires
* Mini-breadboard
* USB power supply (obviously!)

--

As for the **SOFTWARE**, also very simple. The code is divided in three parts. Each iteration adds new features as follows:

1. **Rain detector offline**  
Very easy prototype that it's not connected to the Internet. It only uses the water sensor and the buzzer, and it's based on the examples provided by Seeed Studio for these modules.
2. **Rain detector**  
Same hardware setting as in step #1, but this time it's connected to the relayr Cloud, and boolean value from the water sensor (TRUE or FALSE) can be retrieved from the [Developer Dashboard](https://developer.relayr.io) 
3. **Rain detector pro**  
This iteration implements the temperature and humidity sensor, and shows how to configure the JSON on the `publish()` function and send these values to the relayr Cloud.

--

Am I missing something? Don't panic!  
**Add'l documents and/or instructions will be provided during the workshop! ENJOY!**