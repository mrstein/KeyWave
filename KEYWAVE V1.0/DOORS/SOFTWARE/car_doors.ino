#include <KeyDuino.h>
#include <snep.h>
#include "NdefMessage.h"

KeyDuino keyDuino;
SNEP nfc(keyDuino);
uint8_t ndefBuf[128];

#define BUZZER_PIN 15
#define relay_1 7 //relay 1 on pin 7 of the KeyDuino
#define relay_2 6 //relay 2 on pin 6 of the KeyDuino
#define relay_3 5 //relay 3 on pin 5 of the KeyDuino
#define relay_4 4 //relay 4 on pin 4 of the KeyDuino
#define CHECK_ID false   //change to "true" if you want to check the good ID

const String defined_password = "MrStein"; //The unlocking password

void setup() {
  Serial.begin(9600);

  //Output definition
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(relay_1, OUTPUT);
  pinMode(relay_2, OUTPUT);
  pinMode(relay_3, OUTPUT);
  pinMode(relay_4, OUTPUT);
  digitalWrite(relay_1, LOW);
  digitalWrite(relay_2, LOW);
  digitalWrite(relay_3, LOW);
  digitalWrite(relay_4, LOW);
}

//Format data to be sent as NDEF plain text
int SEND_NDEF(String val) {
  NdefMessage message = NdefMessage();
  message.addTextRecord(val);

  int messageSize = message.getEncodedSize();
  message.encode(ndefBuf);
  if (0 >= nfc.write(ndefBuf, messageSize)) return (0);
  else return (1);
}

//Read NDEF received data and return as a string
String GET_NDEF() {
  int msgSize = nfc.read(ndefBuf, sizeof(ndefBuf));
  if (msgSize > 0) {
    NdefMessage msg  = NdefMessage(ndefBuf, msgSize);
    NdefRecord record = msg.getRecord(0);
    int payloadLength = record.getPayloadLength();
    byte payload[payloadLength];
    record.getPayload(payload);
    int startChar = 0;

    if (record.getTnf() == TNF_WELL_KNOWN && record.getType() == "T") {
      startChar = payload[0] + 1;
    }
    else if (record.getTnf() == TNF_WELL_KNOWN && record.getType() == "U") {
      startChar = 1;
    }

    String payloadAsString = "";
    for (int c = startChar; c < payloadLength; c++) {
      payloadAsString += (char)payload[c];
    }
    return (payloadAsString);
  }
  else return ("NULL");
}

void loop() {
  String content = "NULL";

  //Loop reading NDEF messages
  while (content == "NULL") content = GET_NDEF();

  //Short buzz
  digitalWrite(BUZZER_PIN, 1);
  delay(100);
  digitalWrite(BUZZER_PIN, 0);
  Serial.println("Content received: " + content);

  //If the received message is the password, then unlock
  if(content == defined_password){
      digitalWrite(relay_1, HIGH);
      digitalWrite(relay_2, HIGH);
      digitalWrite(relay_3, HIGH);
      digitalWrite(relay_4, HIGH);
      delay(200);
      digitalWrite(relay_1, LOW);
      digitalWrite(relay_2, LOW);
      digitalWrite(relay_3, LOW);
      digitalWrite(relay_4, LOW);
    
    if(!SEND_NDEF("unlocked")){
      Serial.println("Retrying response");
      delay(100);
      SEND_NDEF("unlocked");
    }
  }
  //Else if the message is "0", then the user asks to lock
  else if (content == "0"){
    if(!SEND_NDEF("locked")){
      Serial.println("Retrying response");
      delay(100);
      SEND_NDEF("locked");
    }
  }
  delay(2000);
}

