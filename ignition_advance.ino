#include <EEPROM.h>
#include <ESP8266WiFi.h>
//#include "SSD1306Wire.h"  //for 0.96" SSD1306 OLED display
#include <SH1106Wire.h>     //for 1.3" SH1106 OLED display
#include "GyverButton.h"
#include "GyverFilters.h"

GFilterRA testFilterRA;

GButton butt1(D7);

//SSD1306Wire  display(0x3c, D2, D1);  //D2=SDA  D1=SCL
SH1106Wire  display(0x3c, D2, D1);  //D2=SDA  D1=SCL

int pickup_advance = 60; //degrees BTDC when pickup generates pulse
unsigned long starttime= 0;
bool saved = false;

int sparkpin = D5;
int advance=0;
int advance_a=0;
int advance2=0;
int advance3=0;
int counter_advance=0;
volatile bool advance_updated = false;
volatile bool spark = false;
volatile unsigned long duration_advance=1;

int ledpin = D8;

int rpmpin = D6;
int rpm = 1;
int rpm1 = 1;
int rpm2 = 1;
float rpmai = 1;
unsigned long last_update_rpm=0;
unsigned long last_show_rpm=0;
unsigned long duration_rpmTmp=0;
volatile unsigned long duration_rpm=0;
volatile unsigned long last_rpm=0;
bool rpmflag=true;
bool rpmupdated=false;

const int numReadings = 4;
int readings[numReadings];
int readIndex = 0;
int total = 0;
int average = 0;

void setup()   {   

  WiFi.mode( WIFI_OFF );
  WiFi.forceSleepBegin();
  delay(300);
  
  Serial.begin(15200);

  testFilterRA.setCoef(0.5);

  display.init();
  display.flipScreenVertically();  
  display.setFont(Dialog_bold_42); 
  display.drawString(0, 0, "Hello");
  display.display(); 
  delay(2000);  

  pinMode(rpmpin,INPUT);
  pinMode(sparkpin,INPUT);
  pinMode(ledpin,OUTPUT);

  attachInterrupt(digitalPinToInterrupt(rpmpin), rpm_counter, RISING); 
  attachInterrupt(digitalPinToInterrupt(sparkpin), advance_counter, RISING);
  
  for (int thisReading = 0; thisReading < numReadings; thisReading++) 
   {
    readings[thisReading] = 0;
   }

  EEPROM.begin(10);
  EEPROM.get(3,pickup_advance);  
  EEPROM.end(); 

  if (pickup_advance>359)
  {
    pickup_advance=60;
  }

starttime=millis();

}

void loop() 
 {

if (((millis()-starttime)>60000) && (!saved)){
  EEPROM.begin(10);
  EEPROM.put(3,pickup_advance);  
  EEPROM.commit();
  saved=true;
}

 butt1.tick(); 

 if (butt1.isClick()) {
    pickup_advance++;
    saved=false;
    starttime=millis();
  }

if (butt1.isStep()) {
    pickup_advance--;
    saved=false;
    starttime=millis();    
  }

  if (pickup_advance>359)
  {
    pickup_advance=0;
  }

if (spark) 
  {
//digitalWrite(ledpin,HIGH);
delayMicroseconds(120);
digitalWrite(ledpin,LOW);
spark = false;
  }

 if ((millis()-last_update_rpm) >700)
{
rpmflag=true;
}
 
  
  

if (rpmupdated){
  
  noInterrupts();
  rpmupdated=false;
  duration_rpmTmp=duration_rpm;
  last_rpmTMP = last_rpm;
  interrupts();
  
 rpmai = 60000000/duration_rpmTmp;
 rpm = round (rpmai);
  
  total = total - readings[readIndex];
  readings[readIndex] = rpm;
  total = total + readings[readIndex];
  readIndex = readIndex + 1;
  if (readIndex >= numReadings)
      {
       readIndex = 0;
       }
  }

rpm = total / numReadings;

rpm=testFilterRA.filtered(float(rpm));

 advance= pickup_advance - ((duration_advance * 360) / duration_rpmTmp );
 

 if ( ((advance_a-5) < advance) && (advance < (advance_a+5)))
  {
  advance_a = advance;
  advance3 = advance3 + advance;
  counter_advance = counter_advance + 1;
  }
  else
   {
   advance_a=advance;
   }  

  last_update_rpm=millis();
  
}

 if ((millis()-last_show_rpm) >300)  //refresh rate

   rpm2=((rpm+5)/10)*10;


if (counter_advance>0){   
  advance2=advance3/counter_advance;
  counter_advance=0;
  advance3=0;
 }
 
  if (rpmflag)
   {
   rpm2=0;
   advance=0;
   rpmflag=false;
   }

    String rpm1 = String (rpm2);
    String advance1 = String (advance2);
    String pickup_advance1 = String (pickup_advance);
   
  Serial.print(rpm1);
  Serial.print(",");
  Serial.println(advance1);


    display.clear();
    display.setTextAlignment(TEXT_ALIGN_RIGHT);
    display.setFont(DSEG7_Classic_Mini_Bold_40);    
    display.drawString(128, 0, advance1);
    display.setFont(ArialMT_Plain_10);
    display.drawRect(0,0,128,64);
    display.drawString(60, 2, "Advance");
    display.drawHorizontalLine(0, 15, 60);    
    display.drawVerticalLine(60,15,37);
    display.drawString(60, 16, "PickupBTDC");   
    display.drawHorizontalLine(0, 52,61); 
    display.drawHorizontalLine(60,46,68); 
    display.drawString(60, 52, "RPM");
    display.setFont(Open_Sans_SemiBold_20);
    display.drawString(48, 26, "°");
    display.drawString(126, 40, rpm1);
    display.drawString(40, 26, pickup_advance1);   
    display.display();


  last_show_rpm= millis();

 }


yield(); 
}

ICACHE_RAM_ATTR void advance_counter()
{
  if (!advance_updated) {
   duration_advance = micros()-last_rpm;
   advance_updated = true;
   digitalWrite(ledpin,HIGH);
   spark = true;
  }
}


ICACHE_RAM_ATTR void rpm_counter()   //ISR for RPM
{
 if ((micros()-last_rpm)>5900)   //debounce signal. Take MAX RPM of engine and calculate time in micro seconds for full revolution: 
 {                               //   60 000 000/(max_RPM+couple_hundred_for_error)  ->  60 000 000 / (10 000 + 200) = 5882 -> rounded it to 5900
  duration_rpm = micros()-last_rpm;
  last_rpm = micros();
  rpmupdated=true;
  advance_updated = false;
 }
}

