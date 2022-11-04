# Oscilloscope-Test

Might have been better to call it a "Strip recorder".. 
Thi uses extensive to dEEbugger (https://github.com/S-March/dEEbugger_Public) To make a more universal device: 
Aiming for ESP32 and faster ADC. but for now: 
Uses integral ESP8266 A0 for analog to about 1ksps.
Sampling tries to be synchronous for two channels.
Has integral 2 input "digital" that displays as 0/1/2/3 'v' states on a single channel.
All data is read in the ESP8266 'c' code and converted to Float variables that are "truth". So 1.03 V adc reading will send as "1.03"
Defaults to a "triangle" signal on Ch2, and a reading of A0 in CH1.
If a HX711 is present,(and with a 10kg strain gauge), it can display Force measurements, useful for test instruments.
The I2C and Terminal functions have been retained. 
![image](https://user-images.githubusercontent.com/6950560/200069121-6060dc1a-fdd6-4684-88f0-ffd7dc79e09c.png)


