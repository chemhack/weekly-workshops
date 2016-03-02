*Please be aware that this documentation is a* ***work in progress!*** *I'll be updating it as time allows. Word! :)*

--

#IoT Punching Bag

The program waits until the value on the Z axis of the accelerometer exceeds the one defined in THRESHOLD. If this happens, it will count as "punch detected". Then, it will take several readings on the same axis, and will calculate the average value during the punch (this average always differs from the peak, and it's closer to the real value). Later, the average is published in the relayr Cloud, and displayed in a dashboard.

**IMPORTANT GUIDELINES FOR THE CODE:**

1. Libraries have to be added manually in the Particle IDE! See the first lines on the source code.
2. To display the values on your profile, in the Developer Dashboard, it's necessary to generate new credentials, and replace the existing ones in the code. This "block" is easy to find, at the begining of it.
3. Some values can be adjusted manually: THRESHOLD (minimum acceleration considered as "punch"), SIZE (the size of the array of values used for the calculation of the average, i.e. the number of values registered during each punch).

--

**HARDWARE SETTING:**

* 1x [Particle Photon](https://www.coolcomponents.co.uk/photon-wi-fi-development-board.html)
* 1x [Accelerometer/Gyroscope grove IMU 10DOF](http://www.amazon.co.uk/Seeedstudio-Grove-IMU-10DOF/dp/B016CX3C4Y)
* 1x Grove cable (usually provided with the sensor)
* Assorted prototyping wires
* Mini-breadboard
* USB power supply

--

**QUICK ASSEMBLY:**  

`ACCEL----PHOTON`  
`red---------VIN`  
`black-------GND`  
`white--------D0`  
`yellow-------D1`
