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

# Notes
Channel select options for NOT FITTED hardware, such as the HX711 for scales, or the I2C ADC on 50 and 60 are now Greyed out.
This test is conducted only at power up, so reset the esp if you add / disconnect these. 
Digital inputs (D7/D6) are presented as simple binary scale: D7 is 2Vor 0V, D6 is 1V or 0V, allowing the "digital channel" to monitor two inputs simulaneously.
Connecting D7 to ground during power up will force the unit to AP mode. Connect to "Oscilloscope" and 192.168.4.1 
Otherwise Wifi manager will automatically attempt to connect to known network, or present an options list on 192.168.4.1 



