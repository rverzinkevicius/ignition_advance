# ignition_advance
display ignition advance for scooter

Uses D1 Mini ESP8266 board and SH1106 display (D2=SDA, D1=SCL, 3V3=VCC, GND=GND). Can be used with SSD1306 display, just edit code accordingly. 

Hook up D6 to pickup coil signal, GND to ground and D5 just simple wire going near (or few turns around) spark plug wire. Button between D7 and GND.

Spark pickup:
![img](spark_pickup.jpg)

Short press button increase pickup coil advance by 1 degree, long press decreeses by 1 degree. After one minute of changing this value, it is saved to memory and will be remembered during reboots.

You need to know how many degrees BTDC pickup coil generates pulse. On my Piaggio LEADER 125 cc 2V engine it was about 60 degrees BTDC.

Code will calculate time between pickup coil signal and actual spark time and will calculate ignition advance.

RPM is also displayed.

![img](at_idle.jpg)

As esp8266 gpio sensitivity is different compared to ECU, accuracy will suffer a bit. Pickup coil signal at idle is raising for about 1ms till it reaches bit above 2V. Esp8266 reads high at lower voltage than 2V, so for error i assume that esp8266 reads high about 1/4-1/3ms before ECU does. This translates to 2-3 degrees of timing advance error, which i observed. So instead of actual 60 degress pickup coil signal advance, i am setting it to 57-58.

![img](IMG_20191208_120550.jpg)



