#include <SoftwareSerial.h>

#include <SPI.h>
#include <MFRC522.h>


#define RST_PIN         9          // Configurable, see typical pin layout above
#define SS_PIN          10         // Configurable, see typical pin layout above

MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
SoftwareSerial btSerial(2, 3);  // Tx, Rx

unsigned long ids[] = {4294957568, 4395}; // 
bool seat[2];

unsigned long getID(){
  if ( ! mfrc522.PICC_ReadCardSerial()) { //Since a PICC placed get Serial and continue
    return -1;
  }
  unsigned long hex_num;
  hex_num =  mfrc522.uid.uidByte[0] << 24;
  hex_num += mfrc522.uid.uidByte[1] << 16;
  hex_num += mfrc522.uid.uidByte[2] <<  8;
  hex_num += mfrc522.uid.uidByte[3];
  mfrc522.PICC_HaltA(); // Stop reading
  return hex_num;
}

String btBuffer;

void setup() {
  SPI.begin();
  mfrc522.PCD_Init();
  delay(4);
  mfrc522.PCD_DumpVersionToSerial();

  btSerial.begin(9600);
  while (!btSerial);
}

void loop() {
  while (btSerial.available()) {
    btBuffer.concat((char)btSerial.read());
  }

  if (btBuffer.length() >= 4) {
    if (btBuffer.substring(0, 4) == "STAT") {
      char tmp[2];
      sprintf(tmp, "%02d", sizeof(ids) / sizeof(unsigned long));
      btSerial.print(tmp);
      for (short i = 0; i < sizeof(ids) / sizeof(unsigned long); i++) {
        btSerial.print(seat[i] ? "1" : "0"); 
      }
      btBuffer = btBuffer.substring(4);
    }
  }

  // put your main code here, to run repeatedly:
  if(mfrc522.PICC_IsNewCardPresent()) {
    unsigned long uid = getID();
    if(uid != -1){
      for (short i = 0; i < sizeof(ids); i++) {
        if (uid == ids[i]) {
          seat[i] = !seat[i];
        }
      }
    }
  }

  delay(10);
}
