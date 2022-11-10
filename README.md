# Oscilloscope-Test

Might have been better to call it a "Strip recorder".. 
This uses extensive modifications to dEEbugger (https://github.com/S-March/dEEbugger_Public) 
and replaces the original graphics with PicoGraph https://github.com/RainingComputers/picograph.js 
The aim was to educate myself and also to create a more universal device for lab tests: 
# Capabilities
Uses integral ESP8266 A0 for analog to about 1ksps. (and TWo internal ADC in ESP32 version
Sampling tries to be synchronous for two channels.
Has 2 integral "digital" inuts that display at the bottom of the screen.
Has an integral on screen display to show "as-read" values.

If a HX711 is present,(and with a 10kg strain gauge), it can display Force measurements, useful for test instruments.
ESP32 version can display the internal Hall reading
The I2C and Terminal dEEbugger functions have been retained. 
![image](https://user-images.githubusercontent.com/6950560/201147059-9e8839f5-9a15-4c72-8099-c261171031b8.png)

# Notes
All data is read in the ESP8266 'c' code and converted to Float variables that are "truth". So 1.03 V adc reading will send a websock message as "1.03"
On start up it defaults to a "triangle" signal on Ch2, and a reading of A0 in CH1.
Channel select options for NOT FITTED hardware, such as the HX711 for scales, or the I2C ADC on 50 and 60 are now Greyed out.
This test is conducted only at power up, so reset the esp if you add / disconnect these. 
Digital inputs (D7/D6) are presented in the datalog output as a simple binary scale: D7 is 2Vor 0V, D6 is 1V or 0V, allowing the "digital channel" to monitor two inputs simulaneously.
Connecting D7 (Esp8266) OR pIN 13 (ESP32) to ground during power up will force the unit to AP mode. 
Then Connect to "Oscilloscope" and 192.168.4.1 
Otherwise Wifi manager will automatically attempt to connect to known network, or present an options list on 192.168.4.1 

#ESP32 Version
Note that the internal ADC is known to be poor! This is just for ROUGH measurements!. But there are two genuine input channels, although both are titled "A0" (a left over from the esp8266!) .
![image](https://user-images.githubusercontent.com/6950560/201144552-92e9394f-e7e9-4628-af5d-a764ca439872.png)

Internal Hall sensor. Scaling is roughly centiTesla (!).
I used an averaging to reduce the HALL noise, so the sample rate drops (automaticay) if the Hall is selected.
Also the hall autozeros on channel selection == Hall.
![image](https://user-images.githubusercontent.com/6950560/201144222-bf573bb7-021d-4888-9ea1-e41bb2aa8774.png)


