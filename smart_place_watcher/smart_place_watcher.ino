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
#define EDITPIN 8

// include section
#include <EEPROM.h>
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
int EditModeValue = 0;
boolean EditMode = false;
int TimeConfig[5];

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
  pinMode(EDITPIN, INPUT);
  pinMode(BUZZERPIN, OUTPUT);
  pinMode(VIBROGROUNDPIN, OUTPUT);
  
  // init vibrosensor
  digitalWrite(VIBROGROUNDPIN, LOW);
  
  // init display
  display.begin();
  display.setContrast(EEPROM.read(0) || 50);
   
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
  Serial.println(digitalRead(EDITPIN));
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
    boolean editButton = digitalRead(EDITPIN);
    
  // start functions
    InitEditMode(editButton, 4);
    CheckButtons(100, button1, button2, 5, 0);
    DisplayLedPowerOn(light, 100);
    PowerOnSignalExpression(flame, 700, 50, false);
    PowerOnSignalExpression(gas, 100, 50, true);

  // start display and set view
    display.clearDisplay();
    display.setTextColor(BLACK);
    if (EditMode) {
      switch (DisplayIndex) {
        case 0 :
          SetYear();
        break;
        case 1 :
          SetMonth();
        break;
        case 2 :
          SetDay();
        break;
        case 3 :
          SetHour();
        break;
        case 4 :
          SetMinute();
        break;
        case 5 :
          rtc.adjust(DateTime(TimeConfig[0], TimeConfig[1], TimeConfig[2], TimeConfig[3], TimeConfig[4], 0)); //year, month, day, hour, minute
          SetContrast();
        break;
      }
    } else {    
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
    }
    display.display();
}

// function section
void InitEditMode (boolean value, int Limit) {
  if (!value) {
    if (EditMode) {
      if (DisplayIndex > Limit) {
        DisplayIndex = 0;
        EditMode = false;
      } else {
        DisplayIndex++;
      }    
    } else {
      EditMode = true;
      DisplayIndex = 0;
    }
  }
}

void CheckButtons (int DefaultLimit, int Decrement, int Increment, int MaxLimit, int TimeDelay) {
  if (EditMode) {
    if ((Decrement > DefaultLimit)) {
      EditModeValue--;
      delay(TimeDelay);
    } 
    if ((Increment > DefaultLimit)) {
      EditModeValue++;
      delay(TimeDelay);
    }
  } else {
    if ((Decrement > DefaultLimit) && (DisplayIndex > 0)) {
      DisplayIndex--;
      delay(TimeDelay);
    } 
    if ((Increment > DefaultLimit) && (DisplayIndex < MaxLimit)) {
      DisplayIndex++;
      delay(TimeDelay);
    }
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
  display.println(rtcnow.day());
  display.print("/");
  display.print(rtcnow.month());
  display.print("/");
  display.print(rtcnow.year());
  display.setCursor(12, 25);
  display.setTextSize(2);
  display.print(rtcnow.hour());
  display.print("-");
  display.print(rtcnow.minute());
  display.setTextSize(1);
  display.setCursor(36, 22);
  display.print(rtcnow.second());
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

void ShowPressureAndAltitude(long pressure, int altitude) {
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Тиск:");
  display.setCursor(5, 10);
  display.print(pressure);
  display.print(" паск.");
  display.setCursor(0, 25);
  display.print("Висота: ");
  display.setCursor(10, 35);
  display.print(altitude);
  display.print(" метрiв");
}

void ShowLightAndTemp2 (int light, int temp) {
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Освiтлення:");
  display.setCursor(10, 10);
  display.print(light);
  display.print(" число");
  display.setCursor(0, 20);
  display.print("Температура з барометра: ");
  display.setCursor(10, 38);
  display.print(temp);
  display.print(" градусiв");
}

void ShowGasAndFlame (int gas, int flame) {
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Гази:");
  display.setCursor(10, 10);
  display.print(gas);
  display.print(" число");
  display.setCursor(0, 25);
  display.print("Вогонь: ");
  display.setCursor(10, 35);
  display.print(flame);
  display.print(" число");
}

void ShowVibro (int vibro) {
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Вiбрацiя:");
  display.setCursor(10, 10);
  display.print(vibro);
  display.print(" число");
}

//edit mode
void SetYear() {
  if ((EditModeValue < 2000) or (EditModeValue > 3000)) {
     EditModeValue = 2000;
  } 
  TimeConfig[0] = EditModeValue;
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Виберiть рiк:");
  display.setCursor(18, 20);
  display.setTextSize(2);
  display.print(TimeConfig[0]);
}

void SetMonth() {
  if ((EditModeValue < 1) or (EditModeValue > 12)) {
     EditModeValue = 1;
  } 
  TimeConfig[1] = EditModeValue;
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Виберiть мiсяць:");
  display.setCursor(18, 20);
  display.setTextSize(2);
  display.print(TimeConfig[1]);
}

void SetDay() {
  if ((EditModeValue < 1) or (EditModeValue > 31)) {
     EditModeValue = 1;
  } 
  TimeConfig[2] = EditModeValue;
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Виберiть день:");
  display.setCursor(18, 20);
  display.setTextSize(2);
  display.print(TimeConfig[2]);
}

void SetHour() {
  if ((EditModeValue < 0) or (EditModeValue > 23)) {
     EditModeValue = 0;
  } 
  TimeConfig[3] = EditModeValue;
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Виберiть годину:");
  display.setCursor(18, 20);
  display.setTextSize(2);
  display.print(TimeConfig[3]);
}

void SetMinute() {
  if ((EditModeValue < 0) or (EditModeValue > 59)) {
     EditModeValue = 0;
  } 
  TimeConfig[4] = EditModeValue;
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Виберiть хвилину:");
  display.setCursor(18, 20);
  display.setTextSize(2);
  display.print(TimeConfig[4]);
}

void SetContrast() {
  if ((EditModeValue < 0) or (EditModeValue > 100)) {
     EditModeValue = 0;
  } 
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print("Виберiть рiвень контрасту:");
  display.setCursor(18, 25);
  display.setTextSize(2);
  EEPROM.write(0, EditModeValue);
  display.print(EEPROM.read(0));
}
