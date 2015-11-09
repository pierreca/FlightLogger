#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_HMC5883_U.h>
//#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <SD.h>
#include "IntersemaBaro.h"

// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_CS     10
#define TFT_RST    8  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to 0!
#define TFT_DC     9
#define SD_CS 7

// Option 1 (recommended): must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

// Option 2: use any pins but a little slower!
#define TFT_SCLK 13   // set these to be whatever pins you like!
#define TFT_MOSI 11   // set these to be whatever pins you like!
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);

Adafruit_HMC5883_Unified mag = Adafruit_HMC5883_Unified(12345);

Intersema::BaroPressure_MS5607B baro(true);

float p = 3.1415926;
File root;

void displaySensorDetails(void)
{
  sensor_t sensor;
  mag.getSensor(&sensor);
  Serial.println("------------------------------------");
  Serial.print  ("Sensor:       "); Serial.println(sensor.name);
  Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
  Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
  Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" uT");
  Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" uT");
  Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" uT");  
  Serial.println("------------------------------------");
  Serial.println("");
  delay(500);
}

//void tftPrintCompassData(float heading, float altFeet) {
//  tft.setTextWrap(false);
//  tft.fillScreen(ST7735_BLACK);
//  tft.setCursor(0, 30);
//  tft.setTextColor(ST7735_RED);
//  tft.setTextSize(1);
//  tft.print("Heading: ");
//  tft.println(heading);
//  tft.print("Altitude (ft): ");
//  tft.println(altFeet);
//  delay(500);
//}

void printDirectory(File dir, int numTabs) {
  // Begin at the start of the directory
  dir.rewindDirectory();
  
  while(true) {
     File entry =  dir.openNextFile();
     if (! entry) {
       // no more files
       //Serial.println("**nomorefiles**");
       break;
     }
     for (uint8_t i=0; i<numTabs; i++) {
       Serial.print('\t');   // we'll have a nice indentation
     }
     // Print the 8.3 name
     Serial.print(entry.name());
     // Recurse for directories, otherwise print the file size
     if (entry.isDirectory()) {
       Serial.println("/");
       printDirectory(entry, numTabs+1);
     } else {
       // files have sizes, directories do not
       Serial.print("\t\t");
       Serial.println(entry.size(), DEC);
     }
     entry.close();
   }
}

void initSD() {
  Serial.println("SD Initializing...");
  pinMode(SS, OUTPUT);
  if (!SD.begin(SD_CS)) {
    Serial.println("SD Initialization failed");
    return;
  }
  
  Serial.println("SD Initialized.");
  root = SD.open("/");
  printDirectory(root,0);
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.println("Hello!");
  Serial.println("ST7735 TFT Initializing...");

  // Use this initializer if you're using a 1.8" TFT
//  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab
  Serial.println("ST7735 TFT Initialized.");
  
  Serial.println("HMC5883 Magnetometer Initializing..."); Serial.println("");
  
  /* Initialise the sensor */
  if(!mag.begin())
  {
    /* There was a problem detecting the HMC5883 ... check your connections */
    Serial.println("Ooops, no HMC5883 detected ... Check your wiring!");
    while(1);
  }
  
  /* Display some basic information on this sensor */
  displaySensorDetails();
  
  uint16_t time = millis();
//  tft.fillScreen(ST7735_BLACK);
  time = millis() - time;

  Serial.println(time, DEC);

  Serial.println("MS5607 (Altimeter) Initializing...");
  baro.init();
  Serial.println("MS5607 (Altimeter) Initialized.");

  initSD();
  
  delay(500);
}

void loop() {
  /* Get a new sensor event */ 
  sensors_event_t event; 
  mag.getEvent(&event);
 
  /* Display the results (magnetic vector values are in micro-Tesla (uT)) */
  Serial.print("X: "); Serial.print(event.magnetic.x); Serial.print("  ");
  Serial.print("Y: "); Serial.print(event.magnetic.y); Serial.print("  ");
  Serial.print("Z: "); Serial.print(event.magnetic.z); Serial.print("  ");Serial.println("uT");

  // Hold the module so that Z is pointing 'up' and you can measure the heading with x&y
  // Calculate heading when the magnetometer is level, then correct for signs of axis.
  float heading = atan2(event.magnetic.y, event.magnetic.x);
  
  // Once you have your heading, you must then add your 'Declination Angle', which is the 'Error' of the magnetic field in your location.
  // Find yours here: http://www.magnetic-declination.com/
  // Mine is: -13* 2' W, which is ~13 Degrees, or (which we need) 0.22 radians
  // If you cannot find your Declination, comment out these two lines, your compass will be slightly off.
  float declinationAngle = 0.28;
  heading += declinationAngle;
  
  // Correct for when signs are reversed.
  if(heading < 0)
    heading += 2*PI;
    
  // Check for wrap due to addition of declination.
  if(heading > 2*PI)
    heading -= 2*PI;
   
  // Convert radians to degrees for readability.
  float headingDegrees = heading * 180/M_PI; 
  
  Serial.print("Heading (degrees): "); Serial.println(headingDegrees);
  
  int altCm = baro.getHeightCentiMeters();
  float altFeet = (float)(altCm) / 30.48;
  Serial.print("Centimeters: ");
  Serial.print((float)(altCm));
  Serial.print(", Feet: ");
  Serial.println(altFeet); // feet to cm conversion 
  
//  tftPrintCompassData(headingDegrees, altFeet);
  delay(500);
}

/*

void tftPrintTest() {
  tft.setTextWrap(false);
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0, 30);
  tft.setTextColor(ST7735_RED);
  tft.setTextSize(1);
  tft.println("Hello World!");
  tft.setTextColor(ST7735_YELLOW);
  tft.setTextSize(2);
  tft.println("Hello World!");
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(3);
  tft.println("Hello World!");
  tft.setTextColor(ST7735_BLUE);
  tft.setTextSize(4);
  tft.print(1234.567);
  delay(1500);
  tft.setCursor(0, 0);
  tft.fillScreen(ST7735_BLACK);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(0);
  tft.println("Hello World!");
  tft.setTextSize(1);
  tft.setTextColor(ST7735_GREEN);
  tft.print(p, 6);
  tft.println(" Want pi?");
  tft.println(" ");
  tft.print(8675309, HEX); // print 8,675,309 out in HEX!
  tft.println(" Print HEX!");
  tft.println(" ");
  tft.setTextColor(ST7735_WHITE);
  tft.println("Sketch has been");
  tft.println("running for: ");
  tft.setTextColor(ST7735_MAGENTA);
  tft.print(millis() / 1000);
  tft.setTextColor(ST7735_WHITE);
  tft.print(" seconds.");
}
 */

