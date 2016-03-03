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
  //Serial.begin(9600);
  
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
   
  // init temp
  dht.begin();

  // init barometr
  if (! bmp.begin()) {
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
    //rtc.adjust(DateTime(F(__DATE__), F(__TIME__)));
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
    boolean editButton = digitalRead(EDITPIN);
    byte contrast = EEPROM.read(3);
    
  // start functions
    InitEditMode(editButton, 7);
    CheckButtons(100, button1, button2, 5, 0);
    DisplayLedPowerOn(light, EEPROM.read(0));
    PowerOnSignalExpression(gas, EEPROM.read(1), 50, true);
    PowerOnSignalExpression(flame, EEPROM.read(2), 50, false);

  // start display and set view
    display.clearDisplay();
    display.setContrast(contrast); // for first (before edit) run set 50
    display.setTextColor(BLACK);
    if (EditMode) {
      switch (DisplayIndex) {
        case 0 :
          DefaultTempleteEditTime(2000, 3000, "Виберiть рiк:", 0);
        break;
        case 1 :
          DefaultTempleteEditTime(1, 12, "Виберiть мiсяць:", 1);
        break;
        case 2 :
          DefaultTempleteEditTime(1, 31, "Виберiть день:", 2);
        break;
        case 3 :
          DefaultTempleteEditTime(0, 23, "Виберiть годину:", 3);
        break;
        case 4 :
          DefaultTempleteEditTime(0, 59, "Виберiть хвилину:", 4);
        break;
        case 5 :
          rtc.adjust(DateTime(TimeConfig[0], TimeConfig[1], TimeConfig[2], TimeConfig[3], TimeConfig[4], 0)); //year, month, day, hour, minute
          DefaultTempleteEditEpprom(0, 1024, "Виберiть лiмiт освiтлення:", 0);
        break;
        case 6 :
          DefaultTempleteEditEpprom(0, 1024, "Виберiть лiмiт газу:", 1);
        break;
        case 7 :
          DefaultTempleteEditEpprom(0, 1024, "Виберiть лiмiт вогню:", 2);
        break;
        case 8 :
          DefaultTempleteEditEpprom(20, 100, "Виберiть контраст:", 3);
        break;
      }
    } else {    
      switch (DisplayIndex) {
        case 0 :
          ShowTime();
        break;
        case 1:
          DefaultTempleteShow("Температура:", temp, " градусiв", "Вологiсть: ", humidity, " процентiв");
        break;
        case 2:
          DefaultTempleteShow("Тиск:", pressure, " паск.", "Висота: ", altitude, " метрiв");
        break;
        case 3:
          DefaultTempleteShow("Освiтлення:", light, " число", "Температура з барометра: ", tempFromBarometr, " градусiв");
        break;
        case 4:
          DefaultTempleteShow("Гази:", gas, " число", "Вогонь: ", flame, "  число");
        break;
        case 5:
          DefaultTempleteShow("Вiбрацiя:", vibro, "  число", "", 0, "");
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
   DateTime now = rtc.now();
   display.setCursor(18, 0);
   display.setTextSize(1); 
   char daysOfTheWeek[7][18] = {"Недiля", "Понедiлок", "Вiвторок", "Середа", "Четвер", "П\'ятниця", "Субота"};
   display.print(daysOfTheWeek[now.dayOfTheWeek()]);
   display.setCursor(14, 10);
   display.println(now.day(), DEC);
   display.print("/");
   display.print(now.month(), DEC);
   display.print("/");
   display.print(now.year(), DEC);
   display.setCursor(12, 25);
   display.setTextSize(2);
   display.print(now.hour(), DEC);
   display.print("-");
   display.print(now.minute(), DEC);
   display.setTextSize(1);
   display.setCursor(36, 22);
   display.print(now.second(), DEC); 
}

void DefaultTempleteShow (char caption1[], long value1, char text1[], char caption2[], long value2, char text2[]) {
  display.setCursor(0, 0);
  display.setTextSize(1);
  display.print(caption1);
  display.setCursor(10, 10);
  display.print(value1);
  display.print(text1);
  if (value2 != 0) {
    display.setCursor(0, 22);
    display.print(caption2);
    display.setCursor(10, 39);
    display.print(value2);
    display.print(text2);
  }
}

//edit mode
void DefaultTempleteEditEpprom (int minLimit, int maxLimit, char text[], int epprom ) {
    if ((EditModeValue < minLimit) or (EditModeValue > maxLimit)) {
       EditModeValue = minLimit;
    } 
    EEPROM.write(epprom, EditModeValue);
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(text);
    display.setCursor(18, 20);
    display.setTextSize(2);
    display.print(EditModeValue);
}

void DefaultTempleteEditTime (int minLimit, int maxLimit, char text[], int index ) {
    if ((EditModeValue < minLimit) or (EditModeValue > maxLimit)) {
       EditModeValue = minLimit;
    } 
    TimeConfig[index] = EditModeValue;
    display.setCursor(0, 0);
    display.setTextSize(1);
    display.print(text);
    display.setCursor(18, 20);
    display.setTextSize(2);
    display.print(TimeConfig[index]);
}
