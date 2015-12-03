#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
Adafruit_PCD8544 display = Adafruit_PCD8544(2, 3, 4, 6, 5);
#include "DHT.h"
#define DHTPIN 9
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);
#include <Adafruit_BMP085.h>
#include <Wire.h>
Adafruit_BMP085 bmp;
#include "RTClib.h"
RTC_DS1307 rtc;
char daysOfTheWeek[7][12] = {"Нед.", "Пон.", "Вiв.", "Сер.", "Чет.", "Пят.", "Суб."};

void setup() {
  //debug
  Serial.begin(9600);
  
  pinMode(7, OUTPUT);
  pinMode(A6, INPUT);
  display.begin();
  clearDisplay();
  dht.begin();
  if (!bmp.begin()) {
	Serial.println("Could not find a valid BMP085 sensor, check wiring!");
	while (1) {}
  }
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    Serial.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  //rtc.adjust(DateTime(2015, 12, 2, 19, 8, 0));
}

void clearDisplay() {
  display.clearDisplay();
  display.display(); 
}

void loop() { 
  DateTime now = rtc.now();
  float h = dht.readHumidity();
  float t = dht.readTemperature();
  float t2 = bmp.readTemperature();
  long pressure = bmp.readPressure();
  int altitude = bmp.readAltitude();
  int light = analogRead(A6);
  if (light < 100) {
    digitalWrite(7, HIGH);
  } else {
    digitalWrite(7, LOW);
  }
  display.setContrast(40);
  display.setCursor(0, 0);
  display.setTextColor(BLACK);
  display.setTextSize(1);
  clearDisplay();
  
  display.print(now.year(), DEC);
  display.print("/");
  display.print(now.month(), DEC);
  display.print("/");
  display.print(now.day(), DEC);
  display.print(" ");
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
}

