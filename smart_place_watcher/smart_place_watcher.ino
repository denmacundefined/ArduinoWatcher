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
#define VIBROGROUNDPIN 13
#define FLAMEPIN A0
#define LIGHTPIN A6
#define BUZZERPIN 10
#define DEBUGTXPIN 12
#define DEBUGRXPIN 11

// include section
#include <SPI.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>
#include <DHT.h>
#include <Adafruit_BMP085.h>
#include <Wire.h>
#include <RTClib.h>
#include <SoftwareSerial.h>

// global variable section
RTC_DS1307 rtc;
Adafruit_PCD8544 display = Adafruit_PCD8544(DISPLAYSCLKPIN, DISPLAYDNPIN, DISPLAYDCPIN, DISPLAYSCEPIN, DISPLAYRSTPIN);
DHT dht(DHTPIN, DHTTYPE);
Adafruit_BMP085 bmp;
SoftwareSerial DEBUG(DEBUGRXPIN, DEBUGTXPIN);
int DisplayIndex = 0;

// config section
void setup() {
  
  //debug
  DEBUG.begin(9600);
  
  // set pin type 
  pinMode(BUTTONPIN1, INPUT);
  pinMode(BUTTONPIN2, INPUT);
  pinMode(GASPIN, INPUT);
  pinMode(VIBROPIN, INPUT);
  pinMode(FLAMEPIN, INPUT);
  pinMode(DISPLAYLEDPIN, OUTPUT);
  pinMode(LIGHTPIN, INPUT);
  pinMode (BUZZERPIN, OUTPUT);
  pinMode (VIBROGROUNDPIN, OUTPUT);
  
  // init vibrosensor
  digitalWrite(VIBROGROUNDPIN, LOW);
  
  // init display
  display.begin();
   
  // init temp
  dht.begin();

  // init barometr
  if (!bmp.begin()) {
    DEBUG.println("Could not find BMP085 sensor");
    while (1);
  }
  
  // init time 
  if (! rtc.begin()) {
    DEBUG.println("Could not find RTC");
    while (1);
  }
  if (! rtc.isrunning()) {
    DEBUG.println("RTC is NOT running!");
    rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
  }
  
  // init wifi
  /*Serial.begin(115200);
  Serial.setTimeout(5000);
  Serial.println("AT+RST");
  delay(1000);
  DEBUG.println(Serial.readString());
  /*if (Serial.find("ready")) {
    DEBUG.print("WiFi - Module is ready");
  }else{
    DEBUG.print("Module dosn't respond.");
    while(1);
  }*/
  
}

// main section
void loop() { 
  
  // get data from sensors
    float h = dht.readHumidity();
    float t = dht.readTemperature();
    float t2 = bmp.readTemperature();
    long pressure = bmp.readPressure();
    int altitude = bmp.readAltitude();
    int light = analogRead(LIGHTPIN);
    int gas = analogRead(GASPIN);
    int flame = analogRead(FLAMEPIN);
    int vibro = analogRead(VIBROPIN);

  // start display
    display.clearDisplay();
    display.setContrast(47);
    display.setTextColor(BLACK);

  // start functions
    CheckButtons(300);
    //DisplayLedPowerOn(light, 100);
    //PowerOnSignalExpression(flame, 250, 10, false);
    //PowerOnSignalExpression(gas, 250, 10, true);
    //PowerOnSignalExpression(vibro, 500, 10, true);

    DEBUG.printls(analogRead(BUTTONPIN1));
    DEBUG.printls(analogRead(BUTTONPIN2));    
    
  // show information on display
    SetView();
    display.display();
}

// function section
void CheckButtons (int DefaultLimit) {
  if (analogRead(BUTTONPIN1) > DefaultLimit && DefaultLimit > 0) {
    DisplayIndex--;
  } 
  if (analogRead(BUTTONPIN2) > DefaultLimit && DefaultLimit >= 0) {
    DisplayIndex++;
  }
}
void PowerOnSignalExpression (int value, int checkValue, int interval, boolean more) {
  boolean validation;
  if (more) {
      validation = (value > checkValue) ? true : false;
  } else {
      validation = (value < checkValue) ? true : false;
  }
  if (validation) {
    analogWrite (BUZZERPIN, 255);
    delay (interval);
    analogWrite (BUZZERPIN, 0);
  }
}
void SetTime (int year, int month, int day, int hour, int minute, int second) { 
  rtc.adjust(DateTime(year, month, day, hour, minute, second));
}
void DisplayLedPowerOn (int light, int DefaultLimit) {
  if (light < DefaultLimit) {
    digitalWrite(DISPLAYLEDPIN, HIGH);
  } else {
    digitalWrite(DISPLAYLEDPIN, LOW);
  }  
}
void SetView () {
  switch (DisplayIndex) {
    case 0 :
      ShowTime();
    break;
  }
}

  // views
  void ShowTime () {
    DateTime rtcnow = rtc.now();
    display.setCursor(0, 0);
    display.setTextSize(3); 
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
  }

