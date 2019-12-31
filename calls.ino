#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>


LiquidCrystal_I2C lcd(0x27, 16, 2);

SoftwareSerial btSerial(2, 3);  // tx, rx

/* PROTOCOL:
 *  CALL-G-CC-NN-R (10)
 *   GCCNN학생을 R번 방으로 부릅니다.
 *  WITH-G-CC-NN (9)
 *   GCCNN학생의 호출을 취소합니다.
 *  DRAW
 *   모든 호출을 취소합니다.
 *  LIST
 *   현재 받아 놓은 모든 요청 목록을 전송합니다.
 *   NNN____
 *    NNN글자의 데이터(____)를 전송합니다.
 *    
 *    
 *    
 * D2 - BLUETOOTH TXD
 * D3 - BLUETOOTH RXD
 * A5 - LCD SCL
 * A4 - LCD SDA 
 */

String calls;
byte index = 0;

const String officeName = "1st Grade Office  Headquarter   Std. Sec. OfficePrincipal Office";

String getOfficeNameByIndex(byte i) {
  return officeName.substring(16 * i, 16 * (i + 1));
}

void lcdStandby() {
  lcd.setCursor(0, 0); lcd.print("   /            ");
  lcd.setCursor(0, 1); lcd.print("  ( NO CALLS )  ");
}

byte getCallsCount() {
  return calls.length() / 6;
}

void refreshIndex() {
  char tmp[5];
  sprintf(tmp, "%02d/%02d", index + 1, getCallsCount());
  lcd.setCursor(1, 0); lcd.print(tmp);
}

byte getGrade(byte i) {
  return calls[index * 6];
}


byte getClass(byte i) {
  return calls.substring(index * 6 + 1, index * 6 + 3).toInt();
}

byte getNumber(byte i) {
  return calls.substring(index * 6 + 3, index * 6 + 5).toInt();
}

byte getOfficeIndex(byte i) {
  return ((String)calls[index * 6 + 5]).toInt();
}

String btBuffer;

float refreshDelta = 0;
unsigned long now;
unsigned long previousNow;

void setup() {
//  calls = "106292102064";

  btSerial.begin(9600);

  while (!btSerial);
  while (!Serial);
  
  lcd.init();
  lcd.backlight();
  lcdStandby();
}

void loop() {
  while (btSerial.available()) {
    btBuffer.concat((char)btSerial.read());
    refreshDelta = 1;
    index = 0;
  }

  if (btBuffer.length() >= 4) {
    if (btBuffer.substring(0, 4) == "CALL") {
      if (btBuffer.length() >= 10) {
        calls.concat(btBuffer.substring(4, 10));
        btBuffer = btBuffer.substring(10);
      }
    } else if (btBuffer.substring(0, 4) == "WITH") {
      if (btBuffer.length() >= 9) {
        short offset = 0;
        for (short i = 0; i < calls.length() / 6; i++) {
          i -= offset;
          if (calls.substring(i * 6, i * 6 + 5) == btBuffer.substring(4, 9)) {
            String tmp = calls.substring(0, i * 6);
            tmp.concat(calls.substring(i * 6 + 6));
            calls = tmp;
            offset++;
          }
        }
        btBuffer = btBuffer.substring(9);
      }
    } else if (btBuffer.substring(0, 4) == "DRAW") {
      calls = "";
      btBuffer = btBuffer.substring(4);
    } else if (btBuffer.substring(0, 4) == "LIST") {
      char tmp[3];
      sprintf(tmp, "%03d", calls.length());
      btSerial.print(tmp);
      btSerial.print(calls);
      btBuffer = btBuffer.substring(4);
    }
  }

  previousNow = now;
  now = millis();

  refreshDelta += (now - previousNow) / 3000.0;

  if (refreshDelta >= 1) {
    if (calls.length()) {
      refreshIndex();
    
      char tmp[7];
      sprintf(tmp, "%c %02d %02d", getGrade(index), getClass(index), getNumber(index));
      lcd.setCursor(8, 0); lcd.print(tmp);
    
      lcd.setCursor(0, 1);
      lcd.print(getOfficeNameByIndex(getOfficeIndex(index) - 1));
    
      index++;
      index %= getCallsCount();
  
      refreshDelta -= 1;
    } else {
      lcdStandby();
    }
  }
}
