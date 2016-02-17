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
    float humidity = dht.readHumidity();
    float temp = dht.readTemperature();
    float tempFromBarometr = bmp.readTemperature();
    long pressure = bmp.readPressure();
    int altitude = bmp.readAltitude();
    int light = analogRead(LIGHTPIN);
    int gas = analogRead(GASPIN);
    int flame = analogRead(FLAMEPIN);
    int vibro = analogRead(VIBROPIN);
    int button1 = analogRead(BUTTONPIN1);
    int button2 = analogRead(BUTTONPIN2);

  // start display and set view
    display.clearDisplay();
    display.setContrast(47);
    display.setTextColor(BLACK);    
    switch (DisplayIndex) {
      case 0 :
        ShowTime();
      break;
      case 1:
        ShowTempAndHumidity(temp, humidity);
      break;
      case 2:
        ShowPressureAndAltitude(pressure, altitude);
      break;
      case 3:
        ShowLightAndTemp2(light, tempFromBarometr);
      break;
      case 4:
        ShowGasAndFlame(gas, flame);
      break;
      case 5:
        ShowVibro(vibro);
      break;
    }
    display.display();
    
  // start functions
    CheckButtons(100, button1, button2, 5, 0);
    DisplayLedPowerOn(light, 100);
    PowerOnSignalExpression(flame, 900, 10, false);
    PowerOnSignalExpression(gas, 100, 10, true);
}

// function section
void CheckButtons (int DefaultLimit, int Decrement, int Increment, int MaxLimit, int TimeDelay) {
  if ((Decrement > DefaultLimit) && (DisplayIndex > 0)) {
    DisplayIndex--;
    delay(TimeDelay);
  } 
  if ((Increment > DefaultLimit) && (DisplayIndex < MaxLimit)) {
    DisplayIndex++;
    delay(TimeDelay);
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

  // views
  void ShowTime () {
    DateTime rtcnow = rtc.now();
    display.setCursor(18, 0);
    display.setTextSize(1); 
    char daysOfTheWeek[7][22] = {"Недiля", "Понедiлок", "Вiвторок", "Середа", "Четвер", "П\'ятниця", "Субота"};
    display.print(daysOfTheWeek[rtcnow.dayOfTheWeek()]);
    display.setCursor(14, 10);
    display.println(rtcnow.day(), DEC);
    display.print("/");
    display.print(rtcnow.month(), DEC);
    display.print("/");
    display.print(rtcnow.year(), DEC);
    display.setCursor(12, 25);
    display.setTextSize(2);
    display.print(rtcnow.hour(), DEC);
    display.print("-");
    display.print(rtcnow.minute(), DEC);
    display.setTextSize(1);
    display.setCursor(36, 22);
    display.print(rtcnow.second(), DEC);
  }
  void ShowTempAndHumidity (int Temp, int Humidity) {
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("Температура:");
    display.setCursor(10, 10);
    display.print(Temp);
    display.print(" градусiв");
    display.setCursor(0, 25);
    display.print("Вологiсть: ");
    display.setCursor(10, 35);
    display.print(Humidity);
    display.print(" процентiв");
  }
  void ShowPressureAndAltitude(int pressure, int altitude) {
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("Тиск:");
    display.setCursor(5, 10);
    display.print(pressure, DEC);
    display.print(" паск.");
    display.setCursor(0, 25);
    display.print("Висота: ");
    display.setCursor(10, 35);
    display.print(altitude, DEC);
    display.print(" метрiв");
  }
  void ShowLightAndTemp2 (int light, int temp) {
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("Освiтлення:");
    display.setCursor(10, 10);
    display.print(light, DEC);
    display.print(" число");
    display.setCursor(0, 20);
    display.print("Температура з барометра: ");
    display.setCursor(10, 38);
    display.print(temp, DEC);
    display.print(" градусiв");
  }
  void ShowGasAndFlame (int gas, int flame) {
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("Гази:");
    display.setCursor(10, 10);
    display.print(gas, DEC);
    display.print(" число");
    display.setCursor(0, 25);
    display.print("Вогонь: ");
    display.setCursor(10, 35);
    display.print(flame, DEC);
    display.print(" число");
  }
  void ShowVibro (int vibro) {
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print("Вiбрацiя:");
    display.setCursor(10, 10);
    display.print(vibro, DEC);
    display.print(" число");
  }

