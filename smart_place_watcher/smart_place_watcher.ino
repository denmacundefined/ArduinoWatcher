// define section
#define DHTPIN 9
#define DHTTYPE DHT11
#define DISPLAYSCLKPIN 2
#define DISPLAYDNPIN 3
#define DISPLAYDCPIN 4
#define DISPLAYRSTPIN 5
#define DISPLAYSCEPIN 6
#define DISPLAYLEDPIN 7
#define BUTTONPIN1 A7
#define BUTTONPIN2 A3
#define GASPIN A2
#define VIBROPIN A1
#define FLAMEPIN A0
#define LIGHTPIN A6
#define BUZZERPIN 10

// include section
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <RTClib.h>

// class section


// create object section
RTC_DS1307 rtc;
Adafruit_PCD8544 display = Adafruit_PCD8544(DISPLAYSCLKPIN, DISPLAYDNPIN, DISPLAYDCPIN, DISPLAYSCEPIN, DISPLAYRSTPIN);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;

// config section
void setup() {
  
  // set pin type 
  pinMode(BUTTONPIN1, INPUT);
  pinMode(BUTTONPIN2, INPUT);
  pinMode(GASPIN, INPUT);
  pinMode(VIBROPIN, INPUT);
  pinMode(FLAMEPIN, INPUT);
  pinMode(DISPLAYLEDPIN, OUTPUT);
  pinMode(LIGHTPIN, INPUT);
  pinMode (BUZZERPIN, OUTPUT);
  
  // init display
  display.begin();
   
  // init temp
  dht.begin();
  if (!bmp.begin()) {
    Serial.println("Could not find a valid BMP085 sensor, check wiring!");
    while (1);
  }
  
  // init time 
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //rtc.adjust(DateTime(2016, 1, 4, 14, 05, 0));
}

// main section
void loop() { 
  String userTime = "";
  DateTime now = rtc.now();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float t2 = bmp.readTemperature();
  long pressure = bmp.readPressure();
  int altitude = bmp.readAltitude();
  int light = analogRead(LIGHTPIN);
  if (light < 100) {
    digitalWrite(DISPLAYLEDPIN, HIGH);
  } else {
    digitalWrite(DISPLAYLEDPIN, LOW);
  }
  display.clearDisplay();
  display.setContrast(47);
  display.setCursor(0, 0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  display.print(now.year(), DEC);
  display.print("/");
  display.print(now.month(), DEC);
  display.print("/");
  display.print(now.day(), DEC);
  display.print(" ");
  char daysOfTheWeek[7][12] = {"Нед.", "Пон.", "Вiв.", "Сер.", "Чет.", "Пят.", "Суб."};
  display.print(daysOfTheWeek[now.dayOfTheWeek()]);
  display.print(now.hour(), DEC);
  display.print(":");
  display.print(now.minute(), DEC);
  display.print(":");
  display.print(now.second(), DEC);
  display.print("(");
  display.print(h);
  display.print(" % вод.)(");
  display.print(t);
  display.print(" град.)(");
  display.print(pressure);
  display.print(" паск.)(");
  display.print(altitude);
  display.print(" метр.)(");
  display.print(t2);
  display.print(" град.2)");  
  display.display(); 
  Serial.println(analogRead(BUTTONPIN1));
  Serial.println(analogRead(BUTTONPIN2));
  Serial.println(analogRead(GASPIN)); //gas
  //Serial.println(analogRead(VIBROPIN)); //vibro
  //Serial.println(analogRead(FLAMEPIN)); //flash
  if (analogRead(FLAMEPIN) < 900 or analogRead(GASPIN) > 200) {
    analogWrite (BUZZERPIN, 255);
    delay (50);
    analogWrite (BUZZERPIN, 0);
  }
}

