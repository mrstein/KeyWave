  #include <Wire.h>
  #include <PN532_I2C.h>
  #include <PN532.h>

PN532_I2C pn532i2c(Wire);
PN532 nfc(pn532i2c);
String Received;

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
  Serial.begin(115200);
  nfc.begin();
  uint32_t versiondata = nfc.getFirmwareVersion(); 


  if (! versiondata) {
    Serial.println("erreur pn532");  
    while (1); // halt
}
  nfc.setPassiveActivationRetries(0xFF);
  nfc.SAMConfig();
Serial.println("attente lecture...");  
}


void loop(void) {
String temp=fct_lecture();
if(Received!=temp){
Received=temp;
Serial.println(Received);
}
}
