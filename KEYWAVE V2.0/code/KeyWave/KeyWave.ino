#include <Servo.h>
#include <Adafruit_GPS.h>
#include <GPRS_Shield_Arduino.h>
#include <SoftwareSerial.h>
#include <Wire.h>
#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
#include <PN532_I2C.h>
#include <PN532Interface.h>
#include <PN532.h>


//  ----------------------PIN DEFINITION
#define PIN_GPS_TX        10
#define PIN_GPS_RX        11
#define PIN_SIM808_PWR     9
#define SIM900_PIN_RX      8
#define SIM900_PIN_TX      7

#define PIN_RF_RECEIVER    3
#define PIN_BUZZER         4
#define PIN_SERVO          5

#define PIN_BP             2

#define LED_GREEN         A0
#define LED_RED           A1
#define LED_BLUE          A2
#define LED_YELLOW        A3

#define SERIAL Serial
//GPRS VAR
//const char ADMIN_NUMBER[] = "+33600000000"; 
const char ADMIN_NUMBER[]   = "+33600000000"; 


//GPRS CONST
#define MESSAGE_LENGTH 160
char message[MESSAGE_LENGTH];
char phone[16];
char datetime[24];
char new_code[5] = "12345";
int messageIndex = 0;

//SERVO CONST
const int SERVO_LOCKPOS = 130;
const int SERVO_UNLOCKPOS = 50;
const int SERVO_NORMPOS = 90;
const int SERVO_DELAY = 1000;

//LED CONST
boolean LED_STATE = false;

Servo servo;
GPRS gprs(SIM900_PIN_TX, SIM900_PIN_RX, 9600); //RX,TX,PWR,BaudRate
SoftwareSerial GPS_SERIAL(PIN_GPS_TX, PIN_GPS_RX);
Adafruit_GPS GPS(&GPS_SERIAL);
SSD1306AsciiWire oled;
PN532_I2C pn532_i2c(Wire);
PN532 nfc(pn532_i2c);

char* string2char(String command) {
  if (command.length() != 0) {
    char *p = const_cast<char*>(command.c_str());
    return p;
  }
}


//  ----------------------SERVO
void SERVO_INIT() {
  servo.attach(PIN_SERVO);
  servo.write(SERVO_NORMPOS);
  delay(100);
  servo.detach();
}

void SERVO_UNLOCK() {
  servo.attach(PIN_SERVO);
  servo.write(SERVO_UNLOCKPOS);
  delay(100);
  servo.detach();

  delay(SERVO_DELAY);

  servo.attach(PIN_SERVO);
  servo.write(SERVO_NORMPOS);
  delay(100);
  servo.detach();

  digitalWrite(LED_GREEN, HIGH);
  oled.clear();
  oled.set2X();
  oled.println("\n  UNLOCKED");
  buzzer_pulse(1, 100);
  delay(1000);
  digitalWrite(LED_GREEN, LOW);
  gprs.sendSMS(ADMIN_NUMBER, "CAR OPEN");
  delay(1000);
  oled.clear();
  oled.println("\n  KeyWave");
}

void SERVO_LOCK() {
  servo.attach(PIN_SERVO);
  servo.write(SERVO_LOCKPOS);
  delay(100);
  servo.detach();

  delay(SERVO_DELAY);

  servo.attach(PIN_SERVO);
  servo.write(SERVO_NORMPOS);
  delay(100);
  servo.detach();

  digitalWrite(LED_RED, HIGH);
  oled.clear();
  oled.set2X();
  oled.println("\n  LOCKED");
  buzzer_pulse(1, 100);
  delay(1000);
  digitalWrite(LED_RED, LOW);

  gprs.sendSMS(ADMIN_NUMBER, "CAR CLOSE");
  delay(1000);
  oled.clear();
  oled.println("\n  KeyWave");
}

//  ----------------------LCD
void LCD_INIT() {
  Wire.begin();
  oled.begin(&Adafruit128x32, 0x3C);
  oled.set400kHz();
  oled.setFont(Adafruit5x7);

  oled.clear();
  oled.set2X();
  oled.println("\n  KeyWave");
}



//  ----------------------SIM808
void SIM808_INIT() {
  // pinMode(PIN_SIM808_PWR, OUTPUT);
  // digitalWrite(PIN_SIM808_PWR, HIGH);
  gprs.checkPowerUp();
  while (!gprs.init()) SERIAL.print(".");
  SERIAL.println("SIM808 OK");
  gprs.sendSMS(ADMIN_NUMBER, "READY");
}

//  ----------------------GPS
void GPS_INIT() {
  GPS.begin(9600);
  GPS_SERIAL.listen();
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);
  GPS.sendCommand(PGCMD_ANTENNA);
  delay(1000);
}


