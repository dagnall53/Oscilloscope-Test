# Oscilloscope-Test

Might have been better to call it a "Strip recorder".. 
This uses extensive modifications to dEEbugger (https://github.com/S-March/dEEbugger_Public) 
and replaces the original graphics with PicoGraph https://github.com/RainingComputers/picograph.js 
The aim was to educate myself and also to create a more universal device for lab tests: 
I may eventualy migrate to ESP32 and faster ADC , and perhaps Interrupts for the faster sample rates.
# Current Capabilities
Uses integral ESP8266 A0 for analog to about 1ksps.
Sampling tries to be synchronous for two channels.
Has integral 2 input "digital" that displays as 0/1/2/3 'v' states on a single channel.
All data is read in the ESP8266 'c' code and converted to Float variables that are "truth". So 1.03 V adc reading will send as "1.03"
Defaults to a "triangle" signal on Ch2, and a reading of A0 in CH1.
If a HX711 is present,(and with a 10kg strain gauge), it can display Force measurements, useful for test instruments.
The I2C and Terminal functions have been retained. 
![image](https://user-images.githubusercontent.com/6950560/200069121-6060dc1a-fdd6-4684-88f0-ffd7dc79e09c.png)


