/*Code par MrSTEIN (mrstein(at)hotmail.fr)
 v2.0/ 18/01/2015
 compatible avec NFC starter V3.0

 configuration harware:
 buzzer 5
 RelaisStarter 6
 RelaisContact_OFF 7  
 RelaisContact_ON 8
 LED 13    

 informations hardware:
 1flash: CONTACT
 2flash: STARTER
 2buzz: STOP
 3buzz: error PN532
 */    

  #include <Wire.h>
  #include <PN532_I2C.h>
  #include <PN532.h>


//definition des clefs  
String CLEF1 = "";
String CLEF2 = "";

#define interRelais 1000 // temps entre contact et starter
#define tempsStarter 800 // temps starter ON 

//definition des sorties, valable pour la v3  
#define buzzer 5
#define RelaisStarter 6
#define RelaisContact_OFF 7  
#define RelaisContact_ON 8
#define LED 13  

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);
boolean etatContact = false;


//*******************fonction LED
void fct_FLASH(int pulse){
  for(int i=0; i<pulse; i++){
  digitalWrite(LED, HIGH);
  delay(200);
  digitalWrite(LED, LOW);
  delay(200);
}
}  

//*******************fonction buzzer
void fct_BUZZ(int pulse){
  for(int i=0; i<pulse; i++){
  digitalWrite(buzzer, HIGH);
  delay(200);
  digitalWrite(buzzer, LOW);
  delay(200);
}
}

//*******************fonction contact
void fct_contact(){
fct_FLASH(1);
digitalWrite(RelaisContact_ON, HIGH);
delay(500);
digitalWrite(RelaisContact_ON, LOW);  
etatContact = true;
}

//*******************fonction starter
void fct_starter(){
fct_FLASH(2);
digitalWrite(RelaisStarter, HIGH);
delay(tempsStarter);
digitalWrite(RelaisStarter, LOW);
}

//*******************fonction STOP
void fct_STOP(){
digitalWrite(RelaisContact_OFF, HIGH);
delay(1000);
digitalWrite(RelaisContact_OFF, LOW); 
Serial.println("STOP");
}

//*******************fonction lecture de tag
String fct_lecture(){
  String Received;
  boolean success;
  uint8_t IdReceived[] = { 0, 0, 0, 0, 0, 0, 0 }; 
  uint8_t Id_Length; 

if (success = nfc.readPassiveTargetID(PN532_MIFARE_ISO14443A, &IdReceived[0],&Id_Length)){
for (uint8_t i=0; i < Id_Length; i++)
 Received += String(IdReceived[i], HEX); 
return(Received);
}
else
return(0);
}


void setup(void) {
  pinMode(buzzer,OUTPUT);
  pinMode(RelaisStarter,OUTPUT);
  pinMode(RelaisContact_ON,OUTPUT);
  pinMode(RelaisContact_OFF,OUTPUT);  
  pinMode(LED,OUTPUT);
  digitalWrite(LED, LOW);
  digitalWrite(buzzer, LOW);
  delay(100);
  
  Serial.begin(115200);
  Serial.println("start"); 
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion(); 

  if (! versiondata) {
    Serial.println("erreur pn532");  
    fct_BUZZ(3); 
    while (1); // halt

}
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
Serial.println("CAR STARTER V2.0");  
}


void loop(void) {
String Received;


    while(etatContact == false){
Serial.println("attente lecture pour contact"); 
Received=fct_lecture();
Serial.println(Received);
  if ((Received == CLEF1 || Received == CLEF2)){  
Serial.println("Clef valide, contact");  
fct_contact();
delay(interRelais);
}
}

while(etatContact == true){
Serial.println("attente lecture pour starter");   
Received=fct_lecture();

if ((Received == CLEF1 || Received == CLEF2)){ 
Serial.println("Clef valide, starter");
etatContact == false;
fct_starter();
delay(1000);
}

else{ 
fct_FLASH(3);
fct_STOP();
etatContact == false;
}
}
}