void GPS_SEND_VALUES() {
  GPS_SERIAL.listen();
  while (1) {
    char c = GPS.read();
    if (GPS.newNMEAreceived())
      if (!GPS.parse(GPS.lastNMEA())) GPS_SEND_VALUES();
      else break;
  }
  SERIAL.print("\nTime: ");
  SERIAL.print(GPS.hour, DEC); SERIAL.print(':');
  SERIAL.print(GPS.minute, DEC); SERIAL.print(':');
  SERIAL.print(GPS.seconds, DEC); SERIAL.print('.');
  SERIAL.println(GPS.milliseconds);
  // SERIAL.print(F("Fix: ")); SERIAL.print((int)GPS.fix);
  SERIAL.print(F(" quality: ")); SERIAL.println((int)GPS.fixquality);

  SERIAL.print(F("Location (deg): "));
  SERIAL.print(GPS.latitudeDegrees, 4);
  SERIAL.print(F(", "));
  SERIAL.println(GPS.longitudeDegrees, 4);
  SERIAL.print(F("Satellites: ")); SERIAL.println((int)GPS.satellites);
  String answer = "Lon: " + (String)GPS.lon +  " Lat: " + (String)GPS.lat;
  String answerLon = "Lon: " + (String)GPS.longitudeDegrees;
  String answerLat = "Lat: " + (String)GPS.latitudeDegrees;
  // gprs.sendSMS(ADMIN_NUMBER,  string2char(answer));
  SERIAL.println(answerLon);
  SERIAL.println(answerLat);

  oled.clear();
  oled.set2X();
  oled.println(answerLon);
  oled.println(answerLat);

  /*
    else {
      oled.clear();
      oled.set2X();
      oled.println("\n  No GPS");
      //gprs.sendSMS(ADMIN_NUMBER, "NO POSITION");
    }
  */
  return (1);
}

//NFC
void NFC_INIT() {
  nfc.begin();
  // nfc.getFirmwareVersion();
  nfc.SAMConfig();
}

//buzzer
void buzzer_pulse(int pulseNb, int timeBuzz) {
  for (int i = 0; i < pulseNb; i++) {
    digitalWrite(LED_BLUE, HIGH);
    digitalWrite(PIN_BUZZER, LOW);
    delay(timeBuzz);
    digitalWrite(LED_BLUE, LOW);
    digitalWrite(PIN_BUZZER, HIGH);
    delay(timeBuzz);
  }
  digitalWrite(LED_BLUE, LOW);
  digitalWrite(PIN_BUZZER, HIGH);
}

void setup() {
  SERIAL.begin(9600);
  SERIAL.println(F("********** KEYWAVE STARTED **********"));
  pinMode(LED_YELLOW, OUTPUT);
  pinMode(LED_RED, OUTPUT);
  pinMode(LED_GREEN, OUTPUT);
  pinMode(LED_BLUE, OUTPUT);
  pinMode(PIN_RF_RECEIVER, OUTPUT);
  pinMode(PIN_BUZZER, OUTPUT);
  digitalWrite(PIN_BUZZER, HIGH);
  SERVO_INIT();
  SIM808_INIT();
  GPS_INIT();
  NFC_INIT();
  LCD_INIT();
  SERIAL.println(F("********** KEYWAVE READY **********"));
}


void loop() {

    LED_STATE = !LED_STATE;
    digitalWrite(LED_YELLOW, LED_STATE);
    if (!digitalRead(PIN_BP)) {  //manual lock car
    SERIAL.println(F("BP pressed"));
    buzzer_pulse(10, 100);
    SERVO_LOCK();
    }

 // GPS_SEND_VALUES();
 
    messageIndex = gprs.isSMSunread();
    if (messageIndex > 0 )  {
      gprs.readSMS(messageIndex, message, MESSAGE_LENGTH, phone, datetime);
      gprs.deleteSMS(messageIndex);
      SERIAL.print(F("From number: "));
      SERIAL.println(phone);
      SERIAL.print(F("Message: "));
      SERIAL.println (message);
    
      if (!strcmp(phone, ADMIN_NUMBER)) {
        String head_sms;
        for (int x = 0; x < 4; x++) head_sms += message[x];
        if (head_sms == "Code") {     // new code
          for (int i = 4; i < sizeof(message); i++) new_code[i] = message[i];
          gprs.sendSMS(ADMIN_NUMBER, "CODE CHANGED");
        }
        else if (!strcmp(message, "Unlock"))    SERVO_UNLOCK();
        else if (!strcmp(message, "Lock"))      SERVO_LOCK();
        else if (!strcmp(message, "Position"))  GPS_SEND_VALUES();
        else gprs.sendSMS(ADMIN_NUMBER, "UNKNOW COMMAND");
      }
      else if (new_code == message)  SERVO_UNLOCK();
    }

    if (nfc.inListPassiveTarget()) {
    SERVO_UNLOCK();
    while (!nfc.inListPassiveTarget());
    oled.clear();
    oled.set2X();
    oled.println("\n Wrong CODE");
    buzzer_pulse(2, 100);
    delay(5000);
    oled.clear();
    oled.set2X();
    oled.println("\n  KeyWave");
    }
}
