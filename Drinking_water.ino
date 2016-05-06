
#include <SPI.h>
#include <MFRC522.h>
#include <SoftwareSerial.h>
#include "HX711.h"
HX711 scale(A4, A5);    // parameter "gain" is ommited; the default value 128 is used by the library
SoftwareSerial BLE_Serial(8, 7); // RX, TX
#define SS_PIN 10
#define RST_PIN 9

MFRC522 rfid(SS_PIN, RST_PIN); // Instance of the class
MFRC522::MIFARE_Key key;

// Init array that will store new NUID
byte nuidPICC[3];

void setup() {
  Serial.begin(115200);
  SPI.begin(); // Init SPI bus
  BLE_Serial.begin(9600);//debug
  int x = 0;
  rfid.PCD_Init(); // Init MFRC522

  scale.set_scale(2280.f);                      // this value is obtained by calibrating the scale with known weights; see the README for details
  scale.tare();                // reset the scale to 0

  for (byte i = 0; i < 6; i++) {
    key.keyByte[i] = 0xFF;
  }

  Serial.println(F("This code scan the MIFARE Classsic NUID."));
  Serial.print(F("Using the following key:"));

  printHex(key.keyByte, MFRC522::MF_KEY_SIZE);
}

void loop() {
  if (BLE_Serial.available() > 0) {
    char inChar = (char)BLE_Serial.read();   //读取串口获取数据
    Serial.write(inChar);       //把获取的数据返回，手机app看到返回的数据证明通信正常

    if (inChar == '?') {
      if (nuidPICC[2] != 0x00 && nuidPICC[3] != 0xFF) {
        BLE_Serial.print("*{");
        BLE_Serial.print(nuidPICC[0], HEX);
        BLE_Serial.print(nuidPICC[1], HEX);
        BLE_Serial.print(nuidPICC[2], HEX);
        BLE_Serial.print(nuidPICC[3], HEX);
        BLE_Serial.print("},{");
        BLE_Serial.print(scale.get_units(), 1);
        BLE_Serial.print("}#");
      } else {
        BLE_Serial.print("UID is Null");
      }
      digitalWrite(13, HIGH);      //B按键按下时候，点亮LED
    }
    else if (inChar == 'R') {
      digitalWrite(13, LOW);     //R按键按下时候，熄灭LED
    }
  }

  // Look for new cards
  if ( ! rfid.PICC_IsNewCardPresent())
    return;

  // Verify if the NUID has been readed
  if ( ! rfid.PICC_ReadCardSerial())
    return;

  Serial.print(F("PICC type: "));
  MFRC522::PICC_Type piccType = rfid.PICC_GetType(rfid.uid.sak);
  Serial.println(rfid.PICC_GetTypeName(piccType));

  // Check is the PICC of Classic MIFARE type
  if (piccType != MFRC522::PICC_TYPE_MIFARE_MINI &&
      piccType != MFRC522::PICC_TYPE_MIFARE_1K &&
      piccType != MFRC522::PICC_TYPE_MIFARE_4K) {
    Serial.println(F("Your tag is not of type MIFARE Classic."));
    return;
  }

  if (rfid.uid.uidByte[0] != nuidPICC[0] ||
      rfid.uid.uidByte[1] != nuidPICC[1] ||
      rfid.uid.uidByte[2] != nuidPICC[2] ||
      rfid.uid.uidByte[3] != nuidPICC[3] ) {
    Serial.println(F("A new card has been detected."));

    // Store NUID into nuidPICC array
    for (byte i = 0; i < 4; i++) {
      nuidPICC[i] = rfid.uid.uidByte[i];
    }

    Serial.println(F("The NUID tag is:"));
    Serial.print(F("In hex: "));
    printHex(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
    Serial.print(F("In dec: "));
    printDec(rfid.uid.uidByte, rfid.uid.size);
    Serial.println();
  }
  else Serial.println(F("Card read previously."));

  // Halt PICC
  rfid.PICC_HaltA();

  // Stop encryption on PCD
  rfid.PCD_StopCrypto1();
}


/**
   Helper routine to dump a byte array as hex values to Serial.
*/
void printHex(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], HEX);
    //BLE_Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    //BLE_Serial.print(buffer[i], HEX);

  }

}

/**
   Helper routine to dump a byte array as dec values to Serial.
*/
void printDec(byte *buffer, byte bufferSize) {
  for (byte i = 0; i < bufferSize; i++) {
    Serial.print(buffer[i] < 0x10 ? " 0" : " ");
    Serial.print(buffer[i], DEC);
  }
}
