#include "BG96.h"

#define DebugSerial Serial
#define M1Serial Serial1
#define PWR_PIN 2
#define STAT_PIN 3

BG96 BG96(M1Serial, DebugSerial, PWR_PIN, STAT_PIN);

void setup() {
  // put your setup code here, to run once:
  M1Serial.begin(115200);
  DebugSerial.begin(115200);

  /* Power On Sequence */
  if (BG96.isPwrON()) {
    DebugSerial.println("BG96 Power ON Status");
    if (BG96.pwrOFF()) {
      DebugSerial.println("BG96 Power Off Error");
    } else {
      DebugSerial.println("BG96 Power Off Success");
      DebugSerial.println("Module Power ON Sequence Start");
      if (BG96.pwrON()) {
        DebugSerial.println("BG96 Power ON Error");
      } else
        DebugSerial.println("BG96 Power ON Success");
    }
  } else {
    DebugSerial.println("BG96 Power OFF Status");
    if (BG96.pwrON()) {
      DebugSerial.println("BG96 Power ON Error");
    } else
      DebugSerial.println("BG96 Power ON Success");
  }

  /* BG96 Module Initialization */
  if (BG96.init()) {
    DebugSerial.println("BG96 Module Error!!!");
  }

  /* BG96 Module Power Saving Mode Disable */
  if (BG96.disablePSM()) {
    DebugSerial.println("BG96 PSM Disable Error!!!");
  }

  /* BG96 Module GNSS Enable */
  if (BG96.actGPS()) {
    DebugSerial.println("BG96 GNSS Enable Error!!!");
  }

  /* Network Regsistraiton Check */
  while (BG96.canConnect() != 0) {
    DebugSerial.println("Network not Ready !!!");
    delay(2000);
  }

  DebugSerial.println("BG96 Module Ready!!!");

void loop() {
  /*** BG96 GNSS Test Code ***/
  /* Get GNSS Data */
  char szGNSS[256];
  if (BG96.getGPSLoc(szGNSS, sizeof(szGNSS)) == 0) {
    DebugSerial.print("GNSS : ");
    DebugSerial.println(szGNSS);
  }

  delay(2000); 
}
