
#include "PM_Sensor.h"

#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define OLED_RESET 16 //LED_BUILTIN  //4
Adafruit_SSD1306 display(OLED_RESET);

#if (SSD1306_LCDHEIGHT != 64)
#error("Height incorrect, please fix Adafruit_SSD1306.h!");
#endif

// Arduino pin numbers.
const int sharpLEDPin = D3; // Arduino digital pin D3 connect to sensor LED.
const int sharpVoPin = A0; // Arduino analog pin A0 connect to sensor Vo.

void setup()
{
  // Debug console
  Serial.begin(115200);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3c);
  pinMode(sharpLEDPin, OUTPUT);
}
int ton=0;
void loop()
{
  PM_loop();
}


void PM_loop(){
// Turn on the dust sensor LED by setting digital pin LOW.
digitalWrite(sharpLEDPin, LOW);
// Wait 0.28ms before taking a reading of the output voltage as per spec.
delayMicroseconds(280);
// Record the output voltage. This operation takes around 100 microseconds.
int VoRaw = analogRead(sharpVoPin);
// Turn the dust sensor LED off by setting digital pin HIGH.
digitalWrite(sharpLEDPin, HIGH);
// Wait for remainder of the 10ms cycle = 10000 - 280 - 100 microseconds.
delayMicroseconds(280);
// Print raw voltage value (number from 0 to 1023).
#ifdef PRINT_RAW_DATA
//printValue("VoRaw", VoRaw, true);
Serial.println("");
#endif // PRINT_RAW_DATA
 
// Use averaging if needed.
float Vo = VoRaw;

#ifdef USE_AVG
VoRawTotal += VoRaw;
VoRawCount++;
if ( VoRawCount >= N ) {
Vo = 1.0 * VoRawTotal / N;
VoRawCount = 0;
VoRawTotal = 0;
} else {
return;
}
#endif // USE_AVG
 
// Compute the output voltage in Volts.
Vo = Vo / 1024.0 * 3.3;
//printFValue("Vo", Vo, "V");
 
// Convert to Dust Density in units of ug/m3.
float dV = Vo - Voc;
if ( dV < 0 ) {
dV = 0;
Voc = Vo;
}
float dustDensity = dV / K * 100.0;
send_blynk(dustDensity);
  if(ton == 0){tone(D5, 0, 50);}
  if(ton == 1){tone(D5, 1000, 50);delay(50);tone(D5, 0, 50);delay(50);}
  if(ton == 2){tone(D5, 2500, 50);delay(50);tone(D5, 0, 50);delay(50);}
}


void send_blynk(float Density){
     display.clearDisplay();
     display.setTextSize(3);
    display.setTextColor(WHITE);
    display.setCursor(0,16);
    display.println(Density);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.setCursor(85,0);
    display.println("ug/m3");
    display.display();
   
   if(Density > 0.0 && Density < 101.0){ ton=0;}
   if(Density > 100 && Density < 201.0){ ton=1;}
   if(Density > 200){ ton=2;}
   
  }

  
